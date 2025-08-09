LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

DLKM_DIR   := $(TOP)/device/qcom/common/dlkm
LOCAL_MODULE              := somc_battman_dbg.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
