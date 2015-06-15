#define ACC_DATA_NAME    "g_sensor"
#define ACC_SYSFS_PATH   "/sys/class/input"
#define ACC_SYSFS_DELAY  "poll"
#define ACC_SYSFS_ENABLE "enable"
#define ACC_EVENT_X REL_RX
#define ACC_EVENT_Y REL_RY
#define ACC_EVENT_Z REL_RZ
#define ACC_DATA_CONVERSION(value) (float)((float)((int)value) * (GRAVITY_EARTH / (0x4000)))

#define ACCEL_INPUT_NUMS	4
char gClassPath[128];

typedef enum {
	eAccelNoneEventState = 0,
	eAccelXEventState,
	eAccelYEventState,
	eAccelZEventState,	
} eAccelXYZEventState_t;

typedef struct {
	int fd;
	pthread_t thread;
	char actived;
	sem_t sem;

    sensors_vec_t acceleration;/* acceleration values are in meter per second per second (m/s^2) */
	float angle;

	size_t numEvents;
    struct input_event* mBuffer;
    struct input_event* mBufferEnd;
    struct input_event* mHead;
    struct input_event* mCurr;
    ssize_t mFreeSpace;
} AccelSensorState;

static AccelSensorState  _accel_sensor_state[1];

#define ACCEL_SENSOR_STATE_LOCK(_s)         \
{                                      \
	int ret = 0;                             \
	do {                                 \
		ret = sem_wait(&(_s)->sem);    \
	} while (ret < 0 && errno == EINTR);   \
}

#define ACCEL_SENSOR_STATE_UNLOCK(_s)       \
	sem_post(&(_s)->sem)

int accel_writeEnable(int isEnable) {
	AccelSensorState* state = _accel_sensor_state;

	char attr[PATH_MAX] = {'\0'};
	if(gClassPath[0] == '\0')
		return -1;

	strcpy(attr, gClassPath);
	strcat(attr,"/");
	strcat(attr,ACC_SYSFS_ENABLE);

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

	state->actived = isEnable;

	return err;
}

int accel_writeDelay(int64_t ns) {
	char attr[PATH_MAX] = {'\0'};
	if(gClassPath[0] == '\0')
		return -1;

	strcpy(attr, gClassPath);
	strcat(attr,"/");
	strcat(attr,ACC_SYSFS_DELAY);

	int fd = open(attr, O_RDWR);
	if (0 > fd) {
		printf("Could not open (write-only) SysFs attribute \"%s\" (%s).", attr, strerror(errno));
		return -errno;
	}
	if (ns > 10240000000LL) {
		ns = 10240000000LL; /* maximum delay in nano second. */
	}
	if (ns < 312500LL) {
		ns = 312500LL; /* minimum delay in nano second. */
	}

    char buf[80];
    sprintf(buf, "%lld", ns/1000/1000);
    write(fd, buf, strlen(buf)+1);
    close(fd);
    return 0;
}


int accel_sensor_get_class_path(char *class_path)
{
	char dirname[] = ACC_SYSFS_PATH;
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
		if (strcmp(buf, ACC_DATA_NAME) == 0) {
		    found = 1;
		    close(fd);
		    break;
		}

		close(fd);
		fd = -1;
	}
	closedir(dir);
	//ALOGE("the G sensor dir is %s",class_path);

	if (found) {
		return 0;
	}else {
		*class_path = '\0';
		return -1;
	}
}

static long accel_get_time2ms(void)
{
	struct timeval tv;
	long time;

	gettimeofday(&tv, NULL);

	time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	return time;
}

float accel_angle_process(AccelSensorState* state)
{
	float ang;


	state->angle = ang;
	return ang;
}

void accel_processEvent(AccelSensorState* state, int code, int value)
{
	static eAccelXYZEventState_t event_state = eAccelNoneEventState;
	long last_time;
	long div_time;

	switch (code) {
		case ACC_EVENT_X:
			event_state++;
			if (event_state != eXEventState)
				event_state = eNoneEventState;
			state->acceleration.x = ACC_DATA_CONVERSION(value);
            break;
        case ACC_EVENT_Y:
        	event_state++;
			if (event_state != eYEventState)
				event_state = eNoneEventState;
        	state->acceleration.y = ACC_DATA_CONVERSION(value);
            break;
        case ACC_EVENT_Z:
        	event_state++;
			if (event_state != eZEventState)
				event_state = eNoneEventState;
			state->acceleration.z = ACC_DATA_CONVERSION(value);
            break;
     }

	if (event_state == eAccelZEventState) {
		div_time = accel_get_time2ms() - last_time;
		printf(">>> [t: %d.%d s] ", div_time / 1000, div_time % 1000);

		printf("xdata = %.2f, ydata = %.2f, zdata = %.2f, Angle = %.2f\n",
			state->acceleration.x, state->acceleration.y,
			state->acceleration.z, accel_angle_process(state));
	}
	last_time = accel_get_time2ms();

	return;
}

ssize_t accel_fill(AccelSensorState* state)
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

ssize_t accel_readEvent(AccelSensorState* state, struct input_event const** events)
{
    *events = state->mCurr;
    ssize_t available = (state->mBufferEnd - state->mBuffer) - state->mFreeSpace;
    return available ? 1 : 0;
}

void accel_next(AccelSensorState* state)
{
    state->mCurr++;
    state->mFreeSpace++;
    if (state->mCurr >= state->mBufferEnd) {
        state->mCurr = state->mBuffer;
    }
}

static void accel_sensor_state_thread(void *arg)
{
	AccelSensorState* state = (AccelSensorState *)arg;
	int fd = state->fd;
	int ret;
	ssize_t nread;
	size_t numEventsRead;

	printf("%s: Sensor thread running\n", __func__);

	while(1) {
		do {
			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(fd, &readfds);
			size_t numEventsRead = 0;
			struct input_event const* event;

			ret = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
			if (ret < 0)
				continue;

			if (FD_ISSET(fd, &readfds))
				//nread = read(fd, state->mHead, state->mFreeSpace * sizeof(struct input_event));
				numEventsRead = accel_fill(state);

			while(accel_readEvent(state, &event)) {
				int type = event->type;
				//printf("type = %d, code = %d, value = %d\n", type, event->code, event->value);
		        //if ((type == EV_ABS) || (type == EV_REL) || (type == EV_KEY)) {
		        if (type == EV_ABS && state->actived) {
		            accel_processEvent(state, event->code, event->value);
		            accel_next(state);
		        } else {
		        	accel_next(state);	
		        }
			};
		} while(ret < 0 && errno == EINTR);
	}
}

static void accel_sensor_state_done(AccelSensorState* state)
{
	void*  dummy;

	pthread_join(state->thread, &dummy);
	free(state->mBuffer);
	sem_destroy(&state->sem);

	if (state->fd > 0) {
		close(state->fd);
		state->fd = -1;
	}

	printf("%s: sensor deinit completed\n", __func__);
}

int accel_openInput(const char* inputName) {
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
    	printf("%s: couldn't find '%s' input device", __func__, inputName);
    else
    	printf("%s: input dev name: %s\n", __func__, devname);

    return fd;
}


static void accel_sensor_state_init(AccelSensorState* state)
{
	int ret;
	char file[128] = { 0 };

	printf("%s: gClassPath = %s\n", __func__, gClassPath);
	state->fd = accel_openInput(ACC_DATA_NAME);
	if (state->fd < 0)
	{
		printf("%s: could not open dev: %s\n", __func__, strerror(errno) );
		return;
	}

	/* sensor state init */

	state->numEvents = ACCEL_INPUT_NUMS;
	state->mBuffer = (struct input_event *)malloc(sizeof(struct input_event) * state->numEvents * 2);
	state->mBufferEnd = state->mBuffer + state->numEvents;
	state->mHead = state->mCurr = state->mBuffer;
	state->mFreeSpace = state->numEvents;

	if (sem_init(&state->sem, 0, 1) != 0) {
		printf("%s: sensor semaphore initialization failed! errno = %d", __func__, errno);
		return;
	}

	ret = state->thread = pthread_create(&state->thread, NULL, (void *)accel_sensor_state_thread, (void*)state);
	if (ret)
	{
		printf("%s: could not create sensor thread: %s\n", __func__, strerror(errno));
		goto err;
	}

	printf("%s: sensor state initialized\n", __func__);

	return;

err:
	accel_sensor_state_done(state);
}

static int accel_sensor_init(void)
{
	AccelSensorState* state = _accel_sensor_state;

	accel_sensor_state_init(state);

	if (state->fd < 0)
		return -1;

	return 0;
}
