#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "core.c"
#include "Audio_7741.h"

char *_ptr_cmd;
char *_ptr_param;
#define TO_SEP_CMD					(_ptr_param = strsep(&_ptr_cmd, " "))
#define GET_PARAM_H(defparam)		(TO_SEP_CMD ? strtoll(_ptr_param, NULL, 16) : defparam)
#define GET_OPCODE(defparam)		GET_PARAM_H(defparam)
#define GET_PARAM(defparam)		GET_PARAM_H(defparam)

typedef enum SAF7741_Channel {
	FrontLeft = 0,
	FrontRight,
	RearLeft,
	RearRight,
} SAF7741_Channel_t;

static unsigned short g_peq_para[4][70] = {0};
static SAF7741_Channel_t g_channel = FrontLeft;

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
	printf("/**************************** CMD Comment *******************************/\n");
	printf("Exit:			0x1000\n");
	printf("Get Current Channel:	0x1001\n");
	printf("Select Channel:		0x1002 xxx\n");
	printf("Print PEQ parameters:	0x1003\n");
	printf("PEQ settings:		0x1004 band xxx xxx ... xxx(10 parameters)\n");
	printf("/*******************************************************************/\n");
}

int main(int argc, char *argv[])
{
	int i, j;
	char str[512];
	int band;

	while (1) {
		fgets(str, sizeof(str), stdin);
		_ptr_cmd = str;

		switch(GET_OPCODE(0x999)) {
		case 0x1000:
			exit(0);
			break;
		case 0x1001:
			printf("Current Chanel: %s\n", channel_strops(g_channel));
			break;
		case 0x1002:
			g_channel = GET_PARAM_H(0);
			printf("Select Chanel: %s\n", channel_strops(g_channel));
			break;
		case 0x1003:
			for (i = 0; i < 4; i++) {
				printf("/********** Channel: %s **********/\n", channel_strops(i));
				for (j = 1; j <= 70; j++)
					printf("0x%03X, %s", g_peq_para[i][j-1], (j % 10 == 0) ? "\n" : "");
			}
			break;
		case 0x1004:
			printf("PEQ settings for channel: %s\n", channel_strops(g_channel));
			band = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 0] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 1] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 2] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 3] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 4] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 5] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 6] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 7] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 8] = GET_PARAM_H(0);
			g_peq_para[g_channel][(band-1)*10 + 9] = GET_PARAM_H(0);
			SAF7741_ConfigurePeqPreset(g_channel, band-1);
			break;
		default:
			printf("unknow opcode\n");
			opcode_help();
			break;
		}
	}
	
	
	return 0;
}
