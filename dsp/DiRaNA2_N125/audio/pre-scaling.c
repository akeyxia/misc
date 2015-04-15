#include <stdio.h>
#include <math.h>
#include "Audio_7741.h"
#include "frac2hex.c"

#define GeqBoost        12

/*
* Input: GeqMaxResp, GeqBoost, ToneBoostMaxP
* Output coefficient: Y:Vol_DesScalGEq, X:Vol_OneOverMaxBoostP
*/
void GEQ_PreScaling(float min, float max, float step)
{
	float value;

	printf("X : Vol_OneOverMaxBoostP --> 0x%06X /* %ddB */\n",
		XMEM2HEX(pow10f(-1.0*(GeqMaxResp+ToneBoostMaxP)/20.0)), GeqMaxResp+ToneBoostMaxP);

	printf("GEQ frequency response Gain: \n");
	value = min;
	while (value <= max) {
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f(-1.0*value/20.0)), value);

		value += step;
	}	
}

/*
* Input: n=<F, R, SwL, SwR>
* Output coefficient: Y:Vol_Scal<n>
*/
void PEQ_PreScaling(float min, float max, float step)
{
	float value;

	printf("PEQ frequency response Gain: \n");
	value = min;
	while (value <= max) {
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f(-1.0*value/20.0)), value);

		value += step;
	}
}

/*
* Input: ToneBoostS(min, max), ToneBoostMaxS
* Output coefficient: Y:Vol_DesScalBMTS, X:Vol_OneOverMaxBoostS
*/
void Secondary_Tone_PreScaling(float min, float max, float step)
{
	float value;

	printf("X : Vol_OneOverMaxBoostS --> 0x%06X /* %ddB */\n",
		XMEM2HEX(pow10f(-1.0*ToneBoostMaxS/20.0)), ToneBoostMaxS);

	printf("Actual tone control boost: \n");
	value = min;
	while (value <= max) {
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f(-1.0*value/20.0)), value);

		value += step;
	}
}

int main(int argc, char *argv[])
{
	GEQ_PreScaling(-GeqBoost, GeqBoost, 1.0);
	PEQ_PreScaling(-12.0, 12.0, 1.0);

	Secondary_Tone_PreScaling(0, 0, 1);

	return 0;
}

