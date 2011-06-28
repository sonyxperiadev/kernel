BRCM_VIDEOCORE_ROOT = ../../../../hardware/broadcom/videocore
BRCM_VIDEOCORE_FIRMWARE_DIR  ?=  $(TOP)/hardware/broadcom/videocore/src


EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2 -Wno-declaration-after-statement -Wno-parentheses

EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/interface/vcos
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/interface/vcos/generic
EXTRA_CFLAGS  += -I${BRCM_VIDEOCORE_FIRMWARE_DIR}/interface/vcos/linuxkernel

EXTRA_CFLAGS  += -DOS_ASSERT_FAILURE
EXTRA_CFLAGS  += -D__STDC_VERSION=199901L
EXTRA_CFLAGS  += -D__STDC_VERSION__=199901L
EXTRA_CFLAGS  += -D__VCCOREVER__=0
EXTRA_CFLAGS  += -D__linux__
EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -DHAVE_VCOS_VERSION

obj-y += vcos-mod.o

$(info ++++++++++++++++++++++ $(BRCM_VIDEOCORE_ROOT))
$(info CONFIG_BCM_VCHIQ_VC04= $(CONFIG_BCM_VCHIQ_VC04))
$(info obj = $(obj))

vcos-mod-objs := \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/generic/vcos_generic_event_flags.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/generic/vcos_cmd.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/generic/vcos_logcat.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/generic/vcos_mem_from_malloc.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/linuxkernel/vcos_linuxkernel.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/linuxkernel/vcos_linuxkernel_cfg.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/linuxkernel/vcos_linuxkernel_misc.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/linuxkernel/vcos_mod_init.o \
    ../$(BRCM_VIDEOCORE_ROOT)/src/interface/vcos/linuxkernel/vcos_thread_map.o \
    vcos_version.o

genversion = $(BRCM_VIDEOCORE_FIRMWARE_DIR)/interface/vcos/vcos_genversion

GENVERSION_SRC_ROOT_DIR = $(obj)

$(obj)/vcos_version.c:  $(genversion) FORCE
	echo "Generating $@ ..."
	perl $(genversion) vcos $(GENVERSION_SRC_ROOT_DIR) > $@

all:    $(obj)/vcos_version.c
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules


install:
	@echo " Install dirs. $(INSTALL_MOD_DIR) $(INSTALL_MOD_PATH) $(KMODPATH)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order *~*
	-rm -rf .tmp_versions


