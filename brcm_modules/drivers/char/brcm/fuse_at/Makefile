#Makefile for at kernel
EXTRA_CFLAGS := -Werror

ifdef CONFIG_BRCM_FUSE_RIL_2153SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc/inc/
else 
ifdef CONFIG_BRCM_FUSE_RIL_2157SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_2157/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_2157/inc/
else
ifdef CONFIG_BRCM_FUSE_RIL_CIB
# for capi2_phonectrl_api.h
# common definitons for all capi2 related include paths
include $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk
endif
endif
endif

EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX

obj-$(CONFIG_BRCM_FUSE_AT) += atc_kernel.o
