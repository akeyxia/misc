#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "core.c"
#include "Audio_7741.h"
#include "frac2hex.c"

#define FS	44100
#define ParametricEqualizerMaxdb	12

/*
char *_ptr_cmd;
char *_ptr_token;
char *_ptr_save;
#define TO_SEP_CMD			(_ptr_token = strtok_r(_ptr_cmd, ", ", &_ptr_save))
#define GET_PARAM_D(defparam)		(TO_SEP_CMD ? strtoll(_ptr_token, NULL, 10) : defparam)
#define GET_PARAM_H(defparam)		(TO_SEP_CMD ? strtoll(_ptr_token, NULL, 16) : defparam)
#define GET_PARAM_F(defparam)		(TO_SEP_CMD ? strtof(_ptr_token, NULL) : defparam)
#define GET_OPCODE(defparam)		GET_PARAM_H(defparam)
#define GET_PARAM(defparam)		GET_PARAM_H(defparam)
*/
char *_ptr_cmd;
char *_ptr_token;
char *_ptr_save;
#define TO_SEP_CMD			(_ptr_token = strtok_r(_ptr_cmd, ", ", &_ptr_save))
#define GET_PARAM_D(defparam)		(TO_SEP_CMD ? strtoll(_ptr_token, NULL, 10) : defparam)
#define GET_PARAM_H(defparam)		(TO_SEP_CMD ? strtoll(_ptr_token, NULL, 16) : defparam)
#define GET_PARAM_F(defparam)		(TO_SEP_CMD ? strtof(_ptr_token, NULL) : defparam)
#define GET_OPCODE(defparam)		GET_PARAM_H(defparam)
#define GET_PARAM(defparam)		GET_PARAM_H(defparam)

typedef enum SAF7741_Channel {
	FrontLeft = 0,
	FrontRight,
	RearLeft,
	RearRight,
} SAF7741_Channel_t;

static unsigned short g_peq_para[4][70] = { 0 };
static SAF7741_Channel_t g_channel = FrontLeft;
static int g_fc[4][7] = { 0 };
static float g_gain[4][7] = { 0 };
static float g_Q[4][7] = { 0 };

static int SAF7741_ConfigurePeqPreset(SAF7741_Channel_t channel, int band)
{
	unsigned int data;
	unsigned short (*preset)[71];
	int retVal, i, j, k;

        unsigned int start_addr[4][7] = {
                {
                        ADSP_Y_EqFL_b10L_REL,
                        ADSP_Y_EqFL_b20L_REL,
                        ADSP_Y_EqFL_b30L_REL,
                        ADSP_Y_EqFL_b40L_REL,
                        ADSP_Y_EqFL_b50L_REL,
                        ADSP_Y_EqFL_b60L_REL,
                        ADSP_Y_EqFL_b70L_REL,
                },
                {
                        ADSP_Y_EqFR_b10L_REL,
                        ADSP_Y_EqFR_b20L_REL,
                        ADSP_Y_EqFR_b30L_REL,
                        ADSP_Y_EqFR_b40L_REL,
                        ADSP_Y_EqFR_b50L_REL,
                        ADSP_Y_EqFR_b60L_REL,
                        ADSP_Y_EqFR_b70L_REL,
                },
                {
                        ADSP_Y_EqRL_b10L_REL,
                        ADSP_Y_EqRL_b20L_REL,
                        ADSP_Y_EqRL_b30L_REL,
                        ADSP_Y_EqRL_b40L_REL,
                        ADSP_Y_EqRL_b50L_REL,
                        ADSP_Y_EqRL_b60L_REL,
                        ADSP_Y_EqRL_b70L_REL,
                },
                {
                        ADSP_Y_EqRR_b10L_REL,
                        ADSP_Y_EqRR_b20L_REL,
                        ADSP_Y_EqRR_b30L_REL,
                        ADSP_Y_EqRR_b40L_REL,
                        ADSP_Y_EqRR_b50L_REL,
                        ADSP_Y_EqRR_b60L_REL,
                        ADSP_Y_EqRR_b70L_REL,
                },
        };

	unsigned int coef_num[7] = {
		7, /* Band1 3 double precision coef + 1 gain */
		7, /* Band2 3 double precision coef + 1 gain */
		7, /* Band3 3 double precision coef + 1 gain */
		7, /* Band4 3 double precision coef + 1 gain */
		10, /* Band5 5 double precision coef */
		10, /* Band6 5 double precision coef */
		10, /* Band7 5 double precision coef */
	};

	for (j = 0; j < 10; j++) {
		retVal = saf7741_WriteDSPMem(I2CDRV_Y_MEM, ADSP_Y_UpdatC_Coeff0 + j,
					g_peq_para[channel][band * 10 + j]);
		if (retVal < 0)
			printf("%s: Failed to write ADSP_Y_UpdatC_Coeff%d %d\n", __func__,
				j - 1, retVal);
	}
	
	retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM,
	ADSP_X_SrcSw_OrDigSrcSelMask, (start_addr[channel][band]));
	if (retVal < 0)
		printf("%s: Failed to write ADSP_X_UpdatC_StartYaddr %d\n", __func__,
			retVal);

	retVal = saf7741_WriteDSPMem(I2CDRV_EP_MEM, 0, coef_num[band]);
	if (retVal < 0)
		printf("%s: Failed to EasyProgramming %d\n", __func__,
			retVal);

	return 0;
}

static void DB2Val_7BandPeq(int band, int fc, float gain, float Q)
{
	float G, t, D, Gmax, b;
	double a2d, a1d, b0d, b1d, b2d;
	int gc, a2l, a2h, a1l, a1h, b0l, b0h, b1l, b1h, b2l, b2h;

/*
	if (gain > ParametricEqualizerMaxdb || gain < -ParametricEqualizerMaxdb) {
		printf("%s: gain %d-%d is supported\n", __func__,
				-ParametricEqualizerMaxdb, ParametricEqualizerMaxdb);
		return;
	}
*/
	if (Q < 0.1f || Q > 10.0f) {
		printf("%s: Qfactor 0.1-10.0 is supported\n", __func__);
		return;
	}

	switch(band) {
	case 1:
	case 2:
	case 3:
        case 4:
                G = (powf(10, gain / 20.0f) - 1) / 4;
                t = 2.0f * M_PI * fc / FS;
                if (gain >= 0)
                        D = 1.0f;
                else
                        D = 1 / (4 * G + 1);
		a2d = -0.5 * (1 - D * t / 2 / Q) / (1 + D * t / 2 / Q);
		a1d = (0.5 - a2d) * cos(t);
		b0d = (0.5 + a2d) / 2;

		gc = YMEM2HEX(G);
		b0h = DYMEM2HEX(b0d) >> 12;
		b0l = DYMEM2HEX(b0d) & 0xFFF;
		a1h = DYMEM2HEX(a1d) >> 12;
		a1l = DYMEM2HEX(a1d) & 0xFFF;
		a2h = DYMEM2HEX(a2d) >> 12;
		a2l = DYMEM2HEX(a2d) & 0xFFF;

		break;
	case 5:
	case 6:
	case 7:
                Gmax = 20.0f * log10f(2.0f + Q * FS / M_PI / fc);
                G = powf(10, gain / 20.0f);
                t = 2.0f * M_PI * fc / FS;
                if (G >= 1.0f)
                        b = t / (2 * Q);
                else
                        b = t / (2 * G * Q);

		a2d = -0.5 * (1 - b) / (1 + b);
		a1d = (0.5 - a2d) * cos(t);
		b0d = (G - 1) * (0.25 + 0.5 * a2d) + 0.5;
                b1d = -a1d;
                b2d = -(G - 1) * (0.25 + 0.5 * a2d) - a2d;

		gc = YMEM2HEX(G);
		b0h = DYMEM2HEX(b0d) >> 12;
		b0l = DYMEM2HEX(b0d) & 0xFFF;
                b1h = DYMEM2HEX(b1d) >> 12;
		b1l = DYMEM2HEX(b1d) & 0xFFF;
                b2h = DYMEM2HEX(b2d) >> 12;
		b2l = DYMEM2HEX(b2d) & 0xFFF;
		a1h = DYMEM2HEX(a1d) >> 12;
		a1l = DYMEM2HEX(a1d) & 0xFFF;
		a2h = DYMEM2HEX(a2d) >> 12;
		a2l = DYMEM2HEX(a2d) & 0xFFF;

		break;
	default:
		break;
	}

	if (band <= 4) {
		printf("0x1004, %d, %5d, %3.1f, %4.2f, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x000, 0x000, 0x000, /* band %d(%d) %3.1fdB %4.2f */\n",
			band, fc, gain, Q, b0l, b0h, a2l, a2h, a1l, a1h, gc, band, fc, gain, Q);
		g_peq_para[g_channel][(band - 1)*10 + 0] = b0l;
		g_peq_para[g_channel][(band - 1)*10 + 1] = b0h;
		g_peq_para[g_channel][(band - 1)*10 + 2] = a2l;
		g_peq_para[g_channel][(band - 1)*10 + 3] = a2h;
		g_peq_para[g_channel][(band - 1)*10 + 4] = a1l;
		g_peq_para[g_channel][(band - 1)*10 + 5] = a1h;
		g_peq_para[g_channel][(band - 1)*10 + 6] = gc;
	} else {
		printf("0x1004, %d, %5d, %3.1f, %4.2f, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, 0x%03X, /* band %d(%d) %3.1fdB max %.2fdB %4.2f*/\n",
			band, fc, gain, Q, b0l, b0h, b1l, b1h, b2l, b2h, a1l, a1h, a2l, a2h, band, fc, gain, Gmax, Q);
		g_peq_para[g_channel][(band - 1)*10 + 0] = b0l;
		g_peq_para[g_channel][(band - 1)*10 + 1] = b0h;
		g_peq_para[g_channel][(band - 1)*10 + 2] = b1l;
		g_peq_para[g_channel][(band - 1)*10 + 3] = b1h;
		g_peq_para[g_channel][(band - 1)*10 + 4] = b2l;
		g_peq_para[g_channel][(band - 1)*10 + 5] = b2h;
		g_peq_para[g_channel][(band - 1)*10 + 6] = a1l;
		g_peq_para[g_channel][(band - 1)*10 + 7] = a1h;
		g_peq_para[g_channel][(band - 1)*10 + 8] = a2l;
		g_peq_para[g_channel][(band - 1)*10 + 9] = a2h;
	}

	SAF7741_ConfigurePeqPreset(g_channel, band - 1);
}

static void restore_peq2flat(int channel, int band)
{
	if (g_gain[channel][band - 1] != 0) {
		DB2Val_7BandPeq(band, g_fc[channel][band - 1], 0, g_Q[channel][band - 1]);
			g_gain[channel][band - 1] = 0;
	}

	return;
}

const char *channel_strops(SAF7741_Channel_t channel)
{
#define CHNO2STR(_c) case _c: return # _c

	switch (channel) {
	CHNO2STR(FrontLeft);
	CHNO2STR(FrontRight);
	CHNO2STR(RearLeft);
	CHNO2STR(RearRight);
	default: return "unknow channel !!!";
	}
}

void opcode_help(void)
{
	printf("\n/**************************** CMD Comment *******************************/\n");
	printf("Exit:			0x1000\n");
	printf("Get Current Channel:	0x1001\n");
	printf("Select Channel:		0x1002 xxx\n");
	printf("Print PEQ parameters:	0x1003\n");
	printf("Set PEQ:		0x1004 band fc gain Q xxx xxx ... xxx(10 parameters)\n");
	printf("Calc and set PEQ:	0x1005 band fc gain Q\n");
	printf("Restore bandX:		0x1006 band\n");
	printf("Restore all band on current channel:	0x1007\n");
	printf("Restore PEQ:		0x1008\n");
	printf("/*******************************************************************/\n");
}

int main(int argc, char *argv[])
{
	int opcode;
	int i, j;
	char str[512];
	char buf[64];
	int band;

	while (1) {
		fgets(str, sizeof(str), stdin);
		_ptr_cmd = str;
		opcode = GET_OPCODE(0x999);
		_ptr_cmd = NULL;

		switch(opcode) {
		case 0x1000:
			exit(0);
			break;
		case 0x1001:
			printf("Current Chanel: %s\n", channel_strops(g_channel));
			break;
		case 0x1002:
			g_channel = GET_PARAM_D(0);
			printf("Select Chanel: %s\n", channel_strops(g_channel));
			break;
		case 0x1003:
			for (i = 0; i < 4; i++) {
				printf("/********** Channel: %s **********/\n", channel_strops(i));
				for (j = 1; j <= 70; j++)
					printf("0x%03X, %s", g_peq_para[i][j - 1], (j % 10 == 0) ? "\n" : "");
			}

			for (i = 0; i < 4; i++) {
				printf("/********** Channel: %s **********/\n", channel_strops(i));
				for (j = 1; j <= 70; j++) {
					printf("0x%03X, ", g_peq_para[i][j - 1]);
					if (j % 10 == 0) {
						band = j / 10 - 1;
						memset(buf, 0x00, sizeof(buf));
						sprintf(buf, "/* band %d(%d) %3.1fdB %4.2f */",
							band + 1, g_fc[i][band], g_gain[i][band], g_Q[i][band]);
						printf("%s\n", buf);
					}
				}
			}
			break;
		case 0x1004:
			band = GET_PARAM_D(0);
			g_fc[g_channel][band - 1] = GET_PARAM_D(0);
			g_gain[g_channel][band - 1] = GET_PARAM_F(0);
			g_Q[g_channel][band - 1] = GET_PARAM_F(0);

			printf("PEQ settings for channel: %s\n", channel_strops(g_channel));
			printf("band = %d, g_fc = %d, g_gain = %3.1f, g_Q = %4.2f\n",
				band, g_fc[g_channel][band - 1], g_gain[g_channel][band - 1], g_Q[g_channel][band - 1]);

			g_peq_para[g_channel][(band - 1)*10 + 0] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 1] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 2] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 3] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 4] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 5] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 6] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 7] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 8] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band - 1)*10 + 9] = GET_PARAM_H(0);

			SAF7741_ConfigurePeqPreset(g_channel, band - 1);
			break;
		case 0x1005:
			band = GET_PARAM_D(0);
			g_fc[g_channel][band - 1] = GET_PARAM_D(0);
			g_gain[g_channel][band - 1] = GET_PARAM_F(0);
			g_Q[g_channel][band - 1] = GET_PARAM_F(0);

			printf("Calculate and set PEQ parameters for channel: %s\n", channel_strops(g_channel));
			printf("band = %d, g_fc = %d, g_gain = %3.1f, g_Q = %4.2f\n",
				band, g_fc[g_channel][band - 1], g_gain[g_channel][band - 1], g_Q[g_channel][band - 1]);

			DB2Val_7BandPeq(band, g_fc[g_channel][band - 1], g_gain[g_channel][band - 1], g_Q[g_channel][band - 1]);
			break;
		case 0x1006:
			band = GET_PARAM_D(0);
			printf("Restore band %d to Flat on channel %s\n", band, channel_strops(g_channel));

			restore_peq2flat(g_channel, band);
			break;
		case 0x1007:
			printf("Restore all band to Flat on channel %s\n", channel_strops(g_channel));

			for (band = 1; band <= 7; band++)
				restore_peq2flat(g_channel, band);
			break;
		case 0x1008:
			printf("Restore PEQ to Flat on All channel\n");

			for (i = 0; i < 4; i++)
				for (band = 1; band <= 7; band++)
					restore_peq2flat(i, band);
			break;
		default:
			printf("unknow opcode\n");
			opcode_help();
			break;
		}
	}
	
	
	return 0;
}
