#include <stdio.h>
#include <math.h>

#define FS	44100

int delay_line(float ms)
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

#if 0
int main(int argc, char *argv[])
{
        printf("FL Delayline:\n");
        delay_line(1.0f);
        printf("FR Delayline:\n");
        delay_line(1.0f);
        printf("RL Delayline:\n");
        delay_line(0.0f);
        printf("RR Delayline:\n");
        delay_line(0.0f);


        return 0;
}
#endif

