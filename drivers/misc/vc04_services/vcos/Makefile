EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -Wno-declaration-after-statement -Wno-parentheses

EXTRA_CFLAGS  += -I"vc4-dev/src/"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/generic"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/linuxkernel"

EXTRA_CFLAGS  += -DOS_ASSERT_FAILURE
EXTRA_CFLAGS  += -D__STDC_VERSION=199901L
EXTRA_CFLAGS  += -D__STDC_VERSION__=199901L
EXTRA_CFLAGS  += -D__VCCOREVER__=0
EXTRA_CFLAGS  += -D__linux__
EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -DHAVE_VCOS_VERSION

EXTRA_CFLAGS  += -Werror

obj-m += vcos-mod.o

vcos-mod-objs := \
    ../../../../vc4-dev/src/interface/vcos/generic/vcos_generic_event_flags.o \
    ../../../../vc4-dev/src/interface/vcos/generic/vcos_cmd.o \
    ../../../../vc4-dev/src/interface/vcos/generic/vcos_logcat.o \
    ../../../../vc4-dev/src/interface/vcos/generic/vcos_mem_from_malloc.o \
    ../../../../vc4-dev/src/interface/vcos/linuxkernel/vcos_linuxkernel.o \
    ../../../../vc4-dev/src/interface/vcos/linuxkernel/vcos_linuxkernel_cfg.o \
    ../../../../vc4-dev/src/interface/vcos/linuxkernel/vcos_linuxkernel_misc.o \
    ../../../../vc4-dev/src/interface/vcos/linuxkernel/vcos_mod_init.o \
    ../../../../vc4-dev/src/interface/vcos/linuxkernel/vcos_thread_map.o \
    vcos_version.o

genversion = vc4-dev/src/interface/vcos/vcos_genversion

GENVERSION_SRC_ROOT_DIR = vc4-dev/src

$(obj)/vcos_version.c: $(genversion) FORCE
	echo "Generating $@ ..."
	perl $(srctree)/$(genversion) vcos $(GENVERSION_SRC_ROOT_DIR) > $@

