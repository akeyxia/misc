#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mem2hex.c"

float highcut_alignment(float freq, float attn)
{
	float frac;
	float f1, f2, f3;
	
	f1 = 1.0f - cosf(2.0f * M_PI * freq / 40625.0f);
	f2 = powf(10.0f, -2.0f * attn / 20.0f);
	f3 = powf(1.0f - f2, 2.0f);
	frac = (f1 * f2 - powf(f1 * f2 - f3, 0.5f)) / f3;

	printf("f1 = %f, f2 = %f, f3 = %f\n", f1, f2, f3);
	printf("freq = %f, attn = %f, frac = %f\n", freq, attn, frac);

	return frac;
}

float new_highcut_alignment(float freq, float attn)
{
	float frac;
	float f1, f2, f3;

/*
=1-(2*(1-COS(2*PI()*K$5/40.625)*10^(-K11/20)^2)-SQRT(4*(1-COS(2*PI()*K$5/40.625)*10^(-K11/20)^2)^2-4*(1-10^(-K11/20)^2)^2))/2/(1-10^(-K11/20)^2)
=1-(2*(1-f1*f2)-SQRT(4*(1-f1*f2)^2-4*(1-f2)^2))/2/(1-f2)
=1-(2*(1-f1*f2)-SQRT(4*(1-f1*f2)^2-4*f3))/2/(1-f2)
*/
	f1 = cosf(2.0f * M_PI * freq / 40625.0f);
	f2 = powf(10.0f, -2.0f * attn / 20.0f);
	f3 = powf(1.0f - f2, 2.0f);
	frac = 1.0f - (2.0f * (1.0f - f1 * f2) - sqrt(4.0f * powf(1.0f - f1 * f2, 2.0f) - 4.0f * f3)) / 2.0f / (1.0f - f2);

	printf("f1 = %f, f2 = %f, f3 = %f\n", f1, f2, f3);
	printf("freq = %f, attn = %f, frac = %f\n", freq, attn, frac);

	return frac;
}

int main(int argc, char *argv[])
{
	float fc;
	float attn;
	float frac;

	if (argc != 3) {
		printf("usage: ./xxx fc attn\n");
		return -1;
	}

	fc = atof(argv[1]);
	attn = atof(argv[2]);
	frac = highcut_alignment(fc, attn);
	printf("Yhighcut = 0x%X\n", YMEM2Hex(frac));
	frac = new_highcut_alignment(fc, attn);
	printf("Yhighcut = 0x%X\n", YMEM2Hex(frac));

	return 0;
}


