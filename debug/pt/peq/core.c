#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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

int saf7741_WriteDSPMem(I2CDrv_MemType_t memType, unsigned int address, unsigned int data)
{
	char addr_buf[128] = {0};
	char data_buf[128] = {0};

	switch (memType) {
	case I2CDRV_EP_MEM:
		sprintf(addr_buf, "\"echo 0x%X > %s\"", ADSP_X_EasyP_Index, XADDR );
		sprintf(data_buf, "\"echo 0x%X > %s\"", data, XDATA);
		break;
	case I2CDRV_X_MEM:
                sprintf(addr_buf, "\"echo 0x%X > %s\"", address, XADDR);
                sprintf(data_buf, "\"echo 0x%X > %s\"", data, XDATA);
		break;
	case I2CDRV_Y_MEM:
                sprintf(addr_buf, "\"echo 0x%X > %s\"", address, YADDR );
                sprintf(data_buf, "\"echo 0x%X > %s\"", data, YDATA);
		break;
	default:		
		break;
		return -EINVAL;
	}

	printf("%s\n%s\n", addr_buf, data_buf);
	system(addr_buf);
	system(data_buf);

	return 0;
}

