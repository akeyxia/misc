LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := mag3110_test.c
LOCAL_MODULE := mag3110
LOCAL_STATIC_LIBRARIES := libcutils libc libm
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

