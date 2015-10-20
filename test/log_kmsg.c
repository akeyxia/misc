#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DATA_SIZE 4096
#define SLEEP_TIME 50000
#define KMSG_PATH "/proc/kmsg"
#define LOG_FILE_PATH "/data/tsi.log"

int main(int arg, char *argv[])
{
	int ret;
	int r_fd;
	int w_fd;
	int size;
	char line_data[DATA_SIZE] = {0};

	printf("Start log ...\n");

	//r_fd = open(KMSG_PATH, O_RDONLY | O_NONBLOCK);
	r_fd = open(KMSG_PATH, O_RDONLY);
	if (r_fd < 0) {
		printf("open %s failed\n", KMSG_PATH);
		return -1;
	}
	printf("kmsg fd = %d\n", r_fd);

	w_fd = open(LOG_FILE_PATH, O_RDWR | O_CREAT | O_TRUNC);
	printf("log fd = %d\n", w_fd);
	if (w_fd < 0) {
		printf("open %s failed\n", LOG_FILE_PATH);
		goto open_log_file_err;
	}

	while (1) {
		size = read(r_fd, line_data, DATA_SIZE);
		printf("size = %d", size);
		if (size > 0) {
			ret = write(w_fd, line_data, size);
			if (ret < 0) {
				printf("write log to %s failed\n", LOG_FILE_PATH);
				goto write_log_file_err;
			}
		}

		//fseek(r_fp, 0, SEEK_CUR);
		//usleep(SLEEP_TIME);
	}

	printf("End log...\n");

write_log_file_err:
	close(w_fd);
open_log_file_err:
	close(r_fd);

	return ret;
}

