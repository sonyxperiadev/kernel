# Copyright (C) 2012 The CyanogenMod Project
# Copyright (C) 2015 Chirayu Desai
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Android makefile to build kernel as a part of Android Build

ifneq ($(BUILD_KERNEL),)
ifeq ($(filter-out amami aries castor eagle flamingo honami leo scorpion seagull sirius tianchi togari,$(TARGET_DEVICE)),)

KERNEL_SRC := $(call my-dir)
# kernel configuration - mandatory:
TARGET_KERNEL_CONFIG ?= $(notdir $(wildcard $(KERNEL_SRC)/arch/arm/configs/aosp_*_$(TARGET_DEVICE)_defconfig))
KERNEL_DEFCONFIG := $(TARGET_KERNEL_CONFIG)

ifeq ($(OUT_DIR),out)
KERNEL_OUT := $(ANDROID_BUILD_TOP)/$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
else
KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
endif
KERNEL_CONFIG := $(KERNEL_OUT)/.config

TARGET_KERNEL_ARCH := $(strip $(TARGET_KERNEL_ARCH))
ifeq ($(TARGET_KERNEL_ARCH),)
KERNEL_ARCH := $(TARGET_ARCH)
else
KERNEL_ARCH := $(TARGET_KERNEL_ARCH)
endif

TARGET_KERNEL_HEADER_ARCH := $(strip $(TARGET_KERNEL_HEADER_ARCH))
ifeq ($(TARGET_KERNEL_HEADER_ARCH),)
KERNEL_HEADER_ARCH := $(KERNEL_ARCH)
else
KERNEL_HEADER_ARCH := $(TARGET_KERNEL_HEADER_ARCH)
endif

KERNEL_HEADER_DEFCONFIG := $(strip $(KERNEL_HEADER_DEFCONFIG))
ifeq ($(KERNEL_HEADER_DEFCONFIG),)
KERNEL_HEADER_DEFCONFIG := $(KERNEL_DEFCONFIG)
endif

TARGET_PREBUILT_INT_KERNEL_TYPE := zImage

TARGET_PREBUILT_INT_KERNEL := $(KERNEL_OUT)/arch/$(KERNEL_ARCH)/boot/$(TARGET_PREBUILT_INT_KERNEL_TYPE)

# Clear this first to prevent accidental poisoning from env
MAKE_FLAGS :=

ifeq ($(KERNEL_ARCH),arm64)
  # Avoid "unsupported RELA relocation: 311" errors (R_AARCH64_ADR_GOT_PAGE)
  MAKE_FLAGS += CFLAGS_MODULE="-fno-pic"
  ifeq ($(TARGET_ARCH),arm)
    KERNEL_CONFIG_OVERRIDE := CONFIG_ANDROID_BINDER_IPC_32BIT=y
  endif
endif


KERNEL_BIN := $(TARGET_PREBUILT_INT_KERNEL)

KERNEL_HEADERS_INSTALL := $(KERNEL_OUT)/usr
KERNEL_MODULES_INSTALL := system
KERNEL_MODULES_OUT := $(TARGET_OUT)/lib/modules

TARGET_KERNEL_CROSS_COMPILE_PREFIX := $(strip $(TARGET_KERNEL_CROSS_COMPILE_PREFIX))
ifeq ($(TARGET_KERNEL_CROSS_COMPILE_PREFIX),)
ifeq ($(KERNEL_TOOLCHAIN_PREFIX),)
KERNEL_TOOLCHAIN_PREFIX := arm-eabi-
endif
else
KERNEL_TOOLCHAIN_PREFIX := $(TARGET_KERNEL_CROSS_COMPILE_PREFIX)
endif

ifeq ($(KERNEL_TOOLCHAIN),)
KERNEL_TOOLCHAIN_PATH := $(KERNEL_TOOLCHAIN_PREFIX)
else
ifneq ($(KERNEL_TOOLCHAIN_PREFIX),)
KERNEL_TOOLCHAIN_PATH := $(KERNEL_TOOLCHAIN)/$(KERNEL_TOOLCHAIN_PREFIX)
endif
endif

ifneq ($(USE_CCACHE),)
    ccache := $(ANDROID_BUILD_TOP)/prebuilts/misc/$(HOST_PREBUILT_TAG)/ccache/ccache
    # Check that the executable is here.
    ccache := $(strip $(wildcard $(ccache)))
endif

KERNEL_CROSS_COMPILE := CROSS_COMPILE="$(ccache) $(KERNEL_TOOLCHAIN_PATH)"
ccache =

define mv-modules
    mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.order`;\
    if [ "$$mdpath" != "" ];then\
        mpath=`dirname $$mdpath`;\
        ko=`find $$mpath/kernel -type f -name *.ko`;\
        for i in $$ko; do $(KERNEL_TOOLCHAIN_PATH)strip --strip-unneeded $$i;\
        mv $$i $(KERNEL_MODULES_OUT)/; done;\
    fi
endef

define clean-module-folder
    mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.order`;\
    if [ "$$mdpath" != "" ];then\
        mpath=`dirname $$mdpath`; rm -rf $$mpath;\
    fi
endef

ifeq ($(HOST_OS),darwin)
  MAKE_FLAGS += C_INCLUDE_PATH=$(ANDROID_BUILD_TOP)/external/elfutils/0.153/libelf/
endif

ifeq ($(TARGET_KERNEL_MODULES),)
    TARGET_KERNEL_MODULES := no-external-modules
endif

$(KERNEL_OUT):
	mkdir -p $(KERNEL_OUT)
	mkdir -p $(KERNEL_MODULES_OUT)

$(KERNEL_CONFIG): $(KERNEL_OUT)
	$(MAKE) $(MAKE_FLAGS) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) $(KERNEL_DEFCONFIG)
	$(hide) if [ ! -z "$(KERNEL_CONFIG_OVERRIDE)" ]; then \
			echo "Overriding kernel config with '$(KERNEL_CONFIG_OVERRIDE)'"; \
			echo $(KERNEL_CONFIG_OVERRIDE) >> $(KERNEL_OUT)/.config; \
			$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) oldconfig; fi

TARGET_KERNEL_BINARIES: $(KERNEL_OUT) $(KERNEL_CONFIG) $(KERNEL_HEADERS_INSTALL)
	$(MAKE) $(MAKE_FLAGS) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) $(TARGET_PREBUILT_INT_KERNEL_TYPE)
	-$(MAKE) $(MAKE_FLAGS) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) dtbs
	-$(MAKE) $(MAKE_FLAGS) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) modules
	-$(MAKE) $(MAKE_FLAGS) -C $(KERNEL_SRC) O=$(KERNEL_OUT) INSTALL_MOD_PATH=../../$(KERNEL_MODULES_INSTALL) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) modules_install
	$(mv-modules)
	$(clean-module-folder)

$(TARGET_KERNEL_MODULES): TARGET_KERNEL_BINARIES

$(TARGET_PREBUILT_INT_KERNEL): $(TARGET_KERNEL_MODULES)
	$(mv-modules)
	$(clean-module-folder)

$(KERNEL_HEADERS_INSTALL): $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(hide) if [ ! -z "$(KERNEL_HEADER_DEFCONFIG)" ]; then \
			rm -f ../$(KERNEL_CONFIG); \
			$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_HEADER_ARCH) $(KERNEL_CROSS_COMPILE) $(KERNEL_HEADER_DEFCONFIG); \
			$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_HEADER_ARCH) $(KERNEL_CROSS_COMPILE) headers_install; fi
	$(hide) if [ "$(KERNEL_HEADER_DEFCONFIG)" != "$(KERNEL_DEFCONFIG)" ]; then \
			echo "Used a different defconfig for header generation"; \
			rm -f ../$(KERNEL_CONFIG); \
			$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) $(KERNEL_DEFCONFIG); fi
	$(hide) if [ ! -z "$(KERNEL_CONFIG_OVERRIDE)" ]; then \
			echo "Overriding kernel config with '$(KERNEL_CONFIG_OVERRIDE)'"; \
			echo $(KERNEL_CONFIG_OVERRIDE) >> $(KERNEL_OUT)/.config; \
			$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) oldconfig; fi

kerneltags: $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) tags

kernelconfig: $(KERNEL_OUT) $(KERNEL_CONFIG)
	env KCONFIG_NOTIMESTAMP=true \
		 $(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) menuconfig
	env KCONFIG_NOTIMESTAMP=true \
		 $(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) savedefconfig
	cp $(KERNEL_OUT)/defconfig $(KERNEL_SRC)/arch/$(KERNEL_ARCH)/configs/$(KERNEL_DEFCONFIG)

## Install it
INSTALLED_KERNEL_TARGET ?= $(PRODUCT_OUT)/kernel

DTB_FILES := $(shell find -L $(KERNEL_OUT)/arch/arm/boot/ -name "*.dtb")
DTBS := $(foreach dtb,$(DTB_FILES),$(PRODUCT_OUT)/dtbs/$(notdir $(dtb)))
$(DTBS) : $(DTB_FILES) | $(ACP)
	$(hide) @mkdir -p $(PRODUCT_OUT)/dtbs
	$(transform-prebuilt-to-target)

file := $(INSTALLED_KERNEL_TARGET)
ALL_PREBUILT += $(file)
$(file) : $(KERNEL_BIN) $(DTBS) | $(ACP)
	$(transform-prebuilt-to-target)

ALL_PREBUILT += $(INSTALLED_KERNEL_TARGET)

endif # Sony AOSP devices
endif # BUILD_KERNEL
