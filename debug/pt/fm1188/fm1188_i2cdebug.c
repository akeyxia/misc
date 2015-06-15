/*
 * Echo Cancel FM1188 driver
 *
 * Copyright (C) 2009, Pateo Inc.
 *
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/sysctl.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/smp_lock.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/suspend.h>
#include <linux/fs.h>
#include <mach/gpio.h>
#include <mach/fm1188.h>
#include <linux/clk.h>
static struct i2c_client *fm1188_client;

//#define _FM1188_DEBUG_	1

/* init all regs after reset*/
const static u8 FM1188_REG_CMDS[] =
{
         0xfc, 0xf3, 0x3b, 0x1e, 0x30, 0x03, 0x39,//{0xfc, 0xf3, 0x3b, 0x1e, 0x30, 0x02, 0x31},
         0xfc, 0xf3, 0x3b, 0x1e, 0x34, 0x00, 0x9a,//mic0_pgagain: 0x92(s)
         0xfc, 0xf3, 0x3b, 0x1e, 0x37, 0x00, 0x1f,
         0xfc, 0xf3, 0x3b, 0x1e, 0x38, 0x00, 0x0f,//0x41, 0x0f,//0x00, 0x0f,//0x1f,
         0xfc, 0xf3, 0x3b, 0x1e, 0x3d, 0x01, 0x00,//mic_volume: 0x0580(s)
         0xfc, 0xf3, 0x3b, 0x1e, 0x3e, 0x00, 0x80,//0x01, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x41, 0x01, 0x01,
         0xfc, 0xf3, 0x3b, 0x1e, 0x44, 0x00, 0x81,////0x0081(s)-->0x00c1
         0xfc, 0xf3, 0x3b, 0x1e, 0x45, 0x43, 0xed,//SP_flag: 0x43ed
         0xfc, 0xf3, 0x3b, 0x1e, 0x46, 0x00, 0x71,////Ft_flag: 0x0071(s)-->0x0050
         //0xfc, 0xf3, 0x3b, 0x1e, 0x47, 0x07, 0xff,//nss: 0x2600(d)
         0xfc, 0xf3, 0x3b, 0x1e, 0x48, 0x08, 0x00,//lbg: 0x1200(d)
         //0xfc, 0xf3, 0x3b, 0x1e, 0x49, 0x16, 0x00,//hbg: 0x800(d)-->0x1200
         0xfc, 0xf3, 0x3b, 0x1e, 0x4d, 0x01, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x57, 0x7e, 0x00,//0x7f,0xff,//0x7e, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x63, 0x00, 0x01,
         0xfc, 0xf3, 0x3b, 0x1e, 0x6a, 0x00, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x6b, 0x00, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x86, 0x00, 0x09,
         0xfc, 0xf3, 0x3b, 0x1e, 0x87, 0x00, 0x03,
         0xfc, 0xf3, 0x3b, 0x1e, 0x88, 0x20, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x8b, 0x02, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x8c, 0x00, 0x80,
         0xfc, 0xf3, 0x3b, 0x1e, 0x92, 0x40, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0x95, 0x30, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xa0, 0x12, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xa2, 0x33, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xbd, 0x70, 0x20,
         0xfc, 0xf3, 0x3b, 0x1e, 0xbe, 0x70, 0x10,
         0xfc, 0xf3, 0x3b, 0x1e, 0xbf, 0x70, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xc0, 0x08, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xc1, 0x0a, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xc2, 0x10, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xca, 0x04, 0x80,
         0xfc, 0xf3, 0x3b, 0x1e, 0xe2, 0x06, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xe3, 0x10, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xe8, 0x60, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xe9, 0x40, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xea, 0x7f, 0xff,
         0xfc, 0xf3, 0x3b, 0x1e, 0xeb, 0x22, 0x00,
         0xfc, 0xf3, 0x3b, 0x1e, 0xef, 0x05, 0x00,
         0xfc, 0xf3, 0x3b, 0x1f, 0x10, 0x7f, 0xff,
         0xfc, 0xf3, 0x3b, 0x1f, 0x11, 0x60, 0x50,
         0xfc, 0xf3, 0x3b, 0x1f, 0x12, 0x4c, 0x9f,
         0xfc, 0xf3, 0x3b, 0x1f, 0x13, 0x3f, 0x82,
         0xfc, 0xf3, 0x3b, 0x1f, 0x14, 0x36, 0x3e,
         0xfc, 0xf3, 0x3b, 0x1f, 0x15, 0x2e, 0xb2,
         0xfc, 0xf3, 0x3b, 0x1f, 0x16, 0x27, 0x11,
         0xfc, 0xf3, 0x3b, 0x1f, 0x17, 0x1e, 0xf8,
         0xfc, 0xf3, 0x3b, 0x1f, 0x18, 0x17, 0xcb,
         0xfc, 0xf3, 0x3b, 0x1f, 0x19, 0x12, 0xf0,
	0xfc, 0xf3, 0x3b, 0x1f, 0x4b, 0x00, 0x00,
	0xfc, 0xf3, 0x3b, 0x1e, 0x35, 0x00, 0x0f,
	0xfc, 0xf3, 0x3b, 0x1e, 0x3a, 0x00, 0x00,
	/*0xfc, 0xf3, 0x3b, 0x1e, 0x32, 0x00, 0x01,*/
};

static int fm1188_initchip(void)
{
	int i2c_ret = 0;
	u16 addr = fm1188_client->addr;
	u16 flags = 0;
	int fm_len = sizeof(FM1188_REG_CMDS);
	struct i2c_msg msg = { addr, flags,fm_len, FM1188_REG_CMDS };

	i2c_ret = i2c_transfer(fm1188_client->adapter, &msg, 1);
	dev_info(&fm1188_client->dev, "fm1188_initchip i2c_ret=%d, fm_len=%d\n", i2c_ret, fm_len);
	return i2c_ret;
}

int fm1188_bypass(int enable)
{
	struct fm1188_platform_data *pdata;
	int ret = 0;

	if (fm1188_client == NULL) {
		pr_err("fm1188_client is not initialized\n");
		return -1;
	}

	pdata = fm1188_client->dev.platform_data;

	if(enable) {
		if(pdata->bypass_io_enable)
			pdata->bypass_io_enable();
		pr_info("%s, enable successful.\n", __func__);
	} else {
		if(pdata->init)
			pdata->init();
		ret = fm1188_initchip();
		if (ret < 0)
			pr_err("%s, disable ret 0x%x\n", __func__, ret);
		else
			pr_info("%s, disable successful.\n", __func__);
	}

	return ret;
}

static int fm1188_write(unsigned short reg, unsigned short data)
{
	unsigned short addr = fm1188_client->addr;
	unsigned short flags = fm1188_client->flags;
	int i2c_ret;
	unsigned char buf[7] = {0xfc, 0xf3, 0x3b, ((reg >> 8) & 0xff), ((reg & 0xff)), ((data >> 8) & 0xff), ((data & 0xff))};
	struct i2c_msg msg = { addr, flags, 7 , buf };
	int i;

	printk("\n");
	for(i = 0; i < 7; i++)
		printk("0x%x, ", buf[i]);
	printk("\n");

	i2c_ret = i2c_transfer(fm1188_client->adapter, &msg, 1);

	if (i2c_ret < 0) {
		pr_err("%s: 0x%X ---> write reg error(%d)\n", __func__, addr, i2c_ret);
	} else
		i2c_ret = 0;

	return i2c_ret;
}

static int fm1188_read(unsigned short reg, unsigned short *data)
{
	unsigned short addr = addr = fm1188_client->addr;
	unsigned short flags = fm1188_client->flags;
	int i2c_ret;
	unsigned char buf[5] = {0xfc, 0xf3, 0x37, ((reg >> 8) & 0xff), ((reg & 0xff))};
	unsigned char rd_buf[2] = {0};
	struct i2c_msg msg[2] = {
		{addr, flags, 5 , buf },
		{addr, flags | I2C_M_RD, 2 , rd_buf },
	};

	i2c_ret = i2c_transfer(fm1188_client->adapter, msg, 2);

	if (i2c_ret < 0) {
		pr_err("%s: reg reg error : Reg 0x%02x: ", __func__, reg);
	} else
		i2c_ret = 0;

	*data = ((rd_buf[0] << 8) | rd_buf[1]);
	pr_info("data = 0x%X\n", *data);

	return i2c_ret;
}


/*
 * sysfs interface for fm1188 reg read and write.
 */
static unsigned short g_reg, g_data;

static ssize_t fm1188_reg_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
        int count;
        count = sprintf(buf, "0x%x\n", g_reg);

        return count;
}

static ssize_t fm1188_reg_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
        int va_count;
        va_count = sscanf(buf, "0x%x", &g_reg);
        if (va_count != 1)
                pr_err("%s: %d string were translated successfully", __func__,
                                va_count);

        return count;
}

static ssize_t fm1188_data_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
        int count;

        g_data = 0x0000;
        fm1188_read(g_reg, &g_data);
        count = sprintf(buf, "0x%x\n", g_data);

        return count;
}

static ssize_t fm1188_data_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
        int va_count;

        g_data = 0x0000;
        va_count = sscanf(buf, "0x%x", &g_data);
        if (va_count != 1)
                pr_err("%s: %d string were translated successfully", __func__,
                                va_count);
        fm1188_write(g_reg, g_data);

        return count;
}

DEVICE_ATTR(g_reg, 0644, fm1188_reg_show, fm1188_reg_store);
DEVICE_ATTR(g_data, 0644, fm1188_data_show, fm1188_data_store);

static int __devinit fm1188_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct  fm1188_platform_data *pdata;
	int ret;

	fm1188_client = client;
	pdata = client->dev.platform_data;

#if !defined(CONFIG_MACH_MX53_A1001EM_PV)
	/* enable fm1188 */
	if (pdata->init)
		pdata->init();

	ret = fm1188_initchip();
	if (ret < 0)
		pr_info("Initialize fm1188 failed with ret 0x%x\n", ret);
	else
		pr_info("Initialize fm1188 successfully\n");
#endif

#ifdef _FM1188_DEBUG_
	ret = device_create_file(&client->dev, &dev_attr_g_reg);
	if (ret)
		device_remove_file(&client->dev, &dev_attr_g_reg);
	ret = device_create_file(&client->dev, &dev_attr_g_data);
	if (ret)
		device_remove_file(&client->dev, &dev_attr_g_data);
#endif

	return 0;
}

static int fm1188_remove(struct i2c_client *client)
{
#ifdef _FM1188_DEBUG_
	device_remove_file(&client->dev, &dev_attr_g_reg);
	device_remove_file(&client->dev, &dev_attr_g_data);
#endif
	return 0;
}

static const struct i2c_device_id fm1188_id[] = {
	{ "fm1188", 0 },
	{ }
};

static struct i2c_driver fm1188_driver = {
	.driver = {
		.name	= "fm1188",
	},
	.probe		= fm1188_probe,
	.remove		= fm1188_remove,
	.id_table	= fm1188_id,
};

static int __init fm1188_init(void)
{
	return i2c_add_driver(&fm1188_driver);
}

static void __exit fm1188_exit(void)
{
	i2c_del_driver(&fm1188_driver);
}

module_init(fm1188_init);
module_exit(fm1188_exit);

MODULE_DESCRIPTION("fm1188 Echo Cancel Driver IC");
MODULE_LICENSE("GPL");

