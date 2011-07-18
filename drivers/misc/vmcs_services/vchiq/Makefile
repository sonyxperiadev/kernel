BRCM_VIDEOCORE_ROOT = ../../../../../hardware/broadcom/videocore
BRCM_VIDEOCORE_FIRMWARE_DIR  ?=  $(TOP)/hardware/broadcom/videocore/src


VCHIQ_FLAVOUR := vchiq_arm_bi
VCHIQ_TEST ?= 0

EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2 -Wno-declaration-after-statement -Wno-parentheses

EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/interface/$(VCHIQ_FLAVOUR)
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/interface/vcos
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/interface/vcos/generic
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/interface/vcos/linuxkernel
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/host_support/linux/added/include

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

obj-y += vc-vchiq.o

ifeq ($(VCHIQ_FLAVOUR),vchiq_arm)

vc-vchiq-objs := \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm/vchiq_arm.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm/vchiq_core.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm/vchiq_kern_lib.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm/vchiq_shim.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm/vchiq_util.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm/vchiq_bi_arm.o

else

vc-vchiq-objs := \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm_bi/big_island/vchiq_arm.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm_bi/vchiq_core.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm_bi/vchiq_memdrv.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm_bi/vchiq_shim.o \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm_bi/vchiq_util.o \

endif

ifeq ($(VCHIQ_TEST),1)
vc-vchiq-objs += vchiq_test.o
endif

obj-y += vchiq-memdrv-hana.o

vchiq-memdrv-hana-objs := \
    $(BRCM_VIDEOCORE_ROOT)/src/interface/vchiq_arm_bi/vchiq_memdrv_hana.o



