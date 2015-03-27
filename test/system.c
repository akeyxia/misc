#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	printf("[%s] Start\n", __FILE__);
	system("ls");
	system("busybox ls");
	printf("[%s] End\n", __FILE__);

	return 0;
}

