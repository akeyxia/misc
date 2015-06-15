#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>

#define DEFAULT_DEVICE "/dev/ttyUSB0"

int main(int argc, char *argv[])
{
	int fd;
	int size;
	char buf[128] = {0};

	if ((fd = open(DEFAULT_DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0){
                printf("open %s error. %s\n", DEFAULT_DEVICE, strerror(errno));
                return -1;
        }

	if (isatty(fd))
	{
		struct termios ios;
		speed_t speed = B115200;		
		tcgetattr(fd, &ios );
		ios.c_iflag = IGNBRK | IGNPAR;
		ios.c_cflag = CLOCAL | CREAD | CS8 | HUPCL;
		ios.c_oflag = 0;
		ios.c_lflag = 0;
		ios.c_cc[VMIN] = 1;
		ios.c_cc[VTIME] = 10;
		cfsetispeed(&ios, speed);
		cfsetospeed(&ios, speed);
		tcsetattr(fd, TCSANOW, &ios);
	} else {
		printf("Not TTY Device\n");
	}

	while(1) {
		printf("rdstool# ");
		size = strlen(fgets(buf, sizeof(buf), stdin));
		write(fd, buf, size);
	}

	return 0;
}

