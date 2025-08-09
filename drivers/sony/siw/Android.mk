LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(SOMC_CFG_TOUCHSCREEN_SIW_SW49501),yes)
LOCAL_MODULE              := ssw49501.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/AndroidKernelModule.mk
endif
