#include <stdio.h>
#include <math.h>

#include "saf7741.h"

/*
 * Translate the dB volume to Y:Vol_Main1P and Y:Vol_Main2P
 */
void DB2Val_PrimaryVolume(float min, float step, float max)
{
	int Main1P, Main2P;
	float volume;

	printf("PrimaryChannelMainVolume:\n");
	volume = min;
	while (volume <= max) {
		if (volume >= 0) {
			Main1P = YMEM2Hex(-1);
			Main2P = YMEM2Hex(powf(10, (volume - PrimaryVolMaxdb) / 20));
		} else if (volume < 0 && volume >= -LoudnessMaxdb) {
			Main1P = YMEM2Hex(-powf(10, volume / 20));
			Main2P = YMEM2Hex(powf(10, -PrimaryVolMaxdb / 20));
		} else if (volume < -LoudnessMaxdb && volume >= -48.16 + PrimaryVolMaxdb - LoudnessMaxdb) {
			Main1P = YMEM2Hex(-powf(10, -LoudnessMaxdb / 20));
			Main2P = YMEM2Hex(powf(10, (volume + LoudnessMaxdb - PrimaryVolMaxdb) /
						20));
		} else {
			Main1P = YMEM2Hex(-256 * powf(10, (volume - PrimaryVolMaxdb) / 20));
			Main2P = YMEM2Hex(1.0f / 256);
		}

		printf("0x%03x, 0x%03x, /* %6.2f dB */\n", Main1P, Main2P, volume);
		volume += step;
	}
}

/*
 * Translate the dB volume to Y:Vol_Nav
 */
void DB2Val_NaviVolume(float min, float step, float max)
{
	int Nav;
	float volume;

	printf("NaviChannelVolume:\n");

	volume = min;
	while (volume <= max) {
		Nav = YMEM2Hex(-0.25f * powf(10, volume / 20));

		printf("0x%03x, /* %6.2f dB */\n", Nav, volume);
		volume += step;
	}
}

/*
 * Translate the dB volume to Y:Vol_Phone
 */
void DB2Val_PhoneVolume(float min, float step, float max)
{
	int Phone;
	float volume;

	printf("PhoneChannelVolume:\n");

	volume = min;
	while (volume <= max) {
		Phone = YMEM2Hex(-0.25f * powf(10, volume / 20));

		printf("0x%03x, /* %6.2f dB */\n", Phone, volume);
		volume += step;
	}
}

/*
 * Translate the dB volume to Y:Vol_RSA<N> (N=1,2)
 */
void DB2Val_RSANVolume(float min, float step, float max)
{
	int RSA;
	float volume;

	printf("RSANChannelVolume:\n");

	volume = min;
	while (volume <= max) {
		RSA = YMEM2Hex(-0.25f * powf(10, volume / 20));

		printf("0x%03x, /* %6.2f dB */\n", RSA, volume);
		volume += step;
	}
}

/*
 * Translate the dB volume to Y:Sup_<X>on<Y>
 *	X = { N, T, Chi, Ext}
 *	Y = { FL, FR, RL, RR, SwL, SwR, ADL, ADR }
 */
void DB2Val_SupVolume(float min, float step, float max)
{
	int Sup;
	float volume;

	printf("SuperpositionVolume:\n");

	volume = min;
	while (volume <= max) {
		Sup = YMEM2Hex(-powf(10, volume / 20));

		printf("0x%03x, /* %6.2f dB */\n", Sup, volume);
		volume += step;
	}
}

int main(int argc, char *argv[])
{
	DB2Val_PrimaryVolume(-PrimaryVol0dbLevel, 0.25f, 12.0f);

	DB2Val_NaviVolume(-NavVol0dbLevel, 1.0f, 12.0f);

	DB2Val_PhoneVolume(-PhoneVol0dbLevel, 1.0f, 12.0f);

	DB2Val_RSANVolume(-RSA1Vol0dbLevel, 1.0f, 0.0f);

	DB2Val_SupVolume(-ChimeVol0dbLevel, 1.0f, 0.0f);

	return 0;
}
