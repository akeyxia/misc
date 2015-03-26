#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mem2hex.c"

/*
3.1.1.2.1	Sensitivity
Register : AmNbk_yIfUsnImpLv
Formula : Increase value, increase sensitivity
                Decrease value, decrease sensitivity

3.1.1.2.2	Time Constants
Registers : AmNbk_yIfUsnAvAtk, AmNbk_yIfUsnAvDcy

Formula : AmNbk_yIfUsnAvAtk =  1.0f - expf(-1.0f / (tc * 81250.0f))
          AmNbk_yIfUsnAvDcy =  1.0f - expf(-1.0f / (tc * 81250.0f))

*/

void time(float tc)
{
	double fracAtk;
	double fracDcy;

	fracAtk = 1.0f - expf(-1.0f / (tc * 81250.0f));
	fracDcy = fracAtk;
	printf("tc = %f s, fracAtk = %f, fracDcy = %f\n", tc, fracAtk, fracDcy);
	printf("%s: tc = %f, AmNbk_yIfUsnAvAtk = 0x%X, AmNbk_yIfUsnDcy = 0x%X\n",
		__func__, tc, YMEM2Hex(fracAtk), YMEM2Hex(fracDcy));

	return;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: ./xxx tc\n");
		return -1;
	}

	time(atof(argv[1]));

	return 0;
}
