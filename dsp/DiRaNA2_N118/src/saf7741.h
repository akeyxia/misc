#ifndef _SAF7741_H_
#define _SAF7741_H_

/* Sample frequency */
#define FS				(48000)

/* Primary Channel */
#define PrimaryVolMaxdb		(12.0f)
#define PrimaryVolMaxLevel	(84.0f)
#define PrimaryVol0dbLevel	(72.0f)

/* Navigation Channel Volume Settings in 1dB unit */
#define NavVolMaxLevel		(66.0f)
#define NavVolMaxdb			(12.0f)
#define NavVol0dbLevel		(54.0f)

/* Phone Channel Volume Settings in 1dB unit */
#define PhoneVolMaxLevel	(66.0f)
#define PhoneVolMaxdb		(12.0f)
#define PhoneVol0dbLevel	(54.0f)

/* RSA1 Channel Volume Settings in 1dB unit */
#define RSA1VolMaxLevel		(66.0f)
#define RSA1VolMaxdb		(0.0f)
#define RSA1Vol0dbLevel		(66.0f)

/* RSA2 Channel Volume Settings in 1dB unit */
#define RSA2VolMaxLevel		(66.0f)
#define RSA2VolMaxdb		(0.0f)
#define RSA2Vol0dbLevel		(66.0f)

/* Chime Channel Volume Settings in 1dB unit */
#define ChimeVolMaxLevel	(66.0f)
#define ChimeVolMaxdb		(66.0f)
#define ChimeVol0dbLevel	(66.0f)

/* ClickClack Channel Volume Settings in 1dB unit */
#define ClickClackVolMaxLevel	(66.0f)
#define ClickClackVolMaxdb		(66.0f)
#define ClickClackVol0dbLevel	(66.0f)

/* Graphic equalizer */
#define GraphicEqualizerMaxLevel	(24)
#define GraphicEqualizerMaxdb		(12)
#define GraphicEqualizer0dbLevel	(12)
#define ToneControlMaxdb			(0)
#define GeqBand1Fc		(80.0f)
#define GeqBand2Fc		(500.0f)
#define GeqBand3Fc		(1000.0f)
#define GeqBand4Fc		(5000.0f)
#define GeqBand5Fc		(16000.0f)

#define GeqPopBand1Fc	(200.0f)
#define GeqPopBand2Fc	(500.0f)
#define GeqPopBand3Fc	(1000.0f)
#define GeqPopBand4Fc	(4000.0f)
#define GeqPopBand5Fc	(15000.0f)
#define GeqJazzBand1Fc	(200.0f)
#define GeqJazzBand2Fc	(500.0f)
#define GeqJazzBand3Fc	(1000.0f)
#define GeqJazzBand4Fc	(4000.0f)
#define GeqJazzBand5Fc	(15000.0f)
#define GeqClassicBand1Fc	(200.0f)
#define GeqClassicBand2Fc	(500.0f)
#define GeqClassicBand3Fc	(1000.0f)
#define GeqClassicBand4Fc	(4000.0f)
#define GeqClassicBand5Fc	(15000.0f)
#define GeqVocalBand1Fc	(200.0f)
#define GeqVocalBand2Fc	(500.0f)
#define GeqVocalBand3Fc	(1000.0f)
#define GeqVocalBand4Fc	(4000.0f)
#define GeqVocalBand5Fc	(15000.0f)
#define GeqRockBand1Fc	(200.0f)
#define GeqRockBand2Fc	(500.0f)
#define GeqRockBand3Fc	(1000.0f)
#define GeqRockBand4Fc	(4000.0f)
#define GeqRockBand5Fc	(15000.0f)
#define GeqFlatBand1Fc	(80.0f)
#define GeqFlatBand2Fc	(500.0f)
#define GeqFlatBand3Fc	(1000.0f)
#define GeqFlatBand4Fc	(5000.0f)
#define GeqFlatBand5Fc	(16000.0f)

/* Parametric Equalizer */
#define ParametricEqualizerMaxdb		(12)

/* Static Loudness */
#define LoudnessMaxLevel		(10.0f)
#define LoudnessMaxdb			(10.0f) /* LoudnessMax equals the max value of BassMax
										   and TrebleMax */
#define LoudnessBassMaxLevel	(10.0f)
#define LoudnessBassMaxdb		(10.0f)
#define LoudnessTrebleMaxLevel	(4.0f)
#define LoudnessTrebleMaxdb		(4.0f)
#define LoudnessBassFc			(100.0f)
#define LoudnessTrebleFc		(10000.0f)

/* Fader & Balance */
#define FaderMaxLevel		(66.0f)
#define FaderMaxdb			(0.0f)
#define Fader0dbLevel		(66.0f)
#define BalanceMaxLevel		(66.0f)
#define BalanceMaxdb		(0.0f)
#define Balance0dbLevel		(66.0f)

/* Source Scale */
#define SrcScalPMaxLevel	(12.0f)
#define SrcScalPMaxdb		(12.0f)
#define SrcScalP0dbLevel	(0.0f)
#define SrcScalRSAMaxdb		(0.0f)

/* Channel Scale */
#define ChannelScalMaxLevel		(2.0f)
#define ChannelScalMaxdb		(2.0f)
#define ChannelScal0dbLevel		(0.0f)

/* Speed Compensation Volume */
#define SCVMaxLevel			(11.0f)
#define SCVMaxdb			(0.0f)
#define SCV0dbLevel			(11.0f)

/* Function declarations */
int YMEM2Hex(float frac);
int DYMEM2Hex(double frac);
int XMEM2Hex(float frac);

#endif
