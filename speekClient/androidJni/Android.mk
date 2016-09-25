LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE    := netCtrl
LOCAL_CFLAGS := -I./include/ -lpthread
LOCAL_SRC_FILES := network.c
LOCAL_SHARED_LIBRARIES  := liblog  
LOCAL_MODULE_TAGS:= debug
LOCAL_LDLIBS += -llog 
include $(BUILD_SHARED_LIBRARY)

