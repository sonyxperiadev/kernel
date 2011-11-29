#
#
# Makefile for building linux vchiq module.
#
# This makefile tries to accomodate the 2 different build systems.
# CONFIG_ARCH_BCMHANA is used for linux-d
# CONFIG_ARCH_KONA is used for the Big Red Tablet and MAP build system
# Everything else is assumed to be for the Cambridge build system.
#

ifeq ($(CONFIG_ARCH_KONA),y)

include $(srctree)/drivers/videocore/interface/vchiq_arm/shared-config.mk

##############################################################################
#
# This section is for building using the MAP build system
#
##############################################################################

$(info Building for MAP build system)
$(info CONFIG_ARCH_KONA    = $(CONFIG_ARCH_KONA))

EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -Wno-declaration-after-statement -Wno-parentheses

EXTRA_CFLAGS  += -I"drivers/videocore/"
EXTRA_CFLAGS  += -I"drivers/videocore/host_support/include"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vchiq_arm"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vcos"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vcos/generic"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vcos/linuxkernel"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vceb/host/linux"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vceb/host/common"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vceb/common"

EXTRA_CFLAGS  += -Werror

VCHIQ_HOST_BUS_ALIGNMENT = 3

EXTRA_CFLAGS  += -DOS_ASSERT_FAILURE
EXTRA_CFLAGS  += -D__STDC_VERSION=199901L
EXTRA_CFLAGS  += -D__STDC_VERSION__=199901L
EXTRA_CFLAGS  += -D__VCCOREVER__=0
EXTRA_CFLAGS  += -D__linux__
EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -DVCHIQ_HOST_BUS_ALIGNMENT=$(VCHIQ_HOST_BUS_ALIGNMENT)
EXTRA_CFLAGS  += -DVCHIQ_SLOT_PADDING=16
EXTRA_CFLAGS  += -DVCHIQ_ARM_SIDE
EXTRA_CFLAGS  += -DUSE_VCHIQ_ARM
ifeq ($(VCHIQ_SM_ALLOC),vcddr)
EXTRA_CFLAGS  += -DVCHIQ_SM_ALLOC_VCDDR
endif
obj-$(CONFIG_VC_VCHIQ) += vc-vchiq.o

vc-vchiq-objs := \
    vchiq_arm.o \
    vchiq_connected.o \
    vchiq_core.o \
    vchiq_kern_lib.o \
    vchiq_shim.o \
    vchiq_util.o \
    vchiq_$(VCHIQ_PLATFORM)_arm.o
ifeq ($(VCHIQ_SM_ALLOC),vcddr)
vc-vchiq-objs += \
    ../../host_applications/linux/libs/debug_sym/debug_sym.o
endif

obj-$(CONFIG_VC_VCHIQ_MEMDRV_KONA) += vc-vchiq-memdrv-kona.o

vc-vchiq-memdrv-kona-objs := \
    vchiq_memdrv_kona.o

else

##############################################################################
#
# This section is for building using the Cambridge legacy? system
# (android for 2835)
#
##############################################################################

#
# Makefile to build linux vchiq module
#
# Use V=1 to get debug output

# location of arm linux kernel build tree
KERNEL_BUILD ?= ../../../arm_linux/linux/build

# destination directory (top level) for install
INSTALL_MOD_PATH ?= $(HOME)/targetfs/root/files

# toolchain
CROSS_COMPILE ?= bcm2708-

# build directory (seems to be ignored)
O ?= $(KERNEL_BUILD)

pwd=$(shell pwd)
interface=$(shell dirname $(pwd))
VC4 ?= $(shell dirname $(interface))

ARCH=arm

EXTRA_CFLAGS += -DVCOS_VERIFY_BKPTS=1 -I. -I$(VC4)
EXTRA_CFLAGS += -I$(VC4)/interface/vcos/linuxkernel -Ilinuxkernel
EXTRA_CFLAGS += -I$(VC4)/interface/vceb/common
EXTRA_CFLAGS += -I$(VC4)/interface/vceb/host/common
EXTRA_CFLAGS += -I$(VC4)/interface/vceb/host/linux

obj-m := vchiq.o
vchiq-objs := vchiq_core.o vchiq_arm.o vchiq_kern_lib.o
vchiq-objs += ../vcos/linuxkernel/vcos_linuxkernel.o
vchiq-objs += ../vcos/linuxkernel/vcos_thread_map.o
vchiq-objs += ../vcos/linuxkernel/vcos_linuxkernel_cfg.o
vchiq-objs += ../vcos/generic/vcos_generic_event_flags.o
vchiq-objs += ../vcos/generic/vcos_logcat.o
vchiq-objs += ../vcos/generic/vcos_mem_from_malloc.o
vchiq-objs += ../vcos/generic/vcos_cmd.o
ifeq ($(CONFIG_MACH_ISLAND_FF)$(CONFIG_MACH_ISLAND_BU)$(CONFIG_MACH_ISLAND_FPGA), y)
   vchiq-objs += vchiq_bi_arm.o
else
   vchiq-objs += vchiq_2835_arm.o
endif

all:
	mkdir -p $(O)
	$(MAKE) -C $(KERNEL_BUILD)  M=$(pwd) modules\
		ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
		VC4=$(VC4) \
		O=$(O)

install:
	$(MAKE) -C $(KERNEL_BUILD) M=$(pwd) modules_install\
		ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
		INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) \
		VC4=$(VC4) \
		O=$(O)

clean:
	$(MAKE) -C $(KERNEL_BUILD) M=$(pwd) clean\
		ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
		VC4=$(VC4) \
		O=$(O)

endif
