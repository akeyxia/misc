LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := gyroap.c
LOCAL_MODULE := gyroap
LOCAL_STATIC_LIBRARIES := libcutils libc
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

