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
	float phi0;
	float phi1;

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
		phi0 = (float)(300*1.5)*32/41600;
		phi1 = (float)(-300*0.5)*32/41600;
		printf("fm+150: mix0 = 0x%x(%f), mix1 = 0x%x(%f)\n",
			(int)((2-phi0)*power(bit)), phi0,
			(int)((2+phi1)*power(bit)), phi1);

                phi0 = (float)(300*0.5)*32/41600;
                phi1 = (float)(-300*1.5)*32/41600;
                printf("fm-150: mix0 = 0x%x(%f), mix1 = 0x%x(%f)\n",
                       (int)((2-phi0)*power(bit)), phi0,
                       (int)((2+phi1)*power(bit)), phi1);

		//phi0 = (float)(56.5*1.5)*32/41600;
		//phi1 = (float)(-56.5*0.5)*32/41600;
		phi0 = (float)(val+56.5)*32/41600;
		phi1 = (float)(val-56.5)*32/41600;
		printf("am+56.5/2: mix0 = 0x%x(%f), mix1 = %x(%f)\n",
			(int)((2-phi0)*power(bit)), phi0,
			(int)((2+phi1)*power(bit)), phi1);

                //phi0 = (float)(56.5*0.5)*32/41600;
                //phi1 = (float)(-56.5*1.5)*32/41600;
                phi0 = (float)(56.5-val)*32/41600;
		phi1 = (float)(-56.5-val)*32/41600;
		printf("am-56.5/2: mix0 = 0x%x(%f), mix1 = %x(%f)\n",
                        (int)((2-phi0)*power(bit)), phi0,
                        (int)((2+phi1)*power(bit)), phi1);

		break;
	default:
		break;
	}

	return 0;
}

