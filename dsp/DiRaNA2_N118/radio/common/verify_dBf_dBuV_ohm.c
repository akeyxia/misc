#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static float dBf_to_dbuV(float value, float ohm)
{
	float RfLevel, femtowatt, uV, dBuV;

	RfLevel = value;//80.0f * value /  8388608.0f + 4;
	femtowatt = pow10f(RfLevel / 10.0f);
	uV = powf(femtowatt * ohm / 1000.0f, 0.5);
	dBuV = 20.0f * log10f(uV);

	printf("--> RfLevel = %f, femtowatt = %f, uV = %f, dBuV = %f\n",
				RfLevel, femtowatt, uV, dBuV);

	return dBuV;
}

int main(int argc, char *argv[])
{
	float dBf, ohm;

	if (argc != 3) {
		printf("usage: ./a.out dBf ohm\n");
		return -1;
	}

	dBf = atof(argv[1]);
	ohm = atof(argv[2]);
	printf("dBf = %f, ohm = %f\n", dBf, ohm);
	dBf_to_dbuV(dBf, ohm);

	return 0;
}

