#include <stdio.h>
#include <math.h>
#include "Audio_7741.h"
#include "frac2hex.c"

/*
* Input: n=<F, R, SwL, SwR>, m=<L, R>
* Output coefficient: Y:Vol_UpScal<n>, Y:Vol_ChanGainPF<m>
*/
void PostScales(void)
{
	float CompN;

	/* Comp<n>=MainVolMax + GeqMaxResp + PeqMaxResp<n> + ToneBoostMaxP + ChanGainPMax */
	CompN = MainVolMax + GeqMaxResp + PeqMaxResp + ToneBoostMaxP + ChanGainPMax;

	//CompN = 38.12;
	printf("0x%03X, /* %6.2f dB */\n",
		YMEM2HEX(pow10f((CompN - FixedBoost)/20.0)), FixedBoost-CompN);

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
		CompN = value + MainVolMax;
		printf("0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f((CompN - FixedBoost)/20.0)), FixedBoost-CompN);

		value += step;
	}
}

/*
* Input: n=<L, R>
* Output coefficient: Y:Vol_UpScalS, Y:Vol_ChanGainS<n>
*/
void Secondary_PostScales(void)
{
	float CompS;
	float chanGain = ChanGainSMax;

	/* CompS = MainVolMax + ToneBoostMaxS + ChanGainSMax */
	CompS = MainVolMax + ToneBoostMaxS + ChanGainSMax;
	printf("MainVolMax = %6.2f, ToneBoostMaxS = %6.2f, ChanGainSMax = %6.2f\n",
				MainVolMax, ToneBoostMaxS, ChanGainSMax);

	printf("Y : Vol_UpScalS = 0x%03X, /* %6.2f dB */\n",
		YMEM2HEX(pow10f((CompS - FixedBoost)/20.0)), CompS - FixedBoost);

	if (chanGain <= ChanGainSMax)
		printf("Y : Vol_ChanGainS<n> = 0x%03X, /* %6.2f dB */\n",
			YMEM2HEX(pow10f((chanGain - ChanGainSMax)/20.0)), chanGain);
	else
	      printf("actual channel gain must small than ChanGainSMax(%6.2f)\n", ChanGainSMax);
}

int main(int argc, char *argv[])
{
	PostScales();
	PostScales_FixMainVolMax(0, (int)(FixedBoost - MainVolMax), 1.0);

	Secondary_PostScales();

	return 0;
}

