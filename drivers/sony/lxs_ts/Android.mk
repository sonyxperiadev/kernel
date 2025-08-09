LOCAL_PATH := $(call my-dir)

KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS=$(shell pwd)/$(call intermediates-dir-for,DLKM,drm-module-symvers)/Module.symvers

include $(CLEAR_VARS)
LOCAL_MODULE              := lxs_touchscreen.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
LOCAL_SRC_FILES           := \
    $(LOCAL_PATH)/lxs_ts.c \
    $(LOCAL_PATH)/lxs_ts_fn.c \
    $(LOCAL_PATH)/lxs_ts_hal.c \
    $(LOCAL_PATH)/lxs_ts_hal_fw.c \
    $(LOCAL_PATH)/lxs_ts_hal_prd.c \
    $(LOCAL_PATH)/lxs_ts_sysfs.c \
    $(LOCAL_PATH)/touch_sw82907.c \

LOCAL_REQUIRED_MODULES    := drm-module-symvers
LOCAL_ADDITIONAL_DEPENDENCIES := $(call intermediates-dir-for,DLKM,drm-module-symvers)/Module.symvers

include $(DLKM_DIR)/Build_external_kernelmodule.mk