LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -std=c99
LOCAL_MODULE    := v-ir
LOCAL_SRC_FILES := v-ir.c

include $(BUILD_EXECUTABLE)

