#Makefile for RPC sources
#EXTRA_CFLAGS := -Werror

ifdef CONFIG_BRCM_FUSE_RIL_2153SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc/inc/
else
ifdef CONFIG_BRCM_FUSE_RPC_2157SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_2157/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_2157/inc/
else
ifdef CONFIG_BRCM_FUSE_RPC_CIB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/public
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/arpc/inc
#EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/srpc/inc

# common definitons for all capi2 related include paths
include $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk
endif
endif
endif

EXTRA_CFLAGS += -Ifs/cifs/
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED

obj-$(CONFIG_BRCM_FUSE_RIL_2153SDB) += \
          fuse_rpc_main.o \
          rpc/src/

obj-$(CONFIG_BRCM_FUSE_RIL_2157SDB) += \
          fuse_rpc_main.o \
          rpc_2157/src/

obj-$(CONFIG_BRCM_FUSE_RPC_CIB) += \
	fuse_rpc_main.o \
	rpc_stubs.o \
    rpc_CIB/arpc/src/ \
    rpc_CIB/srpc/src/ \
    rpc_CIB/xdr/src/  

