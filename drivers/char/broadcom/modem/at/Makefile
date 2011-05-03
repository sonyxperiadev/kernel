#Makefile for at kernel
#EXTRA_CFLAGS := -Werror

# for capi2_phonectrl_api.h
# common definitons for all capi2 related include paths
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_at/at_CIB/public
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/public/
include ../modules/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk

EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX

obj-$(CONFIG_BRCM_FUSE_AT) += atc_kernel.o
