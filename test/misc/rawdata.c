#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int i, len;
	unsigned char ch;
	int fd;

	ch = 0xAA;
	len = 5000000;

	fd = open("audio_rawdata", O_WRONLY | O_CREAT);
	if (fd < 0) {
		printf("open file failed\n");
		return -1;
	}

	printf("begin write...\n");

	for (i = 0; i < len; i++)
		write(fd, &ch, 1);
	close(fd);
	
	printf("write done\n");

	return 0;
}
