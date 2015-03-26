#include <stdio.h>
#include <math.h>

#include "frac2hex.c"

#define ToneBoostMaxP	0
#define GeqMaxResp	0	//share with PeqMaxResp
#define PeqMaxResp	12.04
#define MainVolMax	12.04
#define FixedBoost	48.16
#define FixedBoost_60dB 60.16
#define ChanGainPMax	0
#define SrcScaler	12.04

/*
* Input: n=<F, R, SwL, SwR>, m=<L, R>
* Output coefficient: Y:Vol_UpScal<n>, Y:Vol_ChanGainPF<m>
*/
void PostScales(void)
{
	float CompN;

	/* Comp<n>=MainVolMax + GeqMaxResp + PeqMaxResp<n> + ToneBoostMaxP + ChanGainPMax */
	CompN = MainVolMax + GeqMaxResp + PeqMaxResp + ToneBoostMaxP + ChanGainPMax + SrcScaler;

	//CompN = 38.12;
	printf("0x%03X, /* %6.2f dB */\n",
		YMEM2HEX(pow10f((CompN - FixedBoost)/20.0)), FixedBoost-CompN);

}

void PostScales_60dB(void)
{
	float CompN;

	/* Comp<n>=MainVolMax + GeqMaxResp + PeqMaxResp<n> + ToneBoostMaxP + ChanGainPMax */
	CompN = MainVolMax + GeqMaxResp + PeqMaxResp + ToneBoostMaxP + ChanGainPMax + SrcScaler;

	//CompN = 38.12;
	printf("0x%03X, /* %6.2f dB */\n",
		YMEM2HEX(pow10f((CompN - FixedBoost_60dB)/20.0)), FixedBoost_60dB-CompN);

}

void PostScales_FixMainVolMax(float min, float max, float step)
{
	float value;
	float CompN;

	if (max > (FixedBoost - MainVolMax))
		max = FixedBoost - MainVolMax;

	printf("%s: \n", __func__);

	value = min;
	while (value <= max) {
		CompN = value + MainVolMax + 0.08;
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f((CompN - FixedBoost)/20.0)), FixedBoost-CompN);

		value += step;
	}
}

void PostScales_60dB_FixMainVolMax(float min, float max, float step)
{
	float value;
	float CompN;

	if (max > (FixedBoost_60dB - MainVolMax))
		max = FixedBoost_60dB - MainVolMax;

	printf("%s: \n", __func__);

	value = min;
	while (value <= max) {
		CompN = value + MainVolMax + 0.08;
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f((CompN - FixedBoost_60dB)/20.0)), FixedBoost_60dB-CompN);

		value += step;
	}
}

void PostScales_Debug(float step)
{
	float value;
	float CompN;
	float fixed_boost;

	printf("%s: \n", __func__);

	printf("/****** FixedBoost = 48dB ******/\n");
	fixed_boost = 48.0f;
	value = 0.0f;
	while (value <= fixed_boost) {
		CompN = value;
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f((CompN - fixed_boost)/20.0)), fixed_boost - CompN);

		value += step;
	}

	printf("/****** FixedBoost = 60dB ******/\n");
	fixed_boost = 60.0f;
	value = 0.0f;
	while (value <= fixed_boost) {
		CompN = value;
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f((CompN - fixed_boost)/20.0)), fixed_boost - CompN);

		value += step;
	}
}

int main(int argc, char *argv[])
{
	PostScales_Debug(1.0f);
	PostScales();
	PostScales_FixMainVolMax(0, (int)(FixedBoost - MainVolMax), 1.0);

	PostScales_60dB();
	PostScales_60dB_FixMainVolMax(0, (int)(FixedBoost_60dB - MainVolMax), 1.0);
	return 0;
}

