#include <stdio.h>
#include <math.h>

#include "saf7741.h"

static void delay_line(float ms)
{
        float delayf;
        int delay;

        if (ms <= 0.0f)
                ms = 0.0f;
        if (ms >= 26.0f)
                ms = 26.0f; /* 1280 / 48 = 26.666 */

        delayf = roundf(FS * ms / 1000) + 1.0f;
        delay = (int)delayf;

        printf("0x%06x; /* %.1f ms */\n", delay, ms);
}

int main(int argc, char *argv[])
{
        printf("FL Delayline:\n");
        delay_line(0.0f);
        printf("FR Delayline:\n");
        delay_line(0.8f);
        printf("RL Delayline:\n");
        delay_line(0.0f);
        printf("RR Delayline:\n");
        delay_line(0.0f);


        return 0;
}
