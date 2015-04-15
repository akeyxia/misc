#ifndef _FM_H
#define _FM_H

#include <linux/videodev2.h>

int radio_startup(void);
int radio_set_frequency(int freq);
int radio_get_frequency(int *freq, int *rssi, int *snr, int *stereo);
int radio_seek_up(void);
int radio_close_radio(void);
int radio_muteon(void);
int radio_muteoff(void);

#ifndef VIDIOC_S_HW_FREQ_SEEK
#define VIDIOC_S_HW_FREQ_SEEK	 _IOW('V', 82, struct v4l2_hw_freq_seek)
#endif

typedef struct RadioDescriptor
{
	/* V4l2 structures */
	struct v4l2_capability vc;
	struct v4l2_tuner vt;
	struct v4l2_control vctrl;
	struct v4l2_frequency vf;
	struct v4l2_hw_freq_seek vhfs;
	int radio_fd;
}RadioDescriptor;


#endif

