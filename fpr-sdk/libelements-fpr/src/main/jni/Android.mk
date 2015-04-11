# =========================================================================
# Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
# =========================================================================
# @file    Android.mk
#
###############################################################################
LOCAL_PATH:= $(call my-dir)

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

include $(BUILD_SHARED_LIBRARY)

