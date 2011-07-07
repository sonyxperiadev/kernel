# String based config options have quotes around them, which we don't want.
EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2 -Wno-declaration-after-statement -Wno-parentheses

EXTRA_CFLAGS  += -I"vc4-dev/src/"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vchiq_arm_bi"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/generic"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/linuxkernel"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vceb/host/linux"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vceb/host/common"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vceb/common"

#EXTRA_CFLAGS  += -I$(BRCM_VIDEOCORE_VCEB_DIR)
#EXTRA_CFLAGS  += -I$(BRCM_VIDEOCORE_VCEB_DIR)/..
#EXTRA_CFLAGS  += -I$(BRCM_VIDEOCORE_VCEB_DIR)/../common
#EXTRA_CFLAGS  += -I$(BRCM_VIDEOCORE_VCEB_DIR)/../../common

ifneq ($(CONFIG_VC_VCHIQ_BUSDRV_SHAREDMEM),)
VCHIQ_HOST_BUS_ALIGNMENT = 7
else
VCHIQ_HOST_BUS_ALIGNMENT = 3
endif

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

obj-$(CONFIG_VC_VCHIQ) += vc-vchiq.o

vc-vchiq-objs := \
    ../../../../vc4-dev/src/interface/vchiq_arm_bi/big_island/vchiq_arm.o \
    ../../../../vc4-dev/src/interface/vchiq_arm_bi/vchiq_core.o \
    ../../../../vc4-dev/src/interface/vchiq_arm_bi/vchiq_memdrv.o \
    ../../../../vc4-dev/src/interface/vchiq_arm_bi/vchiq_shim.o \
    ../../../../vc4-dev/src/interface/vchiq_arm_bi/vchiq_util.o

obj-$(CONFIG_VC_VCHIQ_MEMDRV_HANA) += vchiq-memdrv-hana.o

vchiq-memdrv-hana-objs := \
    ../../../../vc4-dev/src/interface/vchiq_arm_bi/vchiq_memdrv_hana.o

