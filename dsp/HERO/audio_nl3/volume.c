#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "frac2hex.c"

#define FixedBoost	48.16
#define MainVolMax	12.04
#define MaxLoudBoost	10

/*
* Input: P(Primary), S(Secondary)
* Output coefficient: Y:Vol_Main1<n>, Y:Vol_Main2<n>
*/
void MainVolume(float min, float max, float step)
{
	float VoldB, Vol_Main1, Vol_Main2;

	printf("Primary and Secondary Volume: \n");
	VoldB = min;
	while(VoldB <= max) {
		if (VoldB >= 0) {
			Vol_Main1 = 1.0f;
			Vol_Main2 = pow10f((VoldB-MainVolMax)/20.0f);
		} else if (-MaxLoudBoost <= VoldB && VoldB < 0) {
			Vol_Main1 = pow10f(VoldB/20.0f);
			Vol_Main2 = pow10f(-1.0f*MainVolMax/20.0);
		} else if ((-FixedBoost+MainVolMax-MaxLoudBoost) <= VoldB
			&& VoldB < -MaxLoudBoost) {
			Vol_Main1 = pow10f(-1.0f*MaxLoudBoost/20.0);
			Vol_Main2 = pow10f((VoldB+MaxLoudBoost-MainVolMax)/20.0f);
		} else {
			Vol_Main1 = 256.0f*pow10f((VoldB-MainVolMax)/20.0);
			Vol_Main2 = 1.0f/256;
		}
		
		//printf("[%+8.4f] Vol_Main1 = %.4f, Vol_Main2 = %.4f --> 0x%03X,0x%03X\n",
		//	VoldB, Vol_Main1, Vol_Main2, YMEM2HEX(Vol_Main1), YMEM2HEX(Vol_Main2));
		printf("0x%03X, 0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(Vol_Main1), YMEM2HEX(Vol_Main2), VoldB);

		VoldB += step;
	}
}

/*
 * Translate the dB volume to Y:Vol_Nav & Vol_Phone
 */
void Navi_Phone_Volume(float min, float max, float step)
{
	int Nav;
	float volume;

	printf("Navi & Phone Channel Volume:\n");

	volume = min;
	while (volume <= max) {
		Nav = YMEM2HEX(-0.25f * powf(10, volume / 20));

		printf("0x%03X, /* %6.2f dB */\n", Nav, volume);
		volume += step;
	}
}

int main(int argc, char *argv[])
{
	float VoldB;

	if (argc == 2) {
		VoldB = atof(argv[1]);
		MainVolume(VoldB, VoldB, 1.0);
	} else {
		MainVolume(-92.0f, 12.0f, 0.25f);
		MainVolume(-92.0f, 12.0f, 1.0f);
		Navi_Phone_Volume(-54.0f, 12.0f, 1.0f);
	}

	return 0;
}

