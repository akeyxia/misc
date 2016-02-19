#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libparam_ip31.h"


#define LOGE(fmt,args...)     printf (fmt ,##args)
#define LOGD(fmt,args...)     printf (fmt ,##args)

#define DEVICEID_SIZE 19
#define BTADDR_SIZE 12
#define WIFIADDR_SIZE 12



int print_menu()
{
    LOGD("%d: write E_Recovery_Wipe_Data\n", E_Recovery_Wipe_Data);
    LOGD("%d: write E_Recovery_Upgrade_Mpu\n", E_Recovery_Upgrade_Mpu);
    LOGD("%d: write E_Recovery_Upgrade_Mcu\n", E_Recovery_Upgrade_Mcu);
    LOGD("%d: write E_Recovery_Upgrade_Map\n", E_Recovery_Upgrade_Map);
    LOGD("%d: write E_Recovery_Upgrade_RL78\n", E_Recovery_Upgrade_RL78);
    LOGD("%d: write E_Recovery_Upgrade_TSFW\n", E_Recovery_Upgrade_TSFW);
    LOGD("%d: write E_Recovery_Upgrade_GPSFW\n", E_Recovery_Upgrade_GPSFW);
    LOGD("%d: write E_Boot_Fastboot\n", E_Boot_Fastboot);
    LOGD("%d: E_RESET_TO_DEFAULE\n", E_Reset_To_Default);
    return 0;
}
void help() {
    LOGD("USAGE:\n");
    LOGD("  --write-deviceID IP31-ZXQ-A1-H-N0001\n");
    LOGD("  --write-btaddr 1A2B3C4D5E6F\n");
    LOGD("  --write-wifiaddr 1A2B3C4D5E6F\n");
    LOGD("  --read-deviceID\n");
    LOGD("  --read-btaddr\n");
    LOGD("  --read-wifiaddr\n");
}
int check_addr(char *addr, int size)
{
    int i = 0;
    char addr_lib[] = "0123456789abcdefgABCDEFG";
    char p[64] = {0};

 //   LOGD("size:%d\n", size);
 //   LOGD("addr:%s\n", addr);
    for(i = 0; i < 12; i++){
			sprintf(p, "%s",addr+i);
			p[1] = '\0';
         if(!strstr(addr_lib, p)){
            LOGE("%c is in address, please check again\n", *p);
            return -1;
         }
    }
}

int main(int argc, char *argv[])
{
    int menu;
    int ret;
    char buf[256] = {0};


    ret = ip31_param_init();
    if (ret != E_OK)
    {
        LOGE("ip31_boot_param_init error:%d\n", ret);
    }

    if(argc < 2) {
        help();
        return 0;
    }



    if(strcmp(argv[1], "--help") == 0) {
        help();
    } else if(strcmp(argv[1], "--write-deviceID") == 0) {
        ip31_id_set(E_DEVICE_ID, argv[2] , strlen(argv[2]));
    } else if(strcmp(argv[1], "--write-btaddr") == 0) {
        check_addr(argv[2], strlen(argv[2]));
        ip31_id_set(E_BT_ADDR, argv[2] , BTADDR_SIZE);
    } else if(strcmp(argv[1], "--write-wifiaddr") == 0) {
        check_addr(argv[2], strlen(argv[2]));
        ip31_id_set(E_WIFI_ADDR, argv[2] , WIFIADDR_SIZE);
    } else if(strcmp(argv[1], "--read-deviceID") == 0) {
        char buf3[64] = {0};
        ip31_id_get(E_DEVICE_ID, buf3, 32);
        LOGD("DeviceID: %s\n", buf3 );
    } else if(strcmp(argv[1], "--read-btaddr") == 0) {
        char buf3[32] = {0};
        ip31_id_get(E_BT_ADDR, buf3, 32);
        LOGD("BT addr: %s\n", buf3 );
    } else if(strcmp(argv[1], "--read-wifiaddr") == 0) {
        char buf3[32] = {0};
        ip31_id_get(E_WIFI_ADDR, buf3, 32);
        LOGD("WIFI addr: %s\n", buf3 );
    }else{
		help();	
	}

    ret = ip31_param_uninit();
    if (ret != E_OK)
    {
        LOGE("ip31_boot_param_uninit error:%d\n", ret);
    }

    return 0;
}
