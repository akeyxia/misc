#include <stdio.h>
#include <stdlib.h>
#include <sys/klog.h>

int main(int argc, char *argv[])
{
	int ret;
	char buffer[4096] = { 0 };
	float ms = 5.0;

	if (argc == 2) {
		ms = atof(argv[1]);
	} else {
		printf("usage: %s ms\n", argv[0]);
	}

	printf("wait time: %.6f ms\n", ms);

	while(1) {
		if((ret = klogctl(9, buffer, sizeof(buffer))) > 0) {
			if((ret = klogctl(2, buffer, sizeof(buffer))) > 0) {
				buffer[ret] = '\0';
				printf("len: %d, kernel: %s", ret, buffer);
			}
		}
		usleep(ms * 1000);
	}

	return 0;
}

