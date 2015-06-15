LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := core.c test_peq.c
LOCAL_MODULE := test_peq
LOCAL_STATIC_LIBRARIES := libcutils libc
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

