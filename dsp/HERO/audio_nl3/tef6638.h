#ifndef _TEF6638_H_
#define _TEF6638_H_

#define FS	44100

#define LoudnessBassFc	100
#define LoudnessTrebleFc	10000
#define LoudnessBassMaxdb	10
#define LoudnessTrebleMaxdb	4

int YMEM2HEX(float frac);
int XMEM2HEX(double frac);
int DYMEM2HEX(double frac);

#endif

