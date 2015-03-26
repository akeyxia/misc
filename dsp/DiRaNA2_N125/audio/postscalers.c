#include <stdio.h>
#include <math.h>

#include "frac2hex.c"

#define ToneBoostMaxP	7
#define GeqMaxResp	12.04	//share with PeqMaxResp
#define PeqMaxResp	10
#define MainVolMax	12.04
#define FixedBoost	48.16
#define ChanGainPMax	6

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

int main(int argc, char *argv[])
{
	PostScales();
	PostScales_FixMainVolMax(0, (int)(FixedBoost - MainVolMax), 1.0);

	return 0;
}

