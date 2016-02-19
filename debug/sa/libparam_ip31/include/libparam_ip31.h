#ifndef __LIBPARAM_IP31_H__
#define __LIBPARAM_IP31_H__

#ifdef __cplusplus
extern "C" {
#endif

//boot-param set/get item list
typedef enum {
    E_Recovery_Wipe_Data,       //will be clean /data and /cache
    E_Recovery_Upgrade_Mpu,     //will be Upgrade all system
    E_Recovery_Upgrade_Mcu,     //will be Upgrade V850
    E_Recovery_Upgrade_Map,     //will be Upgrade /Map part
    E_Recovery_Upgrade_RL78,    //will be Upgrade RL78
    E_Recovery_Upgrade_TSFW,    //will be Upgrade touch firmware
    E_Recovery_Upgrade_GPSFW,   //will be Upgrade gps firmware
    E_Boot_Fastboot,            //boot into uboot fastboot mode
    E_Reset_To_Default,         //reset all param to default
    E_Boot_MAX,                 //MAX num
}
E_BOOT_PARAM_LIST;

typedef enum {
    E_DEVICE_ID,
    E_BT_ADDR,     //bt addr
    E_WIFI_ADDR,    //wifi addr
    E_ID_MAX,    //MAX num
}
E_ID_LIST;

//set/get param fun-list return value check list
typedef enum
{
    E_OK,       //return normal
    E_OPEN,     //open file error
    E_CLOSE,    //close file error
    E_RESET,    //reset param error
    E_NULL,     //param is null error
    E_ITEM,     //param item is not in list error
    E_READ,     //read param error
    E_WRITE,    //write param error
    E_LSEEK,    //seek error while writing
    E_Err_MAX,  //MAX error num
} E_RET_VALUE_LIST;


/*
 * init before read/write param
 * @iparam null
 * @ioparam null
 * @oparam null
 * @return E_OK if the func succeeded, fail otherwise.
 * @see null
 */
E_RET_VALUE_LIST ip31_param_init();


/*
 * uninit after read/write param
 * @iparam null
 * @ioparam null
 * @oparam null
 * @return E_OK if the func succeeded, fail otherwise.
 * @see null
 */
E_RET_VALUE_LIST ip31_param_uninit();


/*
 * reset all param value
 * @iparam null
 * @ioparam null
 * @oparam null
 * @return E_OK if the func succeeded, fail otherwise.
 * @see null
 */
E_RET_VALUE_LIST ip31_boot_param_reset();


/*
 * get param string value
 * @iparam E_PARAM
 * @ioparam null
 * @oparam char *dest:a string
 * @return E_OK if the func succeeded, fail otherwise.
 * @dest must be malloc before call this interface
 * @see null
 */
E_RET_VALUE_LIST ip31_boot_param_get(E_BOOT_PARAM_LIST item, char *dest);

/*
 * set param string value
 * @iparam E_PARAM
 * @ioparam null
 * @oparam null
 * @return E_OK if the func succeeded, fail otherwise.
 * @see null
 */
E_RET_VALUE_LIST ip31_boot_param_set(E_BOOT_PARAM_LIST item);

/*
 * set id string value to param part
 * @iparam E_ID_LIST
 * @iparam char *
 * @iparam int
 * @ioparam null
 * @oparam null
 * @return E_OK if the func succeeded, fail otherwise.
 * @
 * @see null
 */
E_RET_VALUE_LIST ip31_id_set(E_ID_LIST item, char *src, int size);

/*
 * get id string value to param part
 * @iparam E_ID_LIST
 * @iparam char *
 * @iparam int
 * @ioparam null
 * @oparam null
 * @return E_OK if the func succeeded, fail otherwise.
 * @dest must be malloc before call this interface
 * @see null
 */
E_RET_VALUE_LIST ip31_id_get(E_ID_LIST item, char *dest, int size);

int test_get_all();

#ifdef __cplusplus
}
#endif

#endif /* __LIBPARAM_IP31_H__ */
