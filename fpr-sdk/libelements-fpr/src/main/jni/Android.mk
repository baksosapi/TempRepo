# =========================================================================
# Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
# =========================================================================
# @file    Android.mk
#
###############################################################################
LOCAL_PATH:= $(call my-dir)
APP_ALLOW_MISSING_DEPS=true


include $(CLEAR_VARS)
LOCAL_MODULE := mmcamera_faceproc
LOCAL_SRC_FILES := ../prebuilt/libmmcamera_faceproc.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE:= libfacialproc_jni

LOCAL_SRC_FILES:= qcff.c\
        qcff_jni.c

LOCAL_SHARED_LIBRARIES := libutils libmmcamera_faceproc

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc/

LOCAL_CFLAGS += -Wno-multichar

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:= false

APP_ABI := armeabi

ifeq ($(APP_ABI),$(filter $(APP_ABI),armeabi armeabi-v7a))
  ARCH_ABI := arm
else ifeq ($(APP_ABI),$(filter $(APP_ABI),x86))
  ARCH_ABI := x86
else ifeq ($(APP_ABI),$(filter $(APP_ABI),arm64-v8a))
  ARCH_ABI := arm64
else ifeq ($(APP_ABI),$(filter $(APP_ABI),x86_64))
  ARCH_ABI := x86_64
else
  $(error Unsuported / Unknown APP_API '$(APP_ABI)')
endif

#ARCH_ABI := arm
$(info $(LOCAL_PATH))
$(warning $(APP_ABI))
$(warning ARCH_ABI: $(ARCH_ABI))

include $(BUILD_SHARED_LIBRARY)

