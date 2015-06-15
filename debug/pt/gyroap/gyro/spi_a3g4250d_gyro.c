#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <linux/ioctl.h>
#include <linux/proc_fs.h>

/* -----------------------------*/
/*        GYRO A3G4250D Register      */
/* -----------------------------*/

#define WHO_AM_I       0x0F
#define CTRL_REG1      0x20
#define CTRL_REG2      0x21
#define CTRL_REG3      0x22
#define CTRL_REG4      0x23
#define CTRL_REG5      0x24
#define REFERENCE      0x25
#define OUT_TEMP       0x26
#define STATUS_REG     0x27
#define OUT_X_L        0x28
#define OUT_X_H        0x29
#define OUT_Y_L        0x2A
#define OUT_Y_H        0x2B
#define OUT_Z_L        0x2C
#define OUT_Z_H        0x2D
#define FIFO_CTRL_REG  0x2E
#define FIFO_SRC_REG   0x2F
#define INT1_CFG       0x30
#define INT1_SRC       0x31
#define INT1_TSH_XH    0x32
#define INT1_TSH_XL    0x33
#define INT1_TSH_YH    0x34
#define INT1_TSH_YL    0x35
#define INT1_TSH_ZH    0x36
#define INT1_TSH_ZL    0x37
#define INT1_DURATION  0x38

/*#define GYRO_DEBUG*/

#ifdef GYRO_DEBUG
#define gyro_debug(fmt, ...) \
	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#else
#define gyro_debug(fmt, ...) \
	({ 0; })
#endif
#define gyro_info(fmt, ...) \
	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define gyro_err(fmt, ...) \
	printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)

#define GYRO_IOC_MAGIC 'G'
#define GYRO_IOCGSPEED _IOR(GYRO_IOC_MAGIC, 1, int *)
#define GYRO_IOCSREINIT _IOR(GYRO_IOC_MAGIC, 2, int *)
#define GYRO_IOCGGETFREQ _IOR(GYRO_IOC_MAGIC, 3, int *)

#define LOOPTH 300
#define TIME_DIV 200

struct gyro_data
{
	short x;
	short y;
	short z;
};

struct gyro_dev {
	char len;
	struct device *dev;
	dev_t chrdev;
	atomic_t count;
	unsigned int loop;
	unsigned int loop_fm;
	unsigned int write_p;
	unsigned int write_p_fm;
	unsigned long io_base;

	int (*write)(struct device *dev, unsigned int reg, unsigned int val);
	int (*read)(struct device *dev, unsigned int reg);

	struct cdev cdev;
	struct gyro_data data[LOOPTH];
	struct gyro_data data_fm[LOOPTH];
	struct semaphore buffer_sem;
};

static struct gyro_dev *my_gyro_dev = NULL;
static struct delayed_work read_work;

static int spi_read_write(struct spi_device *spi, u8 * buf, u32 len)
{
	struct spi_message m;
	struct spi_transfer t;

	if (len > 24 || len <= 0)
		return -1;

	spi->bits_per_word = 16;
	spi_setup(spi);

	spi_message_init(&m);
	memset(&t, 0, sizeof t);

	t.tx_buf = buf;
	t.rx_buf = buf;
	t.len = ((len - 1) >> 2) + 1;

	spi_message_add_tail(&t, &m);

	if (spi_sync(spi, &m) != 0 || m.status != 0) {
		gyro_err("%s: error\n", __func__);
		return -1;
	}

	return 0;
}

static int gyro_reg_write(struct device *dev, unsigned int reg, unsigned int val)
{
	int ret;
	u16 word;
	struct spi_device *spi = to_spi_device(dev);

	reg &= ~(1<<7);
	reg &= ~(1<<6);
	word = reg << 8 | (val & 0xff);

	ret = spi_read_write(spi, (u8 *)&word, 2);

	return ret;
}

static int gyro_reg_read(struct device *dev, unsigned int reg)
{
	int ret;
	u16 val;
	struct spi_device *spi = to_spi_device(dev);

	reg |= (1<<7);
	reg &= ~(1<<6);
	val = reg << 8;

	ret = spi_read_write(spi, (u8 *)&val, 2);

	return val & 0xff;
}

static void gyro_timer_init(struct gyro_dev *dev)
{
	dev->write_p = dev->loop = 0;
	dev->write_p_fm = dev->loop_fm = 0;
	schedule_delayed_work(&read_work, msecs_to_jiffies(TIME_DIV));
}

static void gyro_value_get(struct work_struct *work)
{
	struct gyro_dev *dev = my_gyro_dev;
	char data_l,data_h;
	int len, i;
	int datax = 0;
	int datay = 0;
	int dataz = 0;
	static int angle_x = 0;
	static int angle_y = 0;
	static int angle_z = 0;

	unsigned long interval = 0;
	static unsigned long lasttime = 0;

	interval = jiffies_to_msecs(jiffies - lasttime);

	len = gyro_reg_read(my_gyro_dev->dev, FIFO_SRC_REG);
	len &= 0x1f;
	if (len == 0) {
		schedule_delayed_work(&read_work, msecs_to_jiffies(TIME_DIV));
		gyro_err("%s no gyro data!\n",__func__);
		return;
	}
	dev->len = len;

	for(i=0; i < len; i++) {
		data_l = gyro_reg_read(my_gyro_dev->dev, OUT_X_L);
		data_h = gyro_reg_read(my_gyro_dev->dev, OUT_X_H);
		dev->data[dev->write_p % LOOPTH].x = (data_h << 8) | data_l;
		dev->data_fm[dev->write_p_fm % LOOPTH].x = (data_h << 8) | data_l;

		data_l = gyro_reg_read(my_gyro_dev->dev, OUT_Y_L);
		data_h = gyro_reg_read(my_gyro_dev->dev, OUT_Y_H);
		dev->data[dev->write_p % LOOPTH].y = (data_h << 8) | data_l;
		dev->data_fm[dev->write_p_fm % LOOPTH].y = (data_h << 8) | data_l;

		data_l = gyro_reg_read(my_gyro_dev->dev, OUT_Z_L);
		data_h = gyro_reg_read(my_gyro_dev->dev, OUT_Z_H);
		dev->data[dev->write_p % LOOPTH].z = (data_h << 8) | data_l;
		dev->data_fm[dev->write_p_fm % LOOPTH].z = (data_h << 8) | data_l;

		datax += dev->data[dev->write_p].x;
		datay += dev->data[dev->write_p].y;
		dataz += dev->data[dev->write_p].z;

		dev->write_p++;
		dev->write_p_fm++;

		if (dev->write_p == LOOPTH) {
			dev->write_p = 0;
			dev->loop = 1;
		}
		if (dev->write_p_fm == LOOPTH) {
			dev->write_p_fm = 0;
			dev->loop_fm = 1;
		}
	}

	angle_x += datax*35/400000;
	angle_y += datay*35/400000;
	angle_z += dataz*35/400000;
	gyro_info("[X]:%d(%d), [Y]:%d(%d), [Z]:%d(%d)\n",
		datax, angle_x,
		datay, angle_y,
		dataz, angle_z);

	lasttime = jiffies;
	schedule_delayed_work(&read_work, msecs_to_jiffies(TIME_DIV));
	gyro_debug("%s interval =%d ms.\n",__func__,interval);
	return;
}

static void gyro_dev_init(void)
{
	gyro_reg_write(my_gyro_dev->dev, CTRL_REG1, 0x07);
	gyro_reg_write(my_gyro_dev->dev, CTRL_REG2, 0x20);
	gyro_reg_write(my_gyro_dev->dev, CTRL_REG3, 0x00);
	gyro_reg_write(my_gyro_dev->dev, CTRL_REG4, 0x80);
	gyro_reg_write(my_gyro_dev->dev, CTRL_REG5, 0x50);
	gyro_reg_write(my_gyro_dev->dev, FIFO_CTRL_REG, 0x5F);
}

static void gyro_power_on(void)
{
	gyro_reg_write(my_gyro_dev->dev, CTRL_REG1, 0x0f);
	msleep(250);
}

static void gyro_power_off(void)
{
	gyro_reg_write(my_gyro_dev->dev, CTRL_REG1, 0x07);
}

static int gyro_open(struct inode *inode, struct file *filp)
{
	struct gyro_dev *dev = container_of(inode->i_cdev, struct gyro_dev, cdev);
	filp->private_data = dev;

	if(dev == NULL) {
		gyro_info("dev is null\n");
		return 0;
	}

	if (atomic_read(&dev->count)) {
		gyro_info("%s again...!\n",__func__);
		atomic_inc(&dev->count);
		return 0;
	}

	gyro_debug("%s first...\n",__func__);
	atomic_inc(&dev->count);
	gyro_power_on();
	gyro_timer_init(dev);

	return 0;
}

static ssize_t gyro_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{

	int fm_flag = 0;
	int array_f = 0;
	int array_i = 0;
	int array_b = 0;
	int read_c = 0;
	int read_b = 0;
	int read_i = count;

	struct gyro_data data[LOOPTH];
	struct gyro_dev *dev = filp->private_data;
	unsigned long interval = 0;
	static unsigned long lasttime = 0;

	if ((buf == NULL) || (count <= 0)) {
		gyro_err("%s [buf:%p,count:%d] is wrong!\n",__func__,buf,count);
		return -EFAULT;
	}

	interval = jiffies_to_msecs(jiffies - lasttime);
	memset(data,0,LOOPTH * sizeof(struct gyro_data));

	down(&dev->buffer_sem);
	if ((read_i&0xff00) == 0x5a00)
		fm_flag = 1;

	read_i &= 0xff;
	if (read_i >= LOOPTH)
		read_i = LOOPTH;
	read_b = read_i * sizeof(struct gyro_data);
	read_c = read_b;

	if (fm_flag == 0) {
		array_f = dev->loop;
		array_i = dev->write_p;
		array_b = dev->write_p * sizeof(struct gyro_data);
		memcpy(data,dev->data,LOOPTH * sizeof(struct gyro_data));
	} else {
		array_f = dev->loop_fm;
		array_i = dev->write_p_fm;
		array_b = dev->write_p_fm * sizeof(struct gyro_data);
		memcpy(data,dev->data_fm,LOOPTH * sizeof(struct gyro_data));
	}
	up(&dev->buffer_sem);

	gyro_debug("array_f:%d,array_i=%d,read_i=%d,fm_flag=%d.\n",array_f,array_i,read_i,fm_flag);

	if (array_f == 0) {
		if (read_i < array_i) {
			if (copy_to_user(buf, &data[array_i - read_i], read_c)) {
				gyro_err("%s:line %d, failed to copy_to_user.\n", __func__,__LINE__);
				return -EFAULT;
			}
		} else {
			read_c = array_b;
			if (copy_to_user(buf, &data[0], read_c)) {
				gyro_err("%s:line %d, failed to copy_to_user.\n", __func__,__LINE__);
				return -EFAULT;
			}
		}
	} else {
		if (read_i < array_i) {
			if (copy_to_user(buf, &data[array_i - read_i], read_c)) {
				gyro_err("%s:line %d, failed to copy_to_user.\n", __func__,__LINE__);
				return -EFAULT;
			}
		} else if (read_i == array_i) {
			if (copy_to_user(buf, &data[0], read_c)) {
				gyro_err("%s:line %d, failed to copy_to_user.\n", __func__,__LINE__);
				return -EFAULT;
			}
		} else {
			if (copy_to_user(buf, &data[LOOPTH-(read_i-array_i)], (read_b - array_b))) {
				gyro_err("%s:line %d, failed to copy_to_user.\n", __func__,__LINE__);
				return -EFAULT;
			}
			if (copy_to_user(buf+(read_b - array_b), &data[0],array_b)) {
				gyro_err("%s:line %d, failed to copy_to_user.\n", __func__,__LINE__);
				return -EFAULT;
			}
		}
	}

	down(&dev->buffer_sem);
	if (fm_flag == 0) {
		dev->loop = 0;
		dev->write_p = 0;
		memset(&dev->data[0], 0, LOOPTH * sizeof(struct gyro_data));
	} else {
		dev->loop_fm = 0;
		dev->write_p_fm = 0;
		memset(&dev->data_fm[0], 0, LOOPTH * sizeof(struct gyro_data));
	}
	up(&dev->buffer_sem);
	read_c = read_c / sizeof(struct gyro_data);

	lasttime = jiffies;

	gyro_debug("%s read count:%d,return count=%d,interval=%d.\n",__func__,count,read_c,interval);

	return read_c;
}

static int gyro_release(struct inode *inode, struct file *filp)
{
	struct gyro_dev *dev = filp->private_data;


	atomic_dec(&dev->count);

	if (atomic_read(&dev->count) == 0) {
		cancel_delayed_work(&read_work);
		gyro_power_off();
	}

	return 0;
}

static int gyro_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	u8 data;
	short freq;
	struct gyro_dev *dev = file->private_data;

	if (_IOC_TYPE(cmd) != GYRO_IOC_MAGIC)
		return -ENOTTY;

	switch (cmd) {
	case GYRO_IOCSREINIT:
		if (atomic_read(&dev->count)) {
			cancel_delayed_work(&read_work);
		}
		gyro_timer_init(dev);
		break;

	case GYRO_IOCGGETFREQ:
		data = gyro_reg_read(my_gyro_dev->dev, CTRL_REG1);
		data &= 0xC0;
		gyro_debug("%s [CTRL_REG1]:0x%x\n",__func__,data);
		switch (data) {
		case 0x0:
			freq = 100;
			break;
		case 0x40:
			freq = 200;
			break;
		case 0x80:
			freq = 400;
			break;
		case 0xc0:
			freq = 800;
			break;
		default:
			break;
		}

		gyro_info("%s [freq]:%d .\n",__func__,freq);
		if (copy_to_user((void __user *)arg, &freq, sizeof(freq))) {
			gyro_err("%s [cmd:%d] copy_to_user failed!\n",__func__,cmd);
			return -EFAULT;
		}
		break;

	default:
		gyro_err("%s invalid ioctl code %d\n", __func__, cmd);
		return -ENOTTY;
	}

	return 0;
}

struct file_operations gyro_fops = {
	.owner = THIS_MODULE,
	.open = gyro_open,
	.read = gyro_read,
	.ioctl = gyro_ioctl,
	.release = gyro_release,
};

#ifdef CONFIG_PROC_FS
#define GYRO_PROC_FILE "driver/gyro"
static struct proc_dir_entry *gyro_proc_file = NULL;
static ssize_t gyro_proc_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
	char i,len;
	char data_l,data_h;
	struct gyro_data data;

	data.x = 0;
	data.y = 0;
	data.z = 0;

	len = gyro_reg_read(my_gyro_dev->dev, FIFO_SRC_REG);
	len &= 0x1f;
	gyro_info("---- [len]:0x%x .\n",len);

	for (i=0; i<len; i++) {
		data_l = gyro_reg_read(my_gyro_dev->dev, OUT_X_L);
		data_h = gyro_reg_read(my_gyro_dev->dev, OUT_X_H);
		data.x += ((data_h << 8) | data_l);

		data_l = gyro_reg_read(my_gyro_dev->dev, OUT_Y_L);
		data_h = gyro_reg_read(my_gyro_dev->dev, OUT_Y_H);
		data.y += ((data_h << 8) | data_l);

		data_l = gyro_reg_read(my_gyro_dev->dev, OUT_Z_L);
		data_h = gyro_reg_read(my_gyro_dev->dev, OUT_Z_H);
		data.z += ((data_h << 8) | data_l);

		//gyro_info("---- [X_Axis]: [%d * 8.75 * 0.00001]	( %d ) .\n",data.x, data.x*35/400000);
		//gyro_info("---- [Y_Axis]: [%d * 8.75 * 0.00001]	( %d ) .\n",data.y, data.y*35/400000);
		//gyro_info("---- [Z_Axis]: [%d * 8.75 * 0.00001]	( %d ) .\n",data.z, data.z*35/400000);
		//gyro_info("-------------------------------------\n");
		gyro_info("[X]:%d(%d), [Y]:%d(%d), [Z]:%d(%d)\n",
			data.x, data.x*35/4000,
			data.y, data.y*35/4000,
			data.z, data.z*35/4000);
	}

	return 0;
}

static ssize_t gyro_proc_write(struct file *filp,
	const char *buff, size_t len, loff_t *off)
{
	char msg[256];

	memset(msg, 0x00, 256);
	if (copy_from_user(msg, buff, len))
		return -EFAULT;

	if (!strncmp(msg, "on", 2)) {
		pr_info("gyro on\n");
		gyro_power_on();
		gyro_timer_init(my_gyro_dev);
	} else if (!strncmp(msg, "off", 3)) {
		pr_info("gyro off\n");
		cancel_delayed_work(&read_work);
		gyro_power_off();
	} else {
		pr_info("gyro ver [%d]\n", gyro_reg_read(my_gyro_dev->dev, WHO_AM_I));
	}

	return len;
}

static struct file_operations gyro_proc_ops = {
	.read = gyro_proc_read,
	.write = gyro_proc_write,
};

static void create_gyro_proc_file(void)
{
	gyro_proc_file = create_proc_entry(GYRO_PROC_FILE, 0644, NULL);
	if (gyro_proc_file) {
		gyro_proc_file->proc_fops = &gyro_proc_ops;
	} else {
		gyro_err("%s failed!\n",__func__);
	}

	return;
}

static void remove_gyro_proc_file(void)
{
	if (gyro_proc_file)
		remove_proc_entry(GYRO_PROC_FILE, NULL);

	return;
}
#endif

static struct class *my_class = NULL;
static int __devinit gyro_probe(struct spi_device *spi)
{
	int err = 0;
	struct device *dev;

	my_gyro_dev = kzalloc(sizeof(struct gyro_dev), GFP_KERNEL);
	if (NULL == my_gyro_dev) {
		gyro_err("%s kzalloc failed!\n",__func__);
		return -ENOMEM;
	}

	spi_set_drvdata(spi, my_gyro_dev);
	my_gyro_dev->read = gyro_reg_read;
	my_gyro_dev->write = gyro_reg_write;
	my_gyro_dev->dev = &spi->dev;

	init_MUTEX(&my_gyro_dev->buffer_sem);
	gyro_dev_init();

	err = alloc_chrdev_region(&my_gyro_dev->chrdev, 0, 1, "gyro");
	if (err) {
		gyro_err("%s alloc_chrdev_region failed!\n",__func__);
		goto alloc_chrdev_err;
	}
	cdev_init(&(my_gyro_dev->cdev), &gyro_fops);
	my_gyro_dev->cdev.owner = THIS_MODULE;
	my_gyro_dev->cdev.ops = &gyro_fops;
	err = cdev_add(&(my_gyro_dev->cdev), my_gyro_dev->chrdev, 1);
	if (err) {
		gyro_err("%s cdev_add failed!\n",__func__);
		goto cdev_add_err;
	}

	my_class = class_create(THIS_MODULE, "gyro_class");
	if (IS_ERR(my_class)) {
		err = PTR_ERR(my_class);
		gyro_err("%s class_create failed!\n",__func__);
		goto class_err;
	}
	dev = device_create(my_class, NULL, my_gyro_dev->chrdev, NULL, "gyro%d", 0 );
	if (IS_ERR(dev)) {
		err = PTR_ERR(dev);
		gyro_err("%s device_create failed!\n",__func__);
		goto device_err;
	}

	create_gyro_proc_file();
	if (!gyro_proc_file)
		gyro_err("%s create_gyro_proc_file failed!\n",__func__);

	INIT_DELAYED_WORK(&read_work, gyro_value_get);

	gyro_info("GYRO A3G4250D initialization\n");

	return 0;

device_err:
	device_destroy(my_class, my_gyro_dev->chrdev);
	class_destroy(my_class);
class_err:
	cdev_del(&my_gyro_dev->cdev);
cdev_add_err:
	unregister_chrdev_region(my_gyro_dev->chrdev, 1);
alloc_chrdev_err:
	kfree(my_gyro_dev);
	return err;
}

static int __devexit gyro_remove(struct spi_device *spi)
{
	cancel_delayed_work(&read_work);
	remove_gyro_proc_file();
	cdev_del(&my_gyro_dev->cdev);
	device_destroy(my_class, my_gyro_dev->chrdev);
	class_destroy(my_class);
	kfree(my_gyro_dev);
	return 0;
}

static int gyro_suspend(struct spi_device *spi, pm_message_t mesg)
{
	if (atomic_read(&my_gyro_dev->count)) {
		cancel_delayed_work(&read_work);
	}
	msleep(50);
	gyro_power_off();

	return 0;
}

static int gyro_resume(struct spi_device *spi)
{
	gyro_dev_init();
	gyro_power_on();
	if (atomic_read(&my_gyro_dev->count)) {
		my_gyro_dev->write_p = my_gyro_dev->loop = 0;
		my_gyro_dev->write_p_fm = my_gyro_dev->loop_fm = 0;
		schedule_delayed_work(&read_work, msecs_to_jiffies(2000));
	}

	return 0;
}

static const struct spi_device_id a3g4250d_id[] = {
	{ "a3g4250d", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, a3g4250d_id);

static struct spi_driver gyro_driver = {
	.id_table = a3g4250d_id,
	.driver = {
		.name = "a3g4250d",
		.owner = THIS_MODULE,
	},
	.probe = gyro_probe,
	.remove = __devexit_p(gyro_remove),
	.suspend = gyro_suspend,
	.resume = gyro_resume,
};

static int __init gyro_init(void)
{
	int ret;

	ret = spi_register_driver(&gyro_driver);

	return ret;
}

static void __exit gyro_exit(void)
{
	spi_unregister_driver(&gyro_driver);
}

module_init(gyro_init);
module_exit(gyro_exit);

MODULE_AUTHOR("yalizhao");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("GYRO");
