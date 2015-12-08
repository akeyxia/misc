#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static float FmNfd_AmNfd_xRssi_to_dbuV(int value)
{
	float RfLevel, femtowatt, uV, dBuV;

	RfLevel = 80.0f * value /  8388608.0f + 4;
	femtowatt = pow10f(RfLevel / 10.0f);
	uV = powf(femtowatt * 50.0f / 1000.0f, 0.5);
	dBuV = 20.0f * log10f(uV);

	printf("--> FM: RfLevel = %f, femtowatt = %f, uV = %f, dBuV = %f\n",
				RfLevel, femtowatt, uV, dBuV);

	RfLevel = 80.0f * value /  8388608.0f + 10;
	femtowatt = pow10f(RfLevel / 10.0f);
	uV = powf(femtowatt * 50.0f / 1000.0f, 0.5);
	dBuV = 20.0f * log10f(uV);

	printf("--> AM: RfLevel = %f, femtowatt = %f, uV = %f, dBuV = %f\n",
				RfLevel, femtowatt, uV, dBuV);

	return dBuV;
}

int main(int argc, char *argv[])
{
	int xRssi;

	if (argc != 2) {
		printf("usage: ./a.out xRssi\n");
		return -1;
	}

	xRssi = strtol(argv[1], NULL, 16);
	printf("xRssi = 0x%X\n", xRssi);
	FmNfd_AmNfd_xRssi_to_dbuV(xRssi);

	return 0;
}

