#include <stdio.h>
#include <stdlib.h>

unsigned int power(int n)
{
	unsigned int i, val = 1;

	for (i = 0; i < n; i++)
		val *= 2;

	return val;
}

int main(int argc, char *argv[])
{
	unsigned int val, bit;
	float cof = 1.0;

	if (argc != 3) {
		printf("usage: ./a.out xx(11/23) xxx\n");
		return 1;
	}

	bit = atoi(argv[1]);
	val = strtol(argv[2], NULL, 16);

	switch(bit) {
	case 11:
		if (val > 0x7ff) {
			val = 0x1000 - val;
			printf("cof = %f\n", -1.0*((float)val/power(bit)));
		} else {
			printf("cof = %f\n", ((float)val/power(bit)));
		}

		break;
	case 23:
		if (val > 0x7fffff) {
			val = 0x1000000 - val;
			printf("cof = %f\n", -1.0*((float)val/power(bit)));
		} else {
			printf("cof = %f\n", ((float)val/power(bit)));
		}

		break;
	default:
		break;
	}

	return 0;
}

