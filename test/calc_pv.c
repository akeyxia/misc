#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define FixedBoost	48.16
#define MainVolMax	12.04
#define MaxLoudBoost	10

int main(int argc, char *argv[])
{
	int i;
	double val;
	double VoldB;
	double Vol_Main1;
	double Vol_Main2;

/*
	for (i = -102; i <= -60; i++) {
		val = 1.0;
		val *= 256*pow(10.0, (double)(i-12)/20);
		printf("val = %f, 0x%x, %d\n", val, 0x1000-(int)(val*2048+0.5), i);
	}
*/

if (argc == 2) {
	VoldB = atof(argv[1]);
	printf("str = %s, VoldB = %lf\n", argv[1], VoldB);

	if (VoldB >= 0) { 
		Vol_Main1 = -1.0;
		Vol_Main2 = pow(10.0, (double)(VoldB-MainVolMax)/20);
	} else if (-MaxLoudBoost <= VoldB && VoldB < 0) {
		Vol_Main1 = -1.0*pow(10.0, (double)VoldB/20);
		Vol_Main2 = pow(10.0, (double)(-1*MainVolMax)/20);
	} else if ((-FixedBoost+MainVolMax-MaxLoudBoost) <= VoldB
		&& VoldB < -MaxLoudBoost) {
		Vol_Main1 = -1.0*pow(10.0, (double)(-1*MaxLoudBoost)/20);
		Vol_Main2 = pow(10.0, (double)(VoldB+MaxLoudBoost-MainVolMax)/20);
	} else {
		Vol_Main1 = -256.0*pow(10.0, (double)(VoldB-MainVolMax)/20);
		Vol_Main2 = (double)1.0/256;
	}
          
	printf("[%3.4f] Vol_Main1 = %6f, Vol_Main2 = %6f --> 0x%03x,0x%03x\n",
			VoldB, Vol_Main1, Vol_Main2,
			Vol_Main1 > 0 ? (int)(Vol_Main1*2048+0.5) : 0x1000+(int)(Vol_Main1*2048-0.5),
			Vol_Main2 > 0 ? (int)(Vol_Main2*2048+0.5) : 0x1000+(int)(Vol_Main2*2048-0.5));
} else
	for (VoldB = -102; VoldB <= MainVolMax; VoldB++) {
		if (VoldB >= 0) {
			Vol_Main1 = -1;
			Vol_Main2 = pow(10.0, (double)(VoldB-MainVolMax)/20);
		} else if (-MaxLoudBoost <= VoldB && VoldB < 0) {
			Vol_Main1 = -1*pow(10.0, (double)VoldB/20);
			Vol_Main2 = pow(10.0, (double)(-1*MainVolMax)/20);
		} else if ((-FixedBoost+MainVolMax-MaxLoudBoost) <= VoldB
			&& VoldB < -MaxLoudBoost) {
			Vol_Main1 = -1*pow(10.0, (double)(-1*MaxLoudBoost)/20);
			Vol_Main2 = pow(10.0, (double)(VoldB+MaxLoudBoost-MainVolMax)/20);
		} else {
			Vol_Main1 = -256*pow(10.0, (double)(VoldB-MainVolMax)/20);
			Vol_Main2 = (double)1/256;
		}

		printf("[%3.4f] Vol_Main1 = %6f, Vol_Main2 = %6f --> 0x%03x,0x%03x\n",
			VoldB, Vol_Main1, Vol_Main2,
			Vol_Main1 > 0 ? (int)(Vol_Main1*2048+0.5) : 0x1000+(int)(Vol_Main1*2048-0.5),
			Vol_Main2 > 0 ? (int)(Vol_Main2*2048+0.5) : 0x1000+(int)(Vol_Main2*2048-0.5));
	}

	return 0;
}

