LOCAL_PATH := $(call my-dir)

KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS=$(shell pwd)/$(call intermediates-dir-for,DLKM,cs40l25-module-symvers)/Module.symvers

include $(CLEAR_VARS)
LOCAL_MODULE := cirrus_cs40l2x_codec.ko
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
LOCAL_ADDITIONAL_DEPENDENCIES := $(KERNEL_MODULES_OUT)/cirrus_cs40l2x.ko
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/*) $(wildcard $(LOCAL_PATH)/**/*)

LOCAL_REQUIRED_MODULES    := cs40l25-module-symvers
LOCAL_ADDITIONAL_DEPENDENCIES := $(call intermediates-dir-for,DLKM,cs40l25-module-symvers)/Module.symvers

include $(TOP)/device/qcom/common/dlkm/Build_external_kernelmodule.mk
