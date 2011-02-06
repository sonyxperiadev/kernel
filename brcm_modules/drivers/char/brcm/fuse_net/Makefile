#Makefile for KRIL & CAPI2
#EXTRA_CFLAGS := -Werror

ifdef CONFIG_BRCM_FUSE_IPC_2153SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc/Implementation/Headers/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc/inc/
else 
ifdef CONFIG_BRCM_FUSE_IPC_2157SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_2157/Implementation/Headers/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_2157/inc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_2157/inc/
else
ifdef CONFIG_BRCM_FUSE_RPC_CIB
# common definitons for all capi2 related include paths
include $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/arpc/inc/
#EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_rpc/rpc_CIB/srpc/inc/

# used to prevent redefinition of U_CHAR in various CIB header files
EXTRA_CFLAGS += -D_BSDTYPES_DEFINED -DTYPEDEF_U_CHAR -DTYPEDEF_U_SHORT
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR -DFUSE_DUAL_PROCESSOR_ARCHITECTURE
endif
endif
endif

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED

obj-$(CONFIG_BRCM_FUSE_NET) += \
	bcm_fuse_net.o

