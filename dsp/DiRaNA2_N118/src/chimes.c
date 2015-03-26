#include <stdio.h>
#include <math.h>

#include "saf7741.h"

struct chime_struct {
        int fs;
        int freq;
        int Tsq_Tnb1Htone_Y;
        int Tsq_Tnb1Ltone_Y;

        int Attenuation;
        int Tsq_TnVtone_Y;

        int master_volume;
        int Tsq_ChimeVol_Y;

        int ta,tr;
        int Tsq_ArLinStpA_X;
        int Tsq_ArRelCoef_X;
        int Tsq_ArType_X;

        int TA, TB, Tn;
        int Tsq_TA_X;
        int Tsq_TB_X;
        int Tsq_Tn_X;

        int r;
        int Tsq_Repeate_X;


} chime_example = {	44100, 2000, 0, 0,
        -12, 0, 
        -12, 0,
        1, 1, 0, 0, 0,
        99, 101, 0, 0, 0, 0,
        1, 0};

/*
 * Sine Wave Generator calculation function
 */
static void sine_generator(double f)
{
        double rad;
        double b1d;
        int b1h, b1l;

        rad = 2.0 * M_PI * f / (FS / 2);
        b1d = cos(rad);
        b1h = DYMEM2Hex(b1d) >> 12;
        b1l = DYMEM2Hex(b1d) & 0xFFF;

        printf("0x%03x, 0x%03x, /* frequency %.1fHz */\n", b1h, b1l, f);
}

/*
 * Output amplitude calculation function
 */
static void amplitude(float vol)
{
        float attenu;
        int Vtone;

        attenu = -powf(10.0f, vol / 20.0);
        Vtone = YMEM2Hex(attenu);
        printf("0x%03x, /* amplitude %.1fdB */\n", Vtone, vol);
}

/*
 * Master Volume control all FL,FR,RL,RR attenuation
 */
static void master_volume(float vol)
{
        float master_vol;
        int ChimeVol;

        master_vol = powf(10.0f, vol / 20.0);
        ChimeVol = YMEM2Hex(master_vol);
        printf("0x%03x, /* master volume %.1fdB */\n", ChimeVol, vol);
}

/*
 * Cross Fader Timing calculation
 */
static void cross_fader_timing(int tcf)
{
        float val;
        int CrossFStp;

        val = 1.0f / (tcf / 1000.0f * FS / 4);
        CrossFStp = XMEM2Hex(val);
        printf("0x%06x, /* CrossFStp %dms */\n", CrossFStp, tcf);
}

/*
 * Linear attach/release timing calculation
 */
static void linear_ar(int ta, int tr)
{
        float ta_val, tr_val;
        int ArLinStpA, ArRelCoef, ArType;

        ta_val = 4.0f / (ta / 1000.0f * FS);
        ArLinStpA = XMEM2Hex(ta_val);
        tr_val = 4.0f / (tr / 1000.0f * FS);
        ArRelCoef = XMEM2Hex(tr_val);
        ArType = YMEM2Hex(0.0f);

        printf("0x%06x, 0x%06x, 0x%03x, /* Linear attack: %dms, release: %dms */\n",
                        ArLinStpA, ArRelCoef, ArType, ta, tr);
}

/*
 * Exponential attach/release timing calculation
 */
static void expon_ar(int ta, int tr)
{
        float ta_val, tr_val;
        int ArLinStpA, ArRelCoef, ArType;

        ta_val = 4.0f / (ta / 1000.0f * FS);
        ArLinStpA = XMEM2Hex(ta_val);
        tr_val = expf(-1.0f / (tr / 1000.0f * FS / 4));
        ArRelCoef = XMEM2Hex(tr_val);
        ArType = YMEM2Hex(1.0f);

        printf("0x%06x, 0x%06x, 0x%03x /* Exponential attack: %dms, release: %dms */\n",
                        ArLinStpA, ArRelCoef, ArType, ta, tr);
}

/*
 * TA, TB, Tn Sequencer timing
 */
static void sequencer(int ta, int tb, int t1, int t2, int t3, int t4)
{
        float ta_val, tb_val;
        float t1_val, t2_val, t3_val, t4_val;
        int TA, TB, T1, T2, T3, T4;

        ta_val = FS * ta / 1000.0f / powf(2.0f, 27);
        TA = XMEM2Hex(ta_val);
        tb_val = FS * tb / 1000.0f / powf(2.0f, 27);
        TB = XMEM2Hex(tb_val);
        t1_val = FS * t1 / 1000.0f / powf(2.0f, 27);
        T1 = XMEM2Hex(t1_val);
        t2_val = FS * t2 / 1000.0f / powf(2.0f, 27);
        T2 = XMEM2Hex(t2_val);
        t3_val = FS * t3 / 1000.0f / powf(2.0f, 27);
        T3 = XMEM2Hex(t3_val);
        t4_val = FS * t4 / 1000.0f / powf(2.0f, 27);
        T4 = XMEM2Hex(t4_val);

        printf("0x%06x, 0x%06x, 0x%06x, 0x%06x, 0x%06x, 0x%06x, /* TA %dms, TB %dms, T1 %dms, T2 %dms, T3 %dms, T4 %dms */\n",
                        TA, TB, T1, T2, T3, T4, ta, tb, t1, t2, t3, t4);
}

/*
 * Number of signals calculation
 */
static void duration(int r)
{
        float r_val;
        int Repeat;

        if (r == -1)
                r_val = -1.0f;
        else
                r_val = (r + 1) / powf(2.0f, 23);
        Repeat = XMEM2Hex(r_val);
        printf("0x%06x, /* duration %dms */\n", Repeat, r);
}

/*
 * Change Threshold
 */
static void change_threshold(int tx, int t)
{
        int ChangeThreshold;
        float val;

        val = expf(-(float)tx / t);
        ChangeThreshold = XMEM2Hex(val);

        printf("0x%06x, /* change threshold %dms,%dms */\n",
                        ChangeThreshold, tx, t);
}

/*
 * Decay Threshold
 */
static void decay_threshold(int td, int t)
{
        int ChangeThreshold;
        float val;

        val = expf(-(float)td / t);
        ChangeThreshold = XMEM2Hex(val);

        printf("0x%06x, /* decay threshold %dms,%dms */\n",
                        ChangeThreshold, td, t);
}

int main()
{
        printf("/* GONG 750Hz */\n");
        sine_generator(750.0);
        sine_generator(750.0);
        sine_generator(750.0);
        sine_generator(750.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        master_volume(-12.0);
        cross_fader_timing(10);
        linear_ar(2, 1198);
        sequencer(2, 1198, 0, 0, 0, 0);
        duration(-1);
        change_threshold(400, 750);
        decay_threshold(3000, 750);

        printf("/* GONG 2000Hz */\n");
        sine_generator(2000.0);
        sine_generator(2000.0);
        sine_generator(2000.0);
        sine_generator(2000.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        master_volume(-12.0);
        cross_fader_timing(10);
        linear_ar(2, 1198);
        sequencer(2, 1198, 0, 0, 0, 0);
        duration(-1);
        change_threshold(400, 750);
        decay_threshold(3000, 750);

        printf("/* BEEP 750Hz */\n");
        sine_generator(750.0);
        sine_generator(750.0);
        sine_generator(750.0);
        sine_generator(750.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        master_volume(-12.0);
        cross_fader_timing(10);
        linear_ar(1, 1);
        sequencer(99, 101, 0, 0, 0, 0);
        duration(-1);
        change_threshold(400, 750);
        decay_threshold(3000, 750);

        printf("/* BEEP 2000Hz */\n");
        sine_generator(2000.0);
        sine_generator(2000.0);
        sine_generator(2000.0);
        sine_generator(2000.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        amplitude(0.0);
        master_volume(-12.0);
        cross_fader_timing(10);
        linear_ar(1, 1);
        sequencer(99, 101, 0, 0, 0, 0);
        duration(-1);
        change_threshold(400, 750);
        decay_threshold(3000, 750);

        return 0;
}
