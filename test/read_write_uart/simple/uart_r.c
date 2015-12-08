#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/ttyO5"
int fd = -1;

int main(int argc, char *argv[])
{
	char buf[1024];
	int nn;
	int ret;

	fd = open(DEVICE_PATH, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) {
		printf("could not open %s, err: %s)\n", DEVICE_PATH, strerror(errno));
		return -1;
	}

	if (isatty(fd)) {
		struct termios  ios;
		speed_t speed = B115200;
		tcgetattr(fd, &ios);
		ios.c_iflag = IGNBRK | IGNPAR;
	//      ios.c_cflag = CLOCAL | CREAD | CS8 | HUPCL | CRTSCTS;
		ios.c_cflag = CLOCAL | CREAD | CS8 | HUPCL;
		ios.c_oflag = 0;
		ios.c_lflag = 0;
		ios.c_cc[VMIN] = 1;
		ios.c_cc[VTIME] = 10;
		cfsetispeed(&ios, speed);
		cfsetospeed(&ios, speed);
		tcsetattr(fd, TCSANOW, &ios);
	}

	while(1) {
		ret = read(fd, buf, sizeof(buf));
		if (ret > 0) {
			printf("read buffer size: %d\n");
			for (nn = 0; nn < ret; nn++)
				printf("%X, ", buf[nn]);
			printf("\n");	
		}
	}

	return 0;
}


