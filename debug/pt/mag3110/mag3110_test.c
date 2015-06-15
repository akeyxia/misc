#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/select.h>
#include <linux/input.h>
#include <sys/time.h>
#include <semaphore.h>
#include <hardware/sensors.h>

#define MAG_CTRL_NAME    "m_sensor"//"FreescaleMagnetometer"
#define MAG_DATA_NAME    "eCompass"
#define MAG_DEV_PATH	"/dev/input"
#define MAG_SYSFS_PATH   "/sys/class/input"
#define MAG_SYSFS_DELAY  "poll"
#define MAG_SYSFS_ENABLE "enable"
#define MAG_SYSFS_DR_MODE "dr_mode"
#define MAG_SYSFS_OS_RATE "os_rate"
#define MAG_SYSFS_POSITION "position"
#define MAG_EVENT_X	ABS_X
#define MAG_EVENT_Y	ABS_Y
#define MAG_EVENT_Z	ABS_Z
#define ORN_EVENT_YAW	ABS_RX
#define ORN_EVENT_PITCH	ABS_RY
#define ORN_EVENT_ROLL		ABS_RZ
#define ORN_EVENT_STATUS	ABS_WHEEL

//#define MAG_DATA_CONVERSION(value) (float)((float)(((int)value)/(20.0f)))
#define MAG_DATA_CONVERSION(value) (float)((float)(((int)value)/(10.0f)))

#if 0
/*
 * sensor event data
 */
typedef struct {
    union {
        float v[3];
        struct {
            float x;
            float y;
            float z;
        };
        struct {
            float azimuth;
            float pitch;
            float roll;
        };
    };
    char status;
    unsigned char reserved[3];
} sensors_vec_t;
#endif

typedef enum {
	eNoneEventState = 0,
	eXEventState,
	eYEventState,
	eZEventState,	
} eXYZEventState_t;

typedef enum {
	eMagSettingModeNone = 0,
	eMagSettingModeNormal,
	eMagSettingModeAlignment,
} eMagSettingMode_t;

#define INPUT_NUMS	16
#define PATH_MAX	128
char mClassPath[PATH_MAX];

typedef struct {
	int fd;
	pthread_t thread;
	char actived;
	sem_t sem;

	float x_offset;
	float y_offset;
	char alignment_first_flag;
	char alignment_enable;

	sensors_vec_t magnetic; /* magnetic vector values are in micro-Tesla (uT) */
	float angle;

	size_t numEvents;
    struct input_event* mBuffer;
    struct input_event* mBufferEnd;
    struct input_event* mHead;
    struct input_event* mCurr;
    ssize_t mFreeSpace;
} SensorState;

static SensorState  _sensor_state[1];
static char cmd[50][20];

#define SENSOR_STATE_LOCK(_s)         \
{                                      \
	int ret = 0;                             \
	do {                                 \
		ret = sem_wait(&(_s)->sem);    \
	} while (ret < 0 && errno == EINTR);   \
}

#define SENSOR_STATE_UNLOCK(_s)       \
	sem_post(&(_s)->sem)


#include "fxls8471q_test.c"

int sensor_get_class_path(char *class_path)
{
	char dirname[] = MAG_SYSFS_PATH;
	char buf[256];
	int res;
	DIR *dir;
	struct dirent *de;
	int fd = -1;
	int found = 0;

	dir = opendir(dirname);
	if (dir == NULL)
		return -1;

	while((de = readdir(dir))) {
		if (strncmp(de->d_name, "input", strlen("input")) != 0) {
		    continue;
        }

		sprintf(class_path, "%s/%s", dirname, de->d_name);
		snprintf(buf, sizeof(buf), "%s/name", class_path);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
		    continue;
		}
		if ((res = read(fd, buf, sizeof(buf))) < 0) {
		    close(fd);
		    continue;
		}
		buf[res - 1] = '\0';
		if (strcmp(buf, MAG_CTRL_NAME) == 0) {
		    found = 1;
		    close(fd);
		    break;
		}

		close(fd);
		fd = -1;
	}
	closedir(dir);
	if (found) {
		return 0;
	}else {
		*class_path = '\0';
		return -1;
	}
}

float angle_process(SensorState* state)
{
	float ang;
	float x = state->magnetic.x - state->x_offset;
	float y = state->magnetic.y - state->y_offset;

	if(x == 0) {
		if (y > 0)
			ang = 90.0f;
		else
			ang = 270.0f;
	} else if (y == 0) {
		if (x > 0)
			ang = 0.0f;
		else
			ang = 180.0f;
	} else if ((x > 0) && (y > 0)) {
		ang = atanf(y / x) * 180.0f / M_PI;
	} else if ((x < 0) && (y > 0)) {
		x = -x;
		ang = 180.0f - atan(y / x) * 180.0f / M_PI;
	} else if ((x < 0) && (y < 0)) {
		x = -x;
		y = -y;
		ang = atan(y / x) * 180.0f / M_PI + 180.0f;
	} else if ((x > 0) && (y < 0)) {
		y = -y;
		ang = 360.0f - atan(y / x) * 180.0f / M_PI;
	}

	state->angle = ang;
	return ang;
}

void alignment(SensorState* state)
{
	static float xmax;
	static float xmin;
	static float ymax;
	static float ymin;
	float xdata = state->magnetic.x;
	float ydata = state->magnetic.y;

	if (state->alignment_first_flag) {
		printf("Start a new calibration\n");
		xmax = xmin = xdata;
		ymax = ymin = ydata;
		state->alignment_first_flag = 0;
	}

	if (xdata > xmax)
		xmax = xdata;
	else if (xdata < xmin)
		xmin = xdata;

	if (ydata > ymax)
		ymax = ydata;
	else if (ydata < ymin)
		ymin = ydata;

	state->x_offset = (xmax + xmin) / 2.0f;
	state->y_offset = (ymax + ymin) / 2.0f;

	printf("x[%.2f, %.2f], y[%.2f, %.2f]: xdata = %.2f, ydata = %.2f, x_offset = %.2f, y_offset = %.2f\n",
		xmin, xmax, ymin, ymax, state->magnetic.x, state->magnetic.y, state->x_offset, state->y_offset);
}


int writeDelay(int ms) {
	char attr[PATH_MAX] = {'\0'};
	if(mClassPath[0] == '\0')
		return -1;

	strcpy(attr, mClassPath);
	strcat(attr,"/");
	strcat(attr,MAG_SYSFS_DELAY);

	int fd = open(attr, O_RDWR);
	if (0 > fd) {
		printf("Could not open (write-only) SysFs attribute \"%s\" (%s).", attr, strerror(errno));
		return -errno;
	}

    char buf[80];
    sprintf(buf, "%d", ms);
    write(fd, buf, strlen(buf)+1);
    close(fd);
    return 0;
}

int writeEnable(int isEnable) {
	SensorState* state = _sensor_state;

	char attr[PATH_MAX] = {'\0'};
	if(mClassPath[0] == '\0')
		return -1;

	strcpy(attr, mClassPath);
	strcat(attr,"/");
	strcat(attr,MAG_SYSFS_ENABLE);

	int fd = open(attr, O_RDWR);
	if (0 > fd) {
		printf("Could not open (write-only) SysFs attribute \"%s\" (%s).", attr, strerror(errno));
		return -errno;
	}

	char buf[2];

	if (isEnable) {
		buf[0] = '1';
	} else {
		buf[0] = '0';
	}
	buf[1] = '\0';

	int err = 0;
	err = write(fd, buf, sizeof(buf));

	if (0 > err) {
		err = -errno;
		printf("Could not write SysFs attribute \"%s\" (%s).", attr, strerror(errno));
	} else {
		err = 0;
	}

	close(fd);

	//printf("lock start\n");
	//SENSOR_STATE_LOCK(state);
	//printf("lock protect area\n");
	state->actived = isEnable;
	//SENSOR_STATE_UNLOCK(state);
	//printf("lock end\n");

	return err;
}

int writeDR_MODE(int dr_mode) {
	char attr[PATH_MAX] = {'\0'};
	if(mClassPath[0] == '\0')
		return -1;

	if (dr_mode > 7) {
		printf("please check the value of dr_mode, range [0, 7]\n");	
		return -2;
	}

	strcpy(attr, mClassPath);
	strcat(attr,"/");
	strcat(attr,MAG_SYSFS_DR_MODE);

	int fd = open(attr, O_RDWR);
	if (0 > fd) {
		printf("Could not open (write-only) SysFs attribute \"%s\" (%s).", attr, strerror(errno));
		return -errno;
	}

	char buf[2];

	buf[0] = '0' + dr_mode;
	buf[1] = '\0';

	int err = 0;
	err = write(fd, buf, sizeof(buf));

	if (0 > err) {
		err = -errno;
		printf("Could not write SysFs attribute \"%s\" (%s).", attr, strerror(errno));
	} else {
		err = 0;
	}

	close(fd);

	return err;
}

int writePOSITION(int position) {
	char attr[PATH_MAX] = {'\0'};
	if(mClassPath[0] == '\0')
		return -1;

	if (position > 7) {
		printf("please check the value of position, range [0, 7]\n");	
		return -2;
	}

	strcpy(attr, mClassPath);
	strcat(attr,"/");
	strcat(attr,MAG_SYSFS_POSITION);

	int fd = open(attr, O_RDWR);
	if (0 > fd) {
		printf("Could not open (write-only) SysFs attribute \"%s\" (%s).", attr, strerror(errno));
		return -errno;
	}

	char buf[2];

	buf[0] = '0' + position;
	buf[1] = '\0';

	int err = 0;
	err = write(fd, buf, sizeof(buf));

	if (0 > err) {
		err = -errno;
		printf("Could not write SysFs attribute \"%s\" (%s).", attr, strerror(errno));
	} else {
		err = 0;
	}

	close(fd);

	return err;
}

int writeOS_RATE(int os_rate) {
	char attr[PATH_MAX] = {'\0'};
	if(mClassPath[0] == '\0')
		return -1;

	if (os_rate > 3) {
		printf("please check the value of os_rate, range [0, 3]\n");	
		return -2;
	}

	strcpy(attr, mClassPath);
	strcat(attr,"/");
	strcat(attr,MAG_SYSFS_OS_RATE);

	int fd = open(attr, O_RDWR);
	if (0 > fd) {
		printf("Could not open (write-only) SysFs attribute \"%s\" (%s).", attr, strerror(errno));
		return -errno;
	}

	char buf[2];

	buf[0] = '0' + os_rate;
	buf[1] = '\0';

	int err = 0;
	err = write(fd, buf, sizeof(buf));

	if (0 > err) {
		err = -errno;
		printf("Could not write SysFs attribute \"%s\" (%s).", attr, strerror(errno));
	} else {
		err = 0;
	}

	close(fd);

	return err;
}

static void configureSettingMode(eMagSettingMode_t mode)
{
	static eMagSettingMode_t last_mode = eMagSettingModeNone;

	if (mode == last_mode) {
		//printf("the same Setting Mode\n");
		return;	
	} else {
		printf("Setting Mode: %d\n", mode);
		last_mode = mode;	
	}

	switch (mode) {
	case eMagSettingModeNormal:
		writeEnable(0);
		writeDelay(100);
		writeDR_MODE(0);
		writeOS_RATE(3);
		break;
	case eMagSettingModeAlignment:
		writeEnable(0);
		writeDelay(5);
		writeDR_MODE(0);
		writeOS_RATE(0);
		break;
	default:
		printf("Unknow MagSettingMode\n");
		break;	
	}

	return;
}

static long get_time2ms(void)
{
	struct timeval tv;
	long time;

	gettimeofday(&tv, NULL);

	time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	return time;
}

void processEvent(SensorState* state, int code, int value)
{
	static eXYZEventState_t event_state = eNoneEventState;
	long last_time;
	long div_time;

	switch (code) {
		case MAG_EVENT_X:
			event_state++;
			if (event_state != eXEventState)
				event_state = eNoneEventState;
			state->magnetic.x = MAG_DATA_CONVERSION(value);
            break;
        case MAG_EVENT_Y:
        	event_state++;
			if (event_state != eYEventState)
				event_state = eNoneEventState;
        	state->magnetic.y = MAG_DATA_CONVERSION(value);
            break;
        case MAG_EVENT_Z:
        	event_state++;
			if (event_state != eZEventState)
				event_state = eNoneEventState;
			state->magnetic.z = MAG_DATA_CONVERSION(value);
            break;
     }

	if (event_state == eZEventState) {
		div_time = get_time2ms() - last_time;
		printf("[t: %d.%d s]------> ", div_time / 1000, div_time % 1000);

		if (state->alignment_enable) {
			configureSettingMode(eMagSettingModeAlignment);
			alignment(state);
		} else {
			configureSettingMode(eMagSettingModeNormal);

			printf("x_offset = %.2f, y_offset = %.2f,"
				"xdata = %.2f, ydata = %.2f, Angle = %.2f\n",
				state->x_offset, state->y_offset,
				state->magnetic.x, state->magnetic.y, angle_process(state));
		}

		event_state = eNoneEventState;
	}

	last_time = get_time2ms();

	return;
}

ssize_t fill(SensorState* state)
{
	size_t numEventsRead = 0;

    if (state->mFreeSpace) {
        const ssize_t nread = read(state->fd, state->mHead, state->mFreeSpace * sizeof(struct input_event));
        if (nread<0 || nread % sizeof(struct input_event)) {
            // we got a partial event!!
            return nread<0 ? -errno : -EINVAL;
        }

        numEventsRead = nread / sizeof(struct input_event);
        if (numEventsRead) {
            state->mHead += numEventsRead;
            state->mFreeSpace -= numEventsRead;
            if (state->mHead > state->mBufferEnd) {
                size_t s = state->mHead - state->mBufferEnd;
                memcpy(state->mBuffer, state->mBufferEnd, s * sizeof(struct input_event));
                state->mHead = state->mBuffer + s;
            }
        }
    }

    return numEventsRead;
}

ssize_t readEvent(SensorState* state, struct input_event const** events)
{
    *events = state->mCurr;
    ssize_t available = (state->mBufferEnd - state->mBuffer) - state->mFreeSpace;
    return available ? 1 : 0;
}

void next(SensorState* state)
{
    state->mCurr++;
    state->mFreeSpace++;
    if (state->mCurr >= state->mBufferEnd) {
        state->mCurr = state->mBuffer;
    }
}

static void sensor_state_thread(void *arg)
{
	SensorState* state = (SensorState *)arg;
	int fd = state->fd;
	int ret;
	ssize_t nread;
	size_t numEventsRead;

	printf("Sensor thread running\n");

	while(1) {
		do {
			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(fd, &readfds);
			size_t numEventsRead = 0;
			struct input_event * event;

			ret = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
			if (ret < 0)
				continue;

			if (FD_ISSET(fd, &readfds))
				//nread = read(fd, state->mHead, state->mFreeSpace * sizeof(struct input_event));
				numEventsRead = fill(state);

			while(readEvent(state, &event)) {
				int type = event->type;
				//printf("type = %d, code = %d, value = %d\n", type, event->code, event->value);
		        //if ((type == EV_ABS) || (type == EV_REL) || (type == EV_KEY)) {
		        if (type == EV_ABS && state->actived) {
		            processEvent(state, event->code, event->value);
		            next(state);
		        } else {
		        	next(state);	
		        }
			};
		} while(ret < 0 && errno == EINTR);
	}
}

static void sensor_state_done(SensorState* state)
{
	void*  dummy;

	pthread_join(state->thread, &dummy);
	free(state->mBuffer);
	sem_destroy(&state->sem);

	if (state->fd > 0) {
		close(state->fd);
		state->fd = -1;
	}

	printf("sensor deinit completed\n");
}

int openInput(const char* inputName) {
    int fd = -1;
    int input_id = -1;
    const char *dirname = "/dev/input";
    const char *inputsysfs = "/sys/class/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;

    dir = opendir(dirname);
    if(dir == NULL)
      return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
                    (de->d_name[1] == '\0' ||
                     (de->d_name[1] == '.' && de->d_name[2] == '\0')))
          continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);

        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }

            if (!strcmp(name, inputName)) {
                //strcpy(input_name, filename);
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);
    if (fd < 0)
    	printf("couldn't find '%s' input device", inputName);
    else
    	printf("input dev name: %s\n", devname);

    return fd;
}


static void sensor_state_init(SensorState* state)
{
	int ret;
	char file[128] = { 0 };

	printf("mClassPath = %s\n", mClassPath);
	state->fd = openInput(MAG_CTRL_NAME);
	if (state->fd < 0)
	{
		printf("could not open dev: %s\n", strerror(errno) );
		return;
	}

	state->alignment_enable = 0;
	state->x_offset = 0.0f;
	state->y_offset = 0.0f;
	configureSettingMode(eMagSettingModeNormal);

	state->numEvents = INPUT_NUMS;
	state->mBuffer = (struct input_event *)malloc(sizeof(struct input_event) * state->numEvents * 2);
	state->mBufferEnd = state->mBuffer + state->numEvents;
	state->mHead = state->mCurr = state->mBuffer;
	state->mFreeSpace = state->numEvents;

	if (sem_init(&state->sem, 0, 1) != 0) {
		printf("sensor semaphore initialization failed! errno = %d", errno);
		return;
	}

	ret = state->thread = pthread_create(&state->thread, NULL, (void *)sensor_state_thread, (void*)state);
	if (ret)
	{
		printf("could not create sensor thread: %s\n", strerror(errno));
		goto err;
	}

	printf("sensor state initialized\n");

	return;

err:
	sensor_state_done(state);
}

static int sensor_init(void)
{
	SensorState* state = _sensor_state;

	sensor_state_init(state);

	if (state->fd < 0)
		return -1;

	return 0;
}

#if 0
/*
 * eCompass Source code from AN4248
 */
typedef short Int16;


/* roll pitch and yaw angles computed by iecompass */
static Int16 iPhi, iThe, iPsi;
/* magnetic field readings corrected for hard iron effects and PCB orientation */
static Int16 iBfx, iBfy, iBfz;
/* hard iron estimate */
static Int16 iVx, iVy, iVz;
/* tilt-compensated e-Compass code */
public static void iecompass(Int16 iBpx, Int16 iBpy, Int16 iBpz,
Int16 iGpx, Int16 iGpy, Int16 iGpz)
{
	/* stack variables */
	/* iBpx, iBpy, iBpz: the three components of the magnetometer sensor */
	/* iGpx, iGpy, iGpz: the three components of the accelerometer sensor */
	/* local variables */
	Int16 iSin, iCos; /* sine and cosine */
	/* subtract the hard iron offset */
	iBpx -= iVx; /* see Eq 16 */
	iBpy -= iVy; /* see Eq 16 */
	iBpz -= iVz; /* see Eq 16 */
	
	/* calculate current roll angle Phi */
	iPhi = iHundredAtan2Deg(iGpy, iGpz);/* Eq 13 */
	/* calculate sin and cosine of roll angle Phi */
	iSin = iTrig(iGpy, iGpz); /* Eq 13: sin = opposite / hypotenuse */
	iCos = iTrig(iGpz, iGpy); /* Eq 13: cos = adjacent / hypotenuse */
	/* de-rotate by roll angle Phi */
	iBfy = (Int16)((iBpy * iCos - iBpz * iSin) >> 15);/* Eq 19 y component */
	iBpz = (Int16)((iBpy * iSin + iBpz * iCos) >> 15);/* Bpy*sin(Phi)+Bpz*cos(Phi)*/
	iGpz = (Int16)((iGpy * iSin + iGpz * iCos) >> 15);/* Eq 15 denominator */
	/* calculate current pitch angle Theta */
	iThe = iHundredAtan2Deg((Int16)-iGpx, iGpz);/* Eq 15 */
	/* restrict pitch angle to range -90 to 90 degrees */
	if (iThe > 9000) iThe = (Int16) (18000 - iThe);
	if (iThe < -9000) iThe = (Int16) (-18000 - iThe);
	/* calculate sin and cosine of pitch angle Theta */
	iSin = (Int16)-iTrig(iGpx, iGpz); /* Eq 15: sin = opposite / hypotenuse */
	iCos = iTrig(iGpz, iGpx); /* Eq 15: cos = adjacent / hypotenuse */
	/* correct cosine if pitch not in range -90 to 90 degrees */
	if (iCos < 0) iCos = (Int16)-iCos;
	/* de-rotate by pitch angle Theta */
	iBfx = (Int16)((iBpx * iCos + iBpz * iSin) >> 15); /* Eq 19: x component */
	iBfz = (Int16)((-iBpx * iSin + iBpz * iCos) >> 15);/* Eq 19: z component */
	/* calculate current yaw = e-compass angle Psi */
	iPsi = iHundredAtan2Deg((Int16)-iBfy, iBfx); /* Eq 22 */

}
#endif

static int match(int argc)
{
	int ret = 0;
	SensorState* state = _sensor_state;
	int isEnable;
	char buf[64] = { 0 };

	if(!strncmp("enable", cmd[0], sizeof("enable"))) {
		state->actived = isEnable = !!atoi(cmd[1]);
		writeEnable(isEnable);
	} else if (!strncmp("debug", cmd[0], sizeof("debug"))) {
		sprintf(buf, "echo 0x%x > /sys/module/mag3110/parameters/debug",
			strtoll(cmd[1], NULL, 16));
		system(buf);
    } else if (!strncmp("poll", cmd[0], sizeof("poll"))) {
    	writeDelay(atoi(cmd[1]));
    } else if (!strncmp("dr_mode", cmd[0], sizeof("dr_mode"))) {
    	writeDR_MODE(atoi(cmd[1]));
    } else if (!strncmp("os_rate", cmd[0], sizeof("os_rate"))) {
    	writeOS_RATE(atoi(cmd[1]));
    } else if (!strncmp("position", cmd[0], sizeof("position"))) {
    	writePOSITION(atoi(cmd[1]));
    } else if (!strncmp("convert", cmd[0], sizeof("convert"))) {
		sprintf(buf, "echo %d > %s/convert", atoi(cmd[1]), mClassPath);
		system(buf);
    } else if (!strncmp("read_poll", cmd[0], sizeof("read_poll"))) {
		sprintf(buf, "cat %s/poll", mClassPath);
		system(buf);
    } else if (!strncmp("read_dr_mode", cmd[0], sizeof("read_dr_mode"))) {
		sprintf(buf, "cat %s/dr_mode", mClassPath);
		system(buf);
    } else if (!strncmp("read_os_rate", cmd[0], sizeof("read_os_rate"))) {
		sprintf(buf, "cat %s/os_rate", mClassPath);
		system(buf);
    } else if (!strncmp("read_position", cmd[0], sizeof("read_position"))) {
		sprintf(buf, "cat %s/position", mClassPath);
		system(buf);
    } else if (!strncmp("read_convert", cmd[0], sizeof("read_convert"))) {
		sprintf(buf, "cat %s/convert", mClassPath);
		system(buf);
    } else if (!strncmp("getoffset", cmd[0], sizeof("getoffset"))) {
    	printf("x_offset = %.2f, y_offset = %.2f\n",
    		state->x_offset, state->y_offset);
    } else if (!strncmp("getangle", cmd[0], sizeof("getangle"))) {
    	printf("angle = %.2f\n", state->angle);
    } else if (!strncmp("calibration", cmd[0], sizeof("calibration"))) {
		state->alignment_enable = !!atoi(cmd[1]);
		if (state->alignment_enable) {
			state->alignment_first_flag = 1;
			configureSettingMode(eMagSettingModeAlignment);
		} else {
			state->actived = 0;
			writeEnable(0);
			configureSettingMode(eMagSettingModeNormal);
		}
    } else if (!strncmp("close", cmd[0], sizeof("close"))) {
    	writeEnable(0);
		ret = -1;
	} else if (!strncmp("help", cmd[0], sizeof("help"))) {
		printf("usage:\n");
		printf("enable x, range[0, 1]\n");
		printf("debug 0xXX\n");
		printf("poll x, range[1, 500]\n");
		printf("dr_mode x, range[0, 7]\n");
		printf("os_rate x, range[0, 3]\n");
		printf("positon x, range[0, 7]\n");
		printf("convert x, range[0, 1]\n");
		printf("getoffset\n");
		printf("getangle\n");
		printf("read_poll\n");
		printf("read_dr_mode\n");
		printf("read_os_rate\n");
		printf("read_positon\n");
		printf("read_convert x\n");
		printf("calibration x, range[0, 1]\n");
		printf("\tclose sensor test, eg. close\n");
	} else {
		printf("\n >>>>>>>>>> unkonw command <<<<<<<<<<\n\n");
	}

	return ret;
}

static int _match(int argc)
{
	int ret = 0;

	if (!strncmp("hwseek", cmd[0], sizeof("hwseek"))) {
		
	}

	cmd[0][0] = 0;

	return ret;
}

void match_func(void * arg)
{
	int argc = *(int *)arg;

	while(1) {
		_match(argc);
		sleep(3);
	}
}

static int read_command(char *argv)
{
	int count = 0;
	char *token = NULL;
	int i = 0;

	memset(cmd, 0x0, sizeof(cmd));

	while((token = strsep(&argv, " ")) != NULL){
		strcpy(cmd[count++], token);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char command[128] = { 0 };
	int i = 0;
	int ret = 0;
	pthread_t thread;

	ret = sensor_get_class_path(mClassPath);
	if(ret) {
		printf("not found sensor\n");
		return -1;
	}

	sensor_init();
	accel_sensor_init();

	ret = pthread_create(&thread, NULL, (void *)match_func, &argc);
	if (ret) {
		printf("Create Test Thread Error!\n");
		goto end;
	}

	while( 1 ) {
		printf("sensor# ");
		i = 0;
		while((command[i++] = getchar()) != '\n' && command[i] != EOF);
		command[i-1] = '\0';
		read_command(command);
		if ((ret = match(argc)) == -1) {
			break;
		}
	}

end:

	sensor_state_done(_sensor_state);

	return 0;
}
