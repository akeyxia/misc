#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Audio_7741.h"

#define XADDR   "/sys/devices/platform/imx-i2c.1/i2c-1/1-001c/xaddr"
#define YADDR   "/sys/devices/platform/imx-i2c.1/i2c-1/1-001c/yaddr"
#define EPADDR  "/sys/devices/platform/imx-i2c.1/i2c-1/1-001c/epaddr"
#define XDATA   "/sys/devices/platform/imx-i2c.1/i2c-1/1-001c/xdata"
#define YDATA   "/sys/devices/platform/imx-i2c.1/i2c-1/1-001c/ydata"
#define EPDATA  "/sys/devices/platform/imx-i2c.1/i2c-1/1-001c/ypdata"

/* Memory type (only used for direct read/write) */
typedef enum {
	I2CDRV_EP_MEM,          /*< Extended P-memory  */
	I2CDRV_X_MEM,           /*< X-memory */
	I2CDRV_Y_MEM            /*< Y-memory */
} I2CDrv_MemType_t;

int g_debug_mask = 0;

int saf7741_WriteDSPMem(I2CDrv_MemType_t memType, unsigned int address,
	unsigned char datalength, unsigned int *pData)
{
	unsigned int data = *pData;
/*
	char addr_buf[128] = {0};
	char data_buf[128] = {0};

	switch (memType) {
	case I2CDRV_EP_MEM:
		sprintf(addr_buf, "echo 0x%X > %s", ADSP_X_EasyP_Index, XADDR);
		sprintf(data_buf, "echo 0x%X > %s", data, XDATA);
		break;
	case I2CDRV_X_MEM:
                sprintf(addr_buf, "echo 0x%X > %s", address, XADDR);
                sprintf(data_buf, "echo 0x%X > %s", data, XDATA);
		break;
	case I2CDRV_Y_MEM:
                sprintf(addr_buf, "echo 0x%X > %s", address, YADDR);
                sprintf(data_buf, "echo 0x%X > %s", data, YDATA);
		break;
	default:		
		break;
		return -EINVAL;
	}

	printf("%s\n%s\n", addr_buf, data_buf);
	system(addr_buf);
	system(data_buf);
*/
	int fd_addr;
	int fd_data;
        char addr_buf[128] = {0};
        char data_buf[128] = {0};

	switch (memType) {
	case I2CDRV_EP_MEM:
		fd_addr = open(XADDR, O_RDWR);
		fd_data = open(XDATA, O_RDWR);
		sprintf(addr_buf, "0x%X", ADSP_X_EasyP_Index);
		sprintf(data_buf, "0x%X", data);
		break;
	case I2CDRV_X_MEM:
		fd_addr = open(XADDR, O_RDWR);
		fd_data = open(XDATA, O_RDWR);
		sprintf(addr_buf, "0x%X", address);
		sprintf(data_buf, "0x%X", data);
		break;
	case I2CDRV_Y_MEM:
		fd_addr = open(YADDR, O_RDWR);
		fd_data = open(YDATA, O_RDWR);
		sprintf(addr_buf, "0x%X", address);
		sprintf(data_buf, "0x%X", data);
		break;
	default:		
		break;
		return -EINVAL;
	}

	if (g_debug_mask) {
		printf("				[addr = %s, data = %s]\n", addr_buf, data_buf);
	}

	write(fd_addr, addr_buf, sizeof(addr_buf));
	write(fd_data, data_buf, sizeof(data_buf));

	return 0;
}

int saf7741_ReadDSPMem(I2CDrv_MemType_t memType, unsigned int address)
//	unsigned char datalength, unsigned int *pData)
{
	int fd_addr;
	int fd_data;
        char addr_buf[128] = {0};

	switch (memType) {
	case I2CDRV_EP_MEM:
		fd_addr = open(XADDR, O_RDWR);
		fd_data = open(XDATA, O_RDWR);
		sprintf(addr_buf, "0x%X", ADSP_X_EasyP_Index);
		break;
	case I2CDRV_X_MEM:
		fd_addr = open(XADDR, O_RDWR);
		fd_data = open(XDATA, O_RDWR);
		sprintf(addr_buf, "0x%X", address);
		break;
	case I2CDRV_Y_MEM:
		fd_addr = open(YADDR, O_RDWR);
		fd_data = open(YDATA, O_RDWR);
		sprintf(addr_buf, "0x%X", address);
		break;
	default:		
		break;
		return -EINVAL;
	}

	write(fd_addr, addr_buf, sizeof(addr_buf));
	read(fd_data, &data, 1);

	if (g_debug_mask) {
		printf("				[addr = %s, data = %s]\n", addr_buf, data);
	}

	return data;
}

int saf7741_WriteEasyP(I2CDrv_MemType_t memType, unsigned int index)
{
	unsigned int data = index;

	return saf7741_WriteDSPMem(I2CDRV_EP_MEM, ADSP_X_EasyP_Index, 1, &data);
}

