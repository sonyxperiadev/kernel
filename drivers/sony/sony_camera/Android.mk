# Kbuild options
KBUILD_OPTIONS := SONY_CAMERA_KERNEL_ROOT=$(shell pwd)/$(LOCAL_PATH)
KBUILD_OPTIONS += KERNEL_ROOT=$(shell pwd)/kernel/msm-$(TARGET_KERNEL_VERSION)/
KBUILD_OPTIONS += CAMERA_KERNEL_ROOT=$(ROOT_DIR)/../vendor/qcom/opensource/camera-kernel
KBUILD_OPTIONS += MODNAME=sony_camera

KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += PRODUCT=$(TARGET_PRODUCT)
KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS=$(shell pwd)/$(call intermediates-dir-for,DLKM,camera-module-symvers)/Module.symvers

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES         := $(LOCAL_PATH)/sony_camera.c
LOCAL_MODULE            := sony_camera.ko
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_PATH       := $(KERNEL_MODULES_OUT)
LOCAL_REQUIRED_MODULES        := camera-module-symvers
LOCAL_ADDITIONAL_DEPENDENCIES := $(call intermediates-dir-for,DLKM,camera-module-symvers)/Module.symvers
include $(DLKM_DIR)/Build_external_kernelmodule.mk
