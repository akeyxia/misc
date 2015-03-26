#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "core.c"
#include "Audio_7741.h"
#include "frac2hex.c"

#define FS	44100
#define ParametricEqualizerMaxdb	12
#define GraphicEqualizerMaxdb	12
#define MAX_DELAY_LINES_MS	20
#define GEQ_BAND_MAX	5
#define PEQ_BAND_MAX	7

/* Channel Gain Settings */
#define ChannelGainMaxLevel	(18)
#define ChannelGainMaxdb	(6)
#define ChannelGain0dbLevel	(12)
#define ChannelGainPFLPresetdb	(-5)
#define ChannelGainPFRPresetdb	(-5)
#define ChannelGainPRLPresetdb	(-5)
#define ChannelGainPRRPresetdb	(-5)

#define dB2Index(db, zerodb_index) (zerodb_index + db)

#define pr_err(fmt, args...) \
do {\
        printf(fmt"\n", ##args); \
} while(0)

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
	ChannelMax,
} SAF7741_Channel_t;

static SAF7741_Channel_t g_channel = FrontLeft;
static unsigned int g_peq_para[ChannelMax][70] = { 0 };/* 4channel x 7band x 10 */
static int g_peq_fc[ChannelMax][PEQ_BAND_MAX] = { 0 };
static float g_peq_gain[ChannelMax][PEQ_BAND_MAX] = { 0 };
static float g_peq_Q[ChannelMax][PEQ_BAND_MAX] = { 0 };

static unsigned int g_geq_para[35] = { 0 };/* 5band x 7 */
static float g_geq_gain[GEQ_BAND_MAX] = { 0 };
static int g_geq_fc[GEQ_BAND_MAX] = { 0 };
static float g_geq_Q[GEQ_BAND_MAX] = { 0 };

static float g_delay[ChannelMax] = { 0 };
static int g_channel_gain[ChannelMax] = { 0 };
extern int g_debug_mask;

static int SAF7741_ConfigurePeqPreset(SAF7741_Channel_t channel, int band)
{
	unsigned int data;
	unsigned int (*preset)[71];
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
		retVal = saf7741_WriteDSPMem(I2CDRV_Y_MEM, ADSP_Y_UpdatC_Coeff0 + j, 1,
					&g_peq_para[channel][band * 10 + j]);
		if (retVal < 0)
			pr_err("%s: Failed to write ADSP_Y_UpdatC_Coeff%d %d\n", __func__,
				j, retVal);
	}
	
	retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM,
	ADSP_X_SrcSw_OrDigSrcSelMask, 1,&(start_addr[channel][band]));
	if (retVal < 0)
		printf("%s: Failed to write ADSP_X_UpdatC_StartYaddr %d\n", __func__,
			retVal);

	retVal = saf7741_WriteEasyP(I2CDRV_X_MEM, coef_num[band]);
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
	if (g_peq_gain[channel][band - 1] != 0) {
		DB2Val_7BandPeq(band, g_peq_fc[channel][band - 1], 0, g_peq_Q[channel][band - 1]);
			g_peq_gain[channel][band - 1] = 0;
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

static int delay_line(float ms)
{
        float delayf;
        int delay;

        if (ms <= 0.0f)
                ms = 0.0f;
        if (ms >= 20.0f)
                ms = 20.0f;

        delayf = roundf(FS * ms / 1000) + 1.0f;
        delay = (int)delayf;

        printf("0x%06x; /* %.1f ms */\n", delay, ms);

        return delay;
}

static int SAF7741_ConfigureSingleDelayLine(SAF7741_Channel_t channel, float dch)
{
	int retVal;
	unsigned int data;

	data = delay_line(dch);
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_Length1_In + channel, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_Length%d_In %d\n", __func__,
                                channel + 1, retVal);

	/* Activate delay line length settings */
        data = 0x7FFFFF;
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_InitFlag, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_InitFlag %d\n", __func__,
                                retVal);

	return retVal;
}

static int SAF7741_ConfigureDelayLines(float dfl, float dfr, float drl, float drr)
{
        int retVal;
        unsigned int data;

	/* Front Left Delay Time */
        data = delay_line(dfl);
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_Length1_In, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_Length1_In %d\n", __func__,
                                retVal);

	/* Front Right Delay Time */
        data = delay_line(dfr);
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_Length2_In, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_Length2_In %d\n", __func__,
                                retVal);

	/* Rear Left Delay Time */
        data = delay_line(drl);
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_Length3_In, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_Length3_In %d\n", __func__,
                                retVal);

	/* Rear Right Delay Time */
        data = delay_line(drr);
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_Length4_In, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_Length4_In %d\n", __func__,
                                retVal);

	/* Subwoofer Left Delay Time **/
        data = 0x000001;
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_Length5_In, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_Length5_In %d\n", __func__,
                                retVal);

	/* Subwoofer Right Delay Time **/
        data = 0x000001;
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_Length6_In, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_Length6_In %d\n", __func__,
                                retVal);

	/* Activate delay line length settings */
        data = 0x7FFFFF;
        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM, ADSP_X_Delay_InitFlag, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_Delay_InitFlag %d\n", __func__,
                                retVal);

        return retVal;

}

static float Max_Of_Array(float *array, int size)
{
	int i;
	float max = array[0];

	for (i = 1; i < size; i++)
		if (max < array[i])
			max = array[i];

	return max;
}

int SAF7741_ConfigureGeqBandGain(int band)
{
        unsigned int start_addr, coef_num;
        unsigned int *coef;
        int retVal, i;

        switch (band) {
        case 1:
                start_addr = ADSP_Y_GEq_b10L_REL;
                coef_num = 7;
                break;
        case 2:
                start_addr = ADSP_Y_GEq_b20L_REL;
                coef_num = 7;
                break;
        case 3:
                start_addr = ADSP_Y_GEq_b30L_REL;
                coef_num = 7;
		break;
        case 4:
                start_addr = ADSP_Y_GEq_b40_REL;
                coef_num = 4;
                break;
        case 5:
                start_addr = ADSP_Y_GEq_b50_REL;
                coef_num = 4;
                break;
        default:
                pr_err("%s: Unsupported band number %d", __func__, band);
                return -EINVAL;
        }

	coef = &g_geq_para[(band - 1) * 7];

	for (i = 0; i < coef_num; i++) {
                retVal = saf7741_WriteDSPMem(I2CDRV_Y_MEM, ADSP_Y_UpdatC_Coeff0 + i,
                                1, &(coef[i]));
                if (retVal < 0)
                        pr_err("%s: Failed to write ADSP_Y_UpdatC_Coeff%d %d\n", __func__,
                                        i, retVal);
        }

        retVal = saf7741_WriteDSPMem(I2CDRV_X_MEM,
				ADSP_X_SrcSw_OrDigSrcSelMask, 1, &start_addr);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_X_SrcSw_OrDigSrcSelMask %d\n", __func__,
                                retVal);

        retVal = saf7741_WriteEasyP(I2CDRV_X_MEM, coef_num);
        if (retVal < 0)
                pr_err("%s: Failed to EasyProgramming %d\n", __func__,
                                retVal);

        return retVal;
}

int SAF7741_ConfigureGeqDesScal(int scale)
{
        int retVal;
        unsigned int data;

        data = scale;
        retVal = saf7741_WriteDSPMem(I2CDRV_Y_MEM, ADSP_Y_Vol_DesScalGEq, 1,
                        &data);
        if (retVal < 0)
                pr_err("%s: Failed to write ADSP_Y_Vol_DesScalGEq %d\n", __func__,
                                retVal);

        return retVal;
}

static void DB2Val_DesScaleGeq(float gain)
{
	float valf;
	int des_scale;

	/*
	 * Y:Vol_DesScalGEQ
	 */
	valf = powf(10.0f, -gain / 20.0f);
	des_scale = YMEM2HEX(valf);

	printf("0x%03x, /* %.1f dB */\n", des_scale, gain);

	SAF7741_ConfigureGeqDesScal(des_scale);
}

static void DB2Val_5BandGeq(int band, int fc, float gain, float Q)
{
	float G, t, D;
	double a2d, a1d, b0d;
	float a2f, a1f, b0f;
	int gc, a2l, a2h, a1l, a1h, b0l, b0h;

	if (gain > GraphicEqualizerMaxdb || gain < -GraphicEqualizerMaxdb) {
		printf("%s: gain %d-%d is supported\n", __func__,
				-GraphicEqualizerMaxdb, GraphicEqualizerMaxdb);
		return;
	}

	if (Q < 0.1f || Q > 10.0f) {
		printf("%s: Qfactor 0.1-10.0 is supported\n", __func__);
		return;
	}

	G = (powf(10, gain / 20.0f) - 1) / 4;
	t = 2.0f * M_PI * fc / FS;
	if (gain >= 0)
		D = 1.0f;
	else
		D = 1 / (4 * G + 1);

	switch(band) {
	case 1:
	case 2:
	case 3:
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
	case 4:
	case 5:
		a2f = -0.5 * (1 - D * t / 2 / Q) / (1 + D * t / 2 / Q);
		a1f = (0.5 - a2f) * cos(t);
		b0f = (0.5 + a2f) / 2;

		gc = YMEM2HEX(G);
		b0h = 0;
		b0l = YMEM2HEX(b0f);
		a1h = 0;
		a1l = YMEM2HEX(a1f);
		a2h = 0;
		a2l = YMEM2HEX(a2f);

		break;
	default:
		break;
	}

	if (band <= 3) {
		printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x%03x, /* band %d(%d) %.1fdB %.2f*/\n", b0l, b0h,
				a2l, a2h, a1l, a1h, gc, band, fc, gain, Q);
		g_geq_para[(band - 1) * 7 + 0] = b0l;
		g_geq_para[(band - 1) * 7 + 1] = b0h;
		g_geq_para[(band - 1) * 7 + 2] = a2l;
		g_geq_para[(band - 1) * 7 + 3] = a2h;
		g_geq_para[(band - 1) * 7 + 4] = a1l;
		g_geq_para[(band - 1) * 7 + 5] = a1h;
		g_geq_para[(band - 1) * 7 + 6] = gc;
	} else {
		printf("0x%03x, 0x%03x, 0x%03x, 0x%03x, 0x000, 0x000, 0x000, /* band %d(%d) %.1fdB %.2f*/\n", b0l, a2l,
				a1l, gc, band, fc, gain, Q);
		g_geq_para[(band - 1) * 7 + 0] = b0l;
		g_geq_para[(band - 1) * 7 + 1] = a2l;
		g_geq_para[(band - 1) * 7 + 2] = a1l;
		g_geq_para[(band - 1) * 7 + 3] = gc;
	}

	SAF7741_ConfigureGeqBandGain(band);
	DB2Val_DesScaleGeq(Max_Of_Array(g_geq_gain, 7));
}

static void restore_geq2flat(int band)
{
	if (g_geq_gain[band - 1] != 0) {
		DB2Val_5BandGeq(band, g_geq_fc[band - 1], 0, g_geq_Q[band - 1]);
			g_geq_gain[band - 1] = 0;
	}

	return;
}

/*
 * Preset gains for speed compensation volume
 */
static unsigned short SAF7741_ChannelGain[ChannelGainMaxLevel + 1] = {
	0x102,  /* -12.00 dB */
	0x121,  /* -11.00 dB */
	0x145,  /* -10.00 dB */
	0x16C,  /*  -9.00 dB */
	0x199,  /*  -8.00 dB */
	0x1CA,  /*  -7.00 dB */
	0x202,  /*  -6.00 dB */
	0x241,  /*  -5.00 dB */
	0x288,  /*  -4.00 dB */
	0x2D7,  /*  -3.00 dB */
	0x32F,  /*  -2.00 dB */
	0x393,  /*  -1.00 dB */
	0x402,  /*   0.00 dB */
	0x480,  /*   1.00 dB */
	0x50C,  /*   2.00 dB */
	0x5AA,  /*   3.00 dB */
	0x65B,  /*   4.00 dB */
	0x721,  /*   5.00 dB */
	0x7FF,  /*   6.00 dB */
};

static int SAf7741_ConfigureChannelGain(SAF7741_Channel_t channel, int gain)
{
	int retVal;
	unsigned int reg, data;
	int presetdb;

	switch(channel) {
	case FrontLeft:
		reg = ADSP_Y_Vol_ChanGainPFL;
		presetdb = ChannelGainPFLPresetdb;
		break;
	case FrontRight:
		reg = ADSP_Y_Vol_ChanGainPFR;
		presetdb = ChannelGainPFRPresetdb;
		break;
	case RearLeft:
		reg = ADSP_Y_Vol_ChanGainPRL;
		presetdb = ChannelGainPRLPresetdb;
		break;
	case RearRight:
		reg = ADSP_Y_Vol_ChanGainPRR;
		presetdb = ChannelGainPRRPresetdb;
		break;
	default:
		pr_err("%s: Unsupported SAF7741_Channel_t %d\n", __func__,
				channel);

		return -EINVAL;
	}

	if ((presetdb + gain) > ChannelGainMaxdb) {
		pr_err("%s: Maximum Primary Channel Gain is %d dB(value : %d)\n", __func__,
				ChannelGainMaxdb, presetdb + gain);
		return -EINVAL;
	}

	data = (unsigned int)SAF7741_ChannelGain[dB2Index(presetdb + gain, ChannelGain0dbLevel)];

	retVal = saf7741_WriteDSPMem(I2CDRV_Y_MEM, reg, 1, &data);
	if (retVal < 0)
		pr_err("%s: Failed to write ADSP_Y_Vol_SrcScalP %d\n", __func__,
				retVal);

	return retVal;
}

void opcode_help(void)
{
	printf("\n/**************************** Test for PEQ and DelayLines *******************************/\n");
	printf("Exit:			0x1000\n");
	printf("Set Deubg Mask:		0x1111 x[0/1: on/off]\n");
	printf("/************** PEQ *************/\n");
	printf("Get Current Channel:	0x1001\n");
	printf("Select Channel:		0x1002 xxx[0--3]\n");
	printf("Print PEQ parameters:	0x1003\n");
	printf("Set PEQ:		0x1004 band fc gain Q xxx xxx ... xxx(10 parameters)\n");
	printf("Calc and set PEQ:	0x1005 band fc gain Q\n");
	printf("Restore PEQ bandX:	0x1006 band\n");
	printf("Restore all band on current channel:	0x1007\n");
	printf("Restore PEQ:		0x1008\n");
	printf("/************** Delay Lines *************/\n");
	printf("Set single DelayLine:	0x1100 ch delay\n");
	printf("Set DelayLines:		0x1101 DFL DFR DRL DRR\n");
	printf("Get DelayLines:		0x1102\n");
	printf("/************** GEQ *************/\n");
	printf("Print GEQ parameters:	0x1200\n");
	printf("Calc and set GEQ:	0x1201 band fc gain Q\n");
	printf("Restore GEQ bandX:	0x1202 band\n");
	printf("Restore GEQ:		0x1203\n");
	printf("/************** Channel Gain *************/\n");
	printf("Get Channel Gain:	0x1300\n");
	printf("Set Channel Gain:	0x1301 FL FR RL RR\n");
	printf("/*******************************************************************/\n");
}

int main(int argc, char *argv[])
{
	int opcode;
	int i, j;
	char str[512];
	char buf[64];
	int band, channel;

	while (1) {
		fgets(str, sizeof(str), stdin);
		_ptr_cmd = str;
		opcode = GET_OPCODE(0x999);
		_ptr_cmd = NULL;

		switch(opcode) {
		case 0x1000:
			exit(0);
			break;
		case 0x1111:
			g_debug_mask = GET_PARAM_D(0);
			if (g_debug_mask)
				printf("Debug Mask is on\n");
			else
				printf("Debug Mask is off\n");
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
							band + 1, g_peq_fc[i][band], g_peq_gain[i][band], g_peq_Q[i][band]);
						printf("%s\n", buf);
					}
				}
			}
			break;
		case 0x1004:
			band = GET_PARAM_D(0);
			g_peq_fc[g_channel][band - 1] = GET_PARAM_D(0);
			g_peq_gain[g_channel][band - 1] = GET_PARAM_F(0);
			g_peq_Q[g_channel][band - 1] = GET_PARAM_F(0);

			printf("PEQ settings for channel: %s\n", channel_strops(g_channel));
			printf("band = %d, g_peq_fc = %d, g_peq_gain = %3.1f, g_peq_Q = %4.2f\n",
				band, g_peq_fc[g_channel][band - 1], g_peq_gain[g_channel][band - 1], g_peq_Q[g_channel][band - 1]);

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
			g_peq_fc[g_channel][band - 1] = GET_PARAM_D(0);
			g_peq_gain[g_channel][band - 1] = GET_PARAM_F(0);
			g_peq_Q[g_channel][band - 1] = GET_PARAM_F(0);

			printf("Calculate and set PEQ parameters for channel: %s\n", channel_strops(g_channel));
			printf("band = %d, g_peq_fc = %d, g_peq_gain = %3.1f, g_peq_Q = %4.2f\n",
				band, g_peq_fc[g_channel][band - 1], g_peq_gain[g_channel][band - 1], g_peq_Q[g_channel][band - 1]);

			DB2Val_7BandPeq(band, g_peq_fc[g_channel][band - 1], g_peq_gain[g_channel][band - 1], g_peq_Q[g_channel][band - 1]);
			break;
		case 0x1006:
			band = GET_PARAM_D(0);
			printf("Restore band %d to Flat on channel %s\n", band, channel_strops(g_channel));

			restore_peq2flat(g_channel, band);
			break;
		case 0x1007:
			printf("Restore all band of PEQ to Flat on channel %s\n", channel_strops(g_channel));

			for (band = 1; band <= 7; band++)
				restore_peq2flat(g_channel, band);
			break;
		case 0x1008:
			printf("Restore PEQ to Flat on All channel\n");

			for (i = 0; i < 4; i++)
				for (band = 1; band <= 7; band++)
					restore_peq2flat(i, band);
			break;
		case 0x1100:
			channel = GET_PARAM_D(FrontLeft);
			g_delay[channel] = GET_PARAM_F(0);

			if ((g_delay[FrontLeft] + g_delay[FrontRight] +
				g_delay[RearLeft] + g_delay[RearRight]) > MAX_DELAY_LINES_MS) {
				printf("Max Delay Lines can't exceed 20ms !!!\n");
				break;
			}
			printf("Set %s delay %.1f ms\n", channel_strops(channel), g_delay[channel]);

			SAF7741_ConfigureSingleDelayLine(channel, g_delay[channel]);
			break;
		case 0x1101:
			g_delay[FrontLeft] = GET_PARAM_F(0);
			g_delay[FrontRight] = GET_PARAM_F(0);
			g_delay[RearLeft] = GET_PARAM_F(0);
			g_delay[RearRight] = GET_PARAM_F(0);

			if ((g_delay[FrontLeft] + g_delay[FrontRight] +
				g_delay[RearLeft] + g_delay[RearRight]) > MAX_DELAY_LINES_MS) {
				printf("Max Delay Lines can't exceed 20ms !!!\n");
				break;
			}
			printf("DFL: %.1fms, DFR: %.1fms, DRL: %.1fms, DRR: %.1fms\n",
				g_delay[FrontLeft], g_delay[FrontRight], g_delay[RearLeft], g_delay[RearRight]);

			SAF7741_ConfigureDelayLines(g_delay[FrontLeft],
				g_delay[FrontRight], g_delay[RearLeft], g_delay[RearRight]);
			break;
		case 0x1102:
			printf("DFL: %.1fms, DFR: %.1fms, DRL: %.1fms, DRR: %.1fms\n",
				g_delay[FrontLeft], g_delay[FrontRight], g_delay[RearLeft], g_delay[RearRight]);
			break;
		case 0x1200:
			DB2Val_DesScaleGeq(Max_Of_Array(g_geq_gain, 7));
			for (i = 1; i <= 35; i++) {
				printf("0x%03X, ", g_geq_para[i - 1]);
				if (i % 7 == 0) {
					band = i / 7 - 1;
					memset(buf, 0x00, sizeof(buf));
					sprintf(buf, "/* band %d(%d) %3.1fdB %4.2f */",
						band + 1, g_geq_fc[band], g_geq_gain[band], g_geq_Q[band]);
					printf("%s\n", buf);
				}
			}
			break;
		case 0x1201:
			band = GET_PARAM_D(0);
			g_geq_fc[band - 1] = GET_PARAM_D(0);
			g_geq_gain[band - 1] = GET_PARAM_F(0);
			g_geq_Q[band - 1] = GET_PARAM_F(0);

			printf("Calculate and set GEQ parameters\n");
			printf("band = %d, g_geq_fc = %d, g_geq_gain = %3.1f, g_geq_Q = %4.2f\n",
				band, g_geq_fc[band - 1], g_geq_gain[band - 1], g_geq_Q[band - 1]);

			DB2Val_5BandGeq(band, g_geq_fc[band - 1], g_geq_gain[band - 1], g_geq_Q[band - 1]);
			break;
		case 0x1202:
			band = GET_PARAM_D(0);
			printf("Restore GEQ band %d to Flat\n", band);

			restore_geq2flat(band);
			break;
		case 0x1203:
			printf("Restore all band of GEQ to Flat\n");

			for (band = 1; band <= GEQ_BAND_MAX; band++)
				restore_geq2flat(band);
			break;
		case 0x1300:
			printf("Relative Channel Gain: FL: %d, FR: %d, RL: %d, RR: %d\n",
				g_channel_gain[FrontLeft], g_channel_gain[FrontRight],
				g_channel_gain[RearLeft], g_channel_gain[RearRight]);
			break;
		case 0x1301:
			g_channel_gain[FrontLeft] = GET_PARAM_D(0);
			g_channel_gain[FrontRight] = GET_PARAM_D(0);
			g_channel_gain[RearLeft] = GET_PARAM_D(0);
			g_channel_gain[RearRight] = GET_PARAM_D(0);

			printf("Relative Channel Gain: FL: %d, FR: %d, RL: %d, RR: %d\n",
				g_channel_gain[FrontLeft], g_channel_gain[FrontRight],
				g_channel_gain[RearLeft], g_channel_gain[RearRight]);
			
			for (channel = FrontLeft; channel < ChannelMax; channel++)
				SAf7741_ConfigureChannelGain(channel, g_channel_gain[channel]);
			break;
		default:
			printf("unknow opcode\n");
			opcode_help();
			break;
		}
	}
	
	
	return 0;
}
