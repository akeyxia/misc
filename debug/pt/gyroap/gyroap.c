#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define DEFAULT_DEVICE "/dev/gyro0"
#define GYRO_IOC_MAGIC 'G'
#define GYRO_IOCGSPEED _IOR(GYRO_IOC_MAGIC, 1, int *)
#define GYRO_IOCSREINIT _IOR(GYRO_IOC_MAGIC, 2, int *)

struct gyro_data {
	short x;
	short y;
	short z;
};

int fd = -1;

static int open_gyro(void)
{
    int dummy = 0; 

    if (fd != -1) return 0;
    if ((fd = open("/dev/gyro0", O_RDWR)) < 0) {
        printf("Can't open /dev/gyro\n");
        return 0;
    }

    if (ioctl(fd, GYRO_IOCSREINIT, &dummy)) {
        printf("Can't GYRO_IOCREINIT\n");
    }
    
    return 1;
}

static int close_gyro(void)
{
    if(fd == -1) return 1;
    close(fd);
    fd = -1;
    return 1;
}

static int gyro_read_func(void)
{
	int i;
	int count;
	static float angle_x = 0;
	static float angle_y = 0;
	static float angle_z = 0;
	struct gyro_data data = {0, 0, 0};
	struct gyro_data *ptr = (struct gyro_data *)malloc(sizeof(struct gyro_data)*32);
	
	if ((count = read(fd, ptr, sizeof(struct gyro_data))) < 0) {
		printf("read gyro data failed\n");
		return 0;
	}
	
	for (i = 0; i < count; i++) {
		data.x += (*(ptr+i)).x;
		data.y += (*(ptr+i)).y;
		data.z += (*(ptr+i)).z;
	}
	
	angle_x += data.x*8.75*0.00001;
	angle_y += data.y*8.75*0.00001;
	angle_z += data.z*8.75*0.00001;
	
	printf("-------------------------[X]: %f, [Y]: %f, [Z]: %f\n", angle_x, angle_y, angle_z);
	
	return 1;
}

int main(int argc, char *argv[])
{
	char buf[8];

	if (open_gyro()) {
		printf("open gyro success\n");
	} else {
		printf("open gyro failed\n");
		return 0;
	}

	while( 1 ) {
		//printf("gyroap# ");
		//if (!strcmp("close", fgets(buf, sizeof(buf), stdin)))
		//	break;
		
		gyro_read_func();
	}

	if(fd != 0){
		close_gyro();
	}

	return 0;
}
