EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -Wno-declaration-after-statement -Wno-parentheses

EXTRA_CFLAGS  += -I"drivers/videocore"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vcos"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vcos/generic"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vcos/linuxkernel"

EXTRA_CFLAGS  += -DOS_ASSERT_FAILURE
EXTRA_CFLAGS  += -D__STDC_VERSION=199901L
EXTRA_CFLAGS  += -D__STDC_VERSION__=199901L
EXTRA_CFLAGS  += -D__VCCOREVER__=0
EXTRA_CFLAGS  += -D__linux__
EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -DHAVE_VCOS_VERSION
EXTRA_CFLAGS  += -Werror

obj-$(CONFIG_VC_VCOS) += vc-vcos.o

vc-vcos-objs := \
    ../generic/vcos_generic_event_flags.o \
    ../generic/vcos_cmd.o \
    ../generic/vcos_logcat.o \
    ../generic/vcos_mem_from_malloc.o \
    vcos_linuxkernel.o \
    vcos_linuxkernel_cfg.o \
    vcos_linuxkernel_misc.o \
    vcos_mod_init.o \
    vcos_thread_map.o \
    vcos_version.o

genversion = drivers/videocore/interface/vcos/vcos_genversion

GENVERSION_SRC_ROOT_DIR = .

$(obj)/vcos_version.c: $(genversion) FORCE
	echo "(new) Generating $@ ..."
	perl $(srctree)/$(genversion) vcos $(GENVERSION_SRC_ROOT_DIR) > $@

clean-files := vcos_version.c
