#include <stdio.h>
#include <stdlib.h>

/*
 * CLKOUTLDO = [CLKINP x M / (N + 1)] / M2
 */
float calc_CM_CLKSEL_DPLL_PCIE_REF_OUT(float clkin, int m, int n, int m2)
{
	float clkout;

	clkout = (clkin * m / (n + 1)) / m2;

	printf("clkin = %f, m = %d, n = %d, m2 = %d, clkout = %f\n",
				clkin, m, n, m2, clkout);

	return clkout;
}

/*
 * CLKINP = (CLKOUTLDO * M2) * (N + 1) / M
 *
 */
float calc_CM_CLKSEL_DPLL_PCIE_REF_IN(float clkout, int m, int n, int m2)
{
	float clkin;

	clkin = (clkout * m2) * (n + 1) / m;

	printf("clkout = %f, m = %d, n = %d, m2 = %d, clkin = %f\n",
				clkout, m, n, m2, clkin);

	return clkin;
}


int main(int argc, char *argv[])
{
	float arg1;
	int arg2, arg3, arg4;
	int inout;

	if (argc != 6) {
		printf("usage: %s 0/1 clkin/clkout m n m2\n", argv[0]);
		return -1;
	}

	inout = atoi(argv[1]);
	arg1 = atof(argv[2]);
	arg2 = atoi(argv[3]);
	arg3 = atoi(argv[4]);
	arg4 = atoi(argv[5]);

	if (inout == 0)
		calc_CM_CLKSEL_DPLL_PCIE_REF_IN(arg1, arg2, arg3, arg4);
	else if (inout == 1)
		calc_CM_CLKSEL_DPLL_PCIE_REF_OUT(arg1, arg2, arg3, arg4);
	else
		printf("usage: %s 0/1 clkin/clkout m n m2\n", argv[0]);

	return 0;
}

