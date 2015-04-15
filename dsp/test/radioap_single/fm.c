#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/videodev.h>
#include "fm.h"
#define DEFAULT_DEVICE "/dev/radio0"

#define DEBUG

#ifdef  DEBUG
#define LOGE(fmt,args...)     printf (fmt ,##args)
#define LOGD(fmt,args...)     printf (fmt ,##args)
#else
#define LOGE(fmt,args...)
#endif

struct v4l2_capability vc;
struct v4l2_tuner vt;
struct v4l2_control vctrl;
struct v4l2_frequency vf;
struct v4l2_hw_freq_seek vhfs;
static int radiofd = 0;
static char cmd[50][20];
static int count = 0;

int radio_open(void)
{
	if (radiofd > 0)
		printf("radio has been opened, current radiofd = %d\n", radiofd);

	if ((radiofd = open(DEFAULT_DEVICE, O_RDONLY))< 0){
		printf("open " DEFAULT_DEVICE " error\n");
		return -1;
	}
	printf("radiofd = %d\n", radiofd);

	// Query Radio device capabilities.
	if (ioctl(radiofd, VIDIOC_QUERYCAP, &vc)<0){
		printf("ioctl VIDIOC_QUERYCAP error\n");
		return -1;
	}

	// Set tuner index. Must be 0.
	vt.index = 0;
	ioctl(radiofd, VIDIOC_S_TUNER, &vt);
	if (!(vc.capabilities & V4L2_CAP_TUNER)){
		printf("capabilities can't match V4L2_CAP_TUNER\n");
		return -1;
	}

	printf("radio open ok!!\n");

	return 0;
}

static int radio_mute(void)
{
	int ret;
	if (radiofd < 0)
                return -1;

	vctrl.id = V4L2_CID_AUDIO_MUTE;
	vctrl.value = 1;
	
	ret = ioctl(radiofd, VIDIOC_S_CTRL, &vctrl);
	if(ret < 0){
		LOGE("ioctl VIDIOC_S_CTRL V4L2_CID_AUDIO_MUTE fail\n");
		return -1;
	}

	return 0;
}

static int radio_unmute(void)
{
	int ret;
	if (radiofd < 0)
                return -1;

	vctrl.id = V4L2_CID_AUDIO_MUTE;
	vctrl.value = 0;
	ret = ioctl(radiofd, VIDIOC_S_CTRL, &vctrl);
	if(ret < 0){
		LOGE("ioctl VIDIOC_S_CTRL V4L2_CID_AUDIO_MUTE fail\n");
		return -1;
	}
	return 0;
}


static int radio_seek(void)
{
	int ret;
	if (radiofd < 0)
		return -1;

	vhfs.tuner = 0;
	vhfs.seek_upward = 0;
	vhfs.wrap_around = 1;
	vhfs.type = V4L2_TUNER_RADIO;

	ret = ioctl(radiofd, VIDIOC_S_HW_FREQ_SEEK, &vhfs);
	if(ret < 0){
		LOGE("ioctl VIDIOC_S_HW_FREQ_SEEK fail, %s\n", strerror(errno));
		return -1;
	}
	return 0;
}


static int radio_getfreq()
{
	int ret = 0;

	if (radiofd < 0)
		return -1;

	vf.tuner = 0;
	ret = ioctl(radiofd, VIDIOC_G_FREQUENCY, &vf);
	if(ret < 0){
		printf("ioctl VIDIOC_G_FREQUENCY fail\n");
		return -1;
	}

	printf("freq is %d, rssi is %d, snr is %d, stereo is %d\n", vf.frequency,
		vf.reserved[0], vf.reserved[1], vf.reserved[2]);

	return 0;
}

static int radio_setfreq(unsigned int freq)
{
        int ret;
        if (radiofd < 0)
                return -1;

        vf.tuner = 0;
        vf.frequency = freq;

        ret = ioctl(radiofd, VIDIOC_S_FREQUENCY, &vf);
        if(ret < 0){
                LOGE("ioctl VIDIOC_S_FREQUENCY fail, %s\n", strerror(errno));
                return -1;
        }
        return 0;
}


static int read_command(char *argv)
{
	char *token = NULL;
	int i = 0;

	count = 0;
	memset(cmd, 0x0, sizeof(cmd));

	while((token = strsep(&argv, " ")) != NULL){
		strcpy(cmd[count++], token);
	}
	return 0;
}

#define CLOSE		(0xA5A5)

struct rds_group {              
       unsigned int blockA;
       unsigned int blockB;
       unsigned int blockC;
       unsigned int blockD;

#define BLOCK_A_FIND    (0x01)
#define BLOCK_B_FIND    (0x02)
#define BLOCK_C_FIND    (0x04)
#define BLOCK_D_FIND    (0x08)
#define BLOCK_ALL_FIND  (0x0F)
        unsigned int findblock;
};

#define V4L2_CID_RDS_GET_MODE	(0x980929)
#define V4L2_CID_RDS_BER_MODE	(0x98092a)

static int radio_rds_get()
{
	struct rds_group rg;
	int ret;
        if (radiofd < 0)
                return -1;

	vctrl.id = V4L2_CID_RDS_GET_MODE;
        vctrl.value = (int)&rg;
        ret = ioctl(radiofd, VIDIOC_S_CTRL, &vctrl);
	if(ret < 0){
                LOGE("%s ioctl VIDIOC_G_CTRL fail, %s\n", __func__, strerror(errno));
                return -1;
        }
	printf("A = 0x%x, D = 0x%x, D = 0x%x, D = 0x%x\n",
			rg.blockA, rg.blockB, rg.blockC, rg.blockD);
        return 0;
}

static int radio_rds_ber(int blocks)
{
        int ret;
        if (radiofd < 0)
                return -1;

        vctrl.id = V4L2_CID_RDS_BER_MODE;
        vctrl.value = blocks;
        ret = ioctl(radiofd, VIDIOC_S_CTRL, &vctrl);
        if(ret < 0){
                LOGE("%s ioctl VIDIOC_G_CTRL fail, %s\n", __func__, strerror(errno));
                return -1;
        }

	printf("get %d error blocks in %d blocks\n", vctrl.value, blocks);
	
        return 0;
}


static int match()
{
	int ret = 0;
	int freq = 0, blocks = 2000;
	struct rds_group rg;
	int time_us;

	if (!strncmp("open", cmd[0], sizeof("open")))
		radio_open();
	else if(!strncmp("muteon", cmd[0], sizeof("muteon"))) {
		radio_mute();	
	} else if (!strncmp("muteoff", cmd[0], sizeof("muteoff"))) {
		radio_unmute();
	} else if (!strncmp("seek", cmd[0], sizeof("seek"))) {
		radio_seek();
	} else if (!strncmp("seek_foo", cmd[0], sizeof("seek_foo"))) {
		radio_setfreq(87500);
		time_us = atoi(cmd[1]);
		usleep(time_us);
		radio_seek();
	} else if (!strncmp("set", cmd[0], sizeof("set"))) {
		freq = atoi(cmd[1]);
		radio_setfreq(freq);
	} else if (!strncmp("get", cmd[0], sizeof("get"))) {
		radio_getfreq();
	} else if (!strncmp("close", cmd[0], sizeof("close"))) {
		ret = -CLOSE;
	} else if (!strncmp("help", cmd[0], sizeof("help"))) {
		printf("usage:\n");
                printf("\tmuteoff\n");
                printf("\tmuteon\n");
                printf("\tseek\n");
                printf("\tset your_freq, eg. set 89700\n");
                printf("\tget\n");
                printf("\trdsget\n");
                printf("\trdsber total_blocks, eg. rdsber 2000\n");
        } else if (!strncmp("rdsget", cmd[0], sizeof("rdsget"))) {
                radio_rds_get();
        } else if (!strncmp("rdsber", cmd[0], sizeof("rdsber"))) {
                blocks = atoi(cmd[1]);
                radio_rds_ber(blocks);
	} 
	return ret;
}

int main(int argc, char *argv[])
{
	char command[128] = { 0 };
	int i = 0;
	int ret = 0;

	if (radio_open() < 0){
                printf("open %s error. %s\n", DEFAULT_DEVICE, strerror(errno));
                return -1;
        }

	while( 1 ) {
		printf("radioap# ");
		i = 0;
		while((command[i++] = getchar()) != '\n' && command[i] != EOF);
		command[i-1] = '\0';
		read_command(command);
		if ((ret = match()) == -CLOSE) {
			break;
		}
	}

	if(radiofd != 0){
		close(radiofd);
	}
	return 0;
}
