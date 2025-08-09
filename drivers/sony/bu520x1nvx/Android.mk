LOCAL_PATH := $(call my-dir)

KBUILD_OPTIONS := HALL_SENSOR_ROOT=$(shell pwd)/$(LOCAL_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE              := bu520x1nvx.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
ifeq ($(TARGET_BOARD_PLATFORM), lahaina)
	include $(DLKM_DIR)/AndroidKernelModule.mk
else
	include $(DLKM_DIR)/Build_external_kernelmodule.mk
endif
