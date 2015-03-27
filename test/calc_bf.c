#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
	int i;
	double val;

	for (i = 0; i <= 66; i++) {
		val = 1.0;
		val *= 1/pow(10.0, (double)i/20);
		printf("val = %f, 0x%x, %d\n", val, 0x1000-(int)(val*2048+0.5), i);
	}

	return 0;
}

