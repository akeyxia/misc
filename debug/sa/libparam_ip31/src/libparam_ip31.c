#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/ioctl.h>
#include "libparam_ip31.h"
#include <mtd/mtd-user.h>

#define LOGE(fmt,args...)     printf (fmt ,##args)
#define LOGD(fmt,args...)     printf (fmt ,##args)
#define ISINIT(a) do{if(a <0) LOGE("please call init first.\n");}while(0)

/*belowing defined the param part data chart --start*/
#define PARAM_PART "/dev/mtd3"

#define SECTOR_SIZE     (256 * 1024) //256kB, used to store info
#define ERASE_SIZE      (256 * 1024) //256kB, flash specific

/*1st chart is boot_param */
#define BOOT_PARAM_OFFSET_S 0x0
#define BOOT_PARAM_SIZE 0x200 //512 byte
/*2nd chart is device_id */
#define DEVICE_ID_OFFSET_S (BOOT_PARAM_OFFSET_S+BOOT_PARAM_SIZE)
#define DEVICE_ID_SIZE 0x800
/*belowing defined the param part data chart --end*/

#define CMDLINE_OFFSET 132
#define NULL_DATA 0x0

struct param_hdr
{
    char type[32]; //current system type systemA or systemB
    char status[100]; //
    char cmd_line[256]; //recovery command eg:recovery--wap-data, recovery--update-mpu
    char reserved[124]; //reserved data
};

struct device_id_list_str
{
    char device_id[64];//device id
    char bt_addr[32]; //bt addr
    char wifi_addr[32]; //wifi addr
    char reserved[1920]; //reserved data
};


char param_string[][256] =
{
    "recovery--wipe_data", //19 char
    "recovery--upgrade_mpu", //21 char
    "recovery--upgrade_mcu", //21 char
    "recovery--upgrade_map", //21 char
    "recovery--upgrade_RL78", //22 char
    "recovery--upgrade_tsfw", //22 char
    "recovery--upgrade_gpsfw", //23 char
    "fastboot"//14 char boot into uboot fastboot mode
};

/*param part read/write ops*/

int fd = -1;

int get_boot_param_string(int menu, char *buf)
{
    //char *buf = (char *)malloc(256);

    strncpy(buf, param_string[menu], strlen(param_string[menu]));

    buf[strlen(param_string[menu]) + 1] = '\0';

    return 0;
}
int get_device_id_prop(int device_id, int *offset, int *size)
{
    switch(device_id) {
    case E_DEVICE_ID:
        *offset = DEVICE_ID_OFFSET_S;
        *size = 64;
        break;
    case E_BT_ADDR:
        *offset = DEVICE_ID_OFFSET_S+64;
        *size = 32;
        break;
    case E_WIFI_ADDR:
        *offset = DEVICE_ID_OFFSET_S+96;
        *size = 32;
        break;
    case E_ID_MAX:
    default:
        ;
    }
    return E_OK;
}

int param_read(int in_offset, char *in_data, int in_size)
{
    int ret = 0;
    int read_offset = in_offset;
    int read_size = in_size;
    char *read_data = in_data;

    int getoffset = lseek(fd, read_offset, SEEK_SET);
    if(getoffset != read_offset) {
        LOGE("read error : %s\n", strerror(errno));
        ret = E_LSEEK;
    }

    ret = read(fd, read_data, read_size);
    if (ret != read_size)
    {
        LOGE("read error : %s\n", strerror(errno));
        ret = E_WRITE;
    }

    return ret;
}

int param_write(int in_offset, char *in_data, int in_size)
{
    int ret = 0;
    int write_offset = in_offset;
    int write_size = in_size;
    char *write_data = in_data;

    int getoffset = lseek(fd, write_offset, SEEK_SET);
    if(getoffset != write_offset) {
        LOGE("write error : %s\n", strerror(errno));
        ret = E_LSEEK;
    }

    ret = write(fd, write_data, write_size);
    if (ret != write_size)
    {
        LOGE("write error : %s\n", strerror(errno));
        ret = E_WRITE;
    }

    return ret;
}

int read_param(struct param_hdr *param)
{
    char *buf = NULL;
    int ret = 0;
    ISINIT(fd);

    if (NULL == (buf = (char *)malloc(BOOT_PARAM_SIZE)))
    {
        LOGE("malloc error : %s\n", strerror(errno));
        ret = -1;
        goto err;
    }

    memset(buf, NULL_DATA, BOOT_PARAM_SIZE);
    lseek(fd, BOOT_PARAM_OFFSET_S, SEEK_SET);
    ret = read(fd, buf, BOOT_PARAM_SIZE);
    if (ret != BOOT_PARAM_SIZE)
    {
        LOGE("read error : %s\n", strerror(errno));
        ret = -2;
        goto err;
    }
    memcpy(param, buf, BOOT_PARAM_SIZE);

err:
    if (buf)
    {
        free(buf);
    }
    return ret;
}

int test_get_all()
{
    int ret = -1;
    struct param_hdr param;
    char buf[BOOT_PARAM_SIZE] = {0};
    int read_offset = BOOT_PARAM_OFFSET_S;

    ret = param_read(read_offset, buf, BOOT_PARAM_SIZE);
    if (ret < 0)
    {
        LOGE("get error:%d\n", ret);
        return ret;
    }

    memcpy(&param, buf, BOOT_PARAM_SIZE);

    printf("param.type[%s]\n", param.type);
    printf("param.status[%s]\n", param.status);
    printf("param.cmd_line[%s]\n", param.cmd_line);
    printf("param.reserved[%s]\n", param.reserved);

    return E_OK;
}

E_RET_VALUE_LIST ip31_param_init()
{
    fd= open(PARAM_PART, O_RDWR);
    if (fd< 0)
    {
        LOGE("open block error, %s\n", strerror(errno));
        return E_OPEN;
    }

    return E_OK;
}

E_RET_VALUE_LIST ip31_param_uninit()
{
    int ret = -1;
    ISINIT(fd);
    ret = close(fd);
    if (ret < 0)
    {
        LOGE("close block error, %s\n", strerror(errno));
        return E_CLOSE;
    }
    return E_OK;
}

E_RET_VALUE_LIST ip31_boot_param_reset()
{
    int ret;
    struct param_hdr default_param = {"SYSTEMA", "DEFAULT", "------", "------"};

    int write_offset = BOOT_PARAM_OFFSET_S;
    char buf[512] = {0};

    memcpy(buf, &default_param, BOOT_PARAM_SIZE);
    ret = param_write(write_offset, buf, BOOT_PARAM_SIZE);
    if (ret != BOOT_PARAM_SIZE)
    {
        LOGE("reset error : %s\n", strerror(errno));
        return E_RESET;
    }

    return E_OK;
}
E_RET_VALUE_LIST ip31_boot_param_set(E_BOOT_PARAM_LIST item)
{
    int ret = -1;
    char buf[256] = {0};
    int write_offset = BOOT_PARAM_OFFSET_S + CMDLINE_OFFSET;

    get_boot_param_string(item, buf);

    ret = param_write(write_offset, buf, 256);
    if (ret < 0)
    {
        LOGE("return error: %d\n", ret);
        return ret;
    }

    return E_OK;
}

E_RET_VALUE_LIST ip31_boot_param_get(E_BOOT_PARAM_LIST item, char *dest)
{
    int ret = -1;
    struct param_hdr param;
    char buf[BOOT_PARAM_SIZE] = {0};
    int read_offset = BOOT_PARAM_OFFSET_S;

    if (dest == NULL)
    {
        LOGE("input dest is null: %s\n", strerror(errno));
        return E_NULL;
    }
    ret = param_read(read_offset, buf, BOOT_PARAM_SIZE);
    if (ret < 0)
    {
        LOGE("get error:%d\n", ret);
        return ret;
    }

    memcpy(&param, buf, BOOT_PARAM_SIZE);
    memcpy(dest, param.cmd_line, 256 );

    return E_OK;
}

static int flash_erase(int fd, int start, int length)
{
	struct erase_info_user erase;

	int istart = start / ERASE_SIZE;
	int iend = (start + length) / ERASE_SIZE;

	if((start + length) % ERASE_SIZE)
		iend++;

	//LOGE("%s: istart = %d, iend = %d\n", __func__, istart, iend);

	for(; istart < iend; istart++){
		erase.start = istart * ERASE_SIZE;
		erase.length = ERASE_SIZE;
		if (ioctl(fd, MEMERASE, &erase) != 0) {
			LOGE("ioctl error: %s\n", strerror(errno));
			return -2;
		}
	}
	return 0;
}


static int flash_write(int start, int length, char *inbuf)
{
	int istart = start / ERASE_SIZE;
	int iend = (start + length) / ERASE_SIZE;
        int ilength;
	char *buf = NULL;
	int ret = 0;
        int read_sector;

	if((start + length) % ERASE_SIZE)
		iend++;
        ilength = iend - istart;

	//LOGE("%s: istart = %d, iend = %d\n", __func__, istart, iend);

	if (NULL == (buf = malloc(ilength * ERASE_SIZE))) {
		LOGE("malloc error : %s\n", strerror(errno));
		ret = -1;
		goto err;
	}
	memset(buf, 0x0, ilength * ERASE_SIZE);


        lseek(fd, 0, SEEK_SET);

	ret = read(fd, buf, ilength * ERASE_SIZE);
	if(ret != ilength * ERASE_SIZE){
		LOGE("read error : %s\n", strerror(errno));
		ret = -2;
		goto err;
	}

	ret = flash_erase(fd, start, length);
        if (ret < 0) {
                LOGE("erase error %d\n", ret);
                goto err;
        }
	memcpy(buf + (start - istart * SECTOR_SIZE), inbuf, length);

	lseek(fd, istart * ERASE_SIZE, SEEK_SET);
	write(fd, buf, ERASE_SIZE /*iend * ERASE_SIZE*/);

err:
	if(buf) free(buf);
	return ret;
}


E_RET_VALUE_LIST ip31_id_set(E_ID_LIST item, char *src, int size)
{
    int ret = -1;
    int write_offset = 0;
    int write_size = 0;
    char buf[1024] = {0};
	int istart;
	int iend;
        int ilength;

    get_device_id_prop(item, &write_offset, &write_size);

    memcpy(buf, src, size);
    //ret = param_write(write_offset, buf, size);//use size instead of write_size
    ret = flash_write(write_offset, size, buf);
    if (ret < 0)
    {
        LOGE("return error: %d\n", ret);
        return ret;
    }

    return E_OK;
}


E_RET_VALUE_LIST ip31_id_get(E_ID_LIST item, char *dest, int size)
{
    int ret = -1;
    int read_offset = 0;
    int read_size = 0;
    char buf[1024] = {0};

    get_device_id_prop(item, &read_offset, &read_size);
    ret = param_read(read_offset, buf, read_size);
    if (ret < 0) {
        LOGE("return error: %d\n", ret);
        return ret;
    }

    memcpy(dest, buf, read_size);

    return E_OK;
}
