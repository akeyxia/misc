#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>

int main(int argc, char **argv)
{
	const char *devpath; //eg. "/dev/bus/usb/003/001"
	int fd;

	devpath = argv[1];
	fd = open(devpath, O_WRONLY);
	if (fd < 0) {
		printf("open %s failed\n", devpath);
		return -1;
	}

	/* perform a USB port reset */
	ioctl(fd, USBDEVFS_RESET, 0);

	close(fd);

	return 0;
}
