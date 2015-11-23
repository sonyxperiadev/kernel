LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#$(info "MHL MAKE")

LOCAL_MODULE       := mhl_sii8620_8061_drv.ko
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_PATH  := $(TARGET_OUT)/lib/modules
LOCAL_MODULE_CLASS := DLKM

MHL_BUILD_DIR := $(OUT)/mhl
MHL_LOCAL_DIR := $(LOCAL_PATH)
include $(BUILD_SYSTEM)/base_rules.mk

local_mhl_path     := $(LOCAL_PATH)

$(LOCAL_BUILT_MODULE): $(MHL_BUILD_DIR)/$(LOCAL_MODULE) | $(ACP)
	$(transform-prebuilt-to-target)

ifeq ($(SOMC_CFG_MHL_VERSION_MAJOR),)
CFG_MHL_VER_MAJOR = 0x30
else
CFG_MHL_VER_MAJOR = $(SOMC_CFG_MHL_VERSION_MAJOR)
endif

ifeq ($(SOMC_CFG_MHL_VERSION_MINOR),)
CFG_MHL_VER_MINOR = 00
else
CFG_MHL_VER_MINOR =  $(SOMC_CFG_MHL_VERSION_MINOR)
endif

ifneq ($(SOMC_CFG_MHL_SUPPORT_UNPOWERED_DONGLE), true)
CFG_MHL_SUPPORT_UNPOWERED_DONGLE = true
else
CFG_MHL_SUPPORT_UNPOWERED_DONGLE = false
endif

ifeq ($(SOMC_CFG_MHL_SUPPORT_CHG_TIMING), true)
CFG_MHL_SUPPORT_CHG_TIMING = true
else
CFG_MHL_SUPPORT_CHG_TIMING = false
endif

ifeq ($(SOMC_CFG_MHL_PRODUCT_SPECIFC_HW_CONFIG),)
#SIMG original value for Drive Strength
SOMC_CFG_MHL2_DS_DP_CTL1 = 0xFE
SOMC_CFG_MHL2_DS_DP_CTL6 = 0x2A
SOMC_CFG_MHL2_DS_DP_CTL7 = 0x08
SOMC_CFG_MHL2_DS_DP_CTL8 = 0x00
SOMC_CFG_MHL2_DS_COC_CTL1 = 0x07
SOMC_CFG_MHL3_DS_DP_CTL1 = 0xA2
SOMC_CFG_MHL3_DS_DP_CTL6 = 0x02
SOMC_CFG_MHL3_DS_DP_CTL7 = 0x08
SOMC_CFG_MHL3_DS_DP_CTL8 = 0x00
SOMC_CFG_MHL3_DS_COC_CTL1 = 0xBC
endif

CFG_MHL_DS_PARAM = "$(SOMC_CFG_MHL2_DS_DP_CTL1) \
			$(SOMC_CFG_MHL2_DS_DP_CTL6) \
			$(SOMC_CFG_MHL2_DS_DP_CTL7) \
			$(SOMC_CFG_MHL2_DS_DP_CTL8) \
			$(SOMC_CFG_MHL2_DS_COC_CTL1) \
			$(SOMC_CFG_MHL3_DS_DP_CTL1) \
			$(SOMC_CFG_MHL3_DS_DP_CTL6) \
			$(SOMC_CFG_MHL3_DS_DP_CTL7) \
			$(SOMC_CFG_MHL3_DS_DP_CTL8) \
			$(SOMC_CFG_MHL3_DS_COC_CTL1)"

ifeq ($(SOMC_CFG_MHL_DEVICE_ID_L),)
CFG_MHL_DEVICE_ID = 0x05
else
CFG_MHL_DEVICE_ID =  $(SOMC_CFG_MHL_DEVICE_ID_L)
endif

# y: Upstream HPD is push pull mode.
# n: Upstream HPD is open drain mode.
export SOMC_CFG_MHL_HPD_PUSH_PULL = y

# y: Activate upstream HPD is High.
# n: Activate upstream HPD is Low.
export SOMC_CFG_MHL_HPD_ACTIVE_HIGH = y

# Workaround for build error of mm command.
# KERNEL_CROSS_COMPILE and KERNEL_ARCH are empty when execute mm command.
# So, copied Define from kernel/AndroidKernel.mk to use mm command.
#
# WRANING!!!
# Check KERNEL_CROSS_COMPILE and KERNEL_ARCH value when platform is updated.
# This two value may be different on other platform.
ifeq ($(KERNEL_CROSS_COMPILE),)
KERNEL_CROSS_COMPILE := $(TARGET_KERNEL_CROSS_COMPILE_PREFIX)
endif

ifeq ($(KERNEL_ARCH),)
KERNEL_ARCH := $(TARGET_KERNEL_ARCH)
endif

# Workaround for failure of insmod of .ko made by mm command.
# KERNEL_CFLAGS is empty when execute mm command.
# So, copied Define from kernel/AndroidKernel.mk to fix this issue.
#
# WRANING!!!
# Check KERNEL_CFLAGS value when platform is updated.
# This value may be different on other platform.
ifeq ($(KERNEL_CFLAGS),)
KERNEL_CFLAGS := KCFLAGS=-mno-android
endif

$(MHL_BUILD_DIR)/$(LOCAL_MODULE): $(TARGET_PREBUILT_INT_KERNEL) $(MHL_LOCAL_DIR)
	mkdir -p $(MHL_BUILD_DIR)
	cp -f $(local_mhl_path)/Kbuild $(MHL_BUILD_DIR)
	cp -f $(local_mhl_path)/*.c $(MHL_BUILD_DIR)
	cp -f $(local_mhl_path)/*.h $(MHL_BUILD_DIR)
ifeq ($(filter %64,$(TARGET_KERNEL_ARCH)),)
	$(MAKE) -C kernel M=$(MHL_BUILD_DIR) O=$(OUT)/obj/KERNEL_OBJ PRODUCT_MHL_VER_MAJOR=$(CFG_MHL_VER_MAJOR) PRODUCT_MHL_VER_MINOR=$(CFG_MHL_VER_MINOR) FEATURE_SUPPORT_UNPOWERED_DONGLE=$(CFG_MHL_SUPPORT_UNPOWERED_DONGLE) MHL_SUPPORT_CHG_TIMING=$(CFG_MHL_SUPPORT_CHG_TIMING) ARCH=arm CROSS_COMPILE=$(ARM_EABI_TOOLCHAIN)/arm-eabi- CFG_MHL_BIST=$(SOMC_CFG_MHL_BIST) DS_PARAM=$(CFG_MHL_DS_PARAM) PRODUCT_MHL_DEVICE_ID=$(CFG_MHL_DEVICE_ID) modules
else
	$(MAKE) -C kernel M=$(MHL_BUILD_DIR) O=$(OUT)/obj/KERNEL_OBJ PRODUCT_MHL_VER_MAJOR=$(CFG_MHL_VER_MAJOR) PRODUCT_MHL_VER_MINOR=$(CFG_MHL_VER_MINOR) FEATURE_SUPPORT_UNPOWERED_DONGLE=$(CFG_MHL_SUPPORT_UNPOWERED_DONGLE) MHL_SUPPORT_CHG_TIMING=$(CFG_MHL_SUPPORT_CHG_TIMING) ARCH=$(KERNEL_ARCH) CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) CFG_MHL_BIST=$(SOMC_CFG_MHL_BIST) DS_PARAM=$(CFG_MHL_DS_PARAM) PRODUCT_MHL_DEVICE_ID=$(CFG_MHL_DEVICE_ID) $(KERNEL_CFLAGS) modules
endif
