LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_BOARD_PLATFORM), lahaina)
KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS=$(shell pwd)/$(call intermediates-dir-for,DLKM,drm-module-symvers)/Module.symvers
KBUILD_OPTIONS += TOUCH_DRIVER_SOD=$(shell pwd)/$(LOCAL_PATH)
endif

include $(CLEAR_VARS)
LOCAL_MODULE              := sec_touchscreen.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
LOCAL_SRC_FILES           := \
    $(LOCAL_PATH)/sec_ts.c \
    $(LOCAL_PATH)/sec_ts_fn.c \
    $(LOCAL_PATH)/sec_ts_fw.c \
    $(LOCAL_PATH)/sec_ts_only_vendor.c \
    $(LOCAL_PATH)/sec_cmd.c \

ifneq ($(TARGET_BOARD_PLATFORM), lahaina)
LOCAL_REQUIRED_MODULES    := drm-module-symvers
LOCAL_ADDITIONAL_DEPENDENCIES := $(call intermediates-dir-for,DLKM,drm-module-symvers)/Module.symvers
endif

ifeq ($(TARGET_BOARD_PLATFORM), lahaina)
include $(DLKM_DIR)/AndroidKernelModule.mk
else
include $(DLKM_DIR)/Build_external_kernelmodule.mk
endif