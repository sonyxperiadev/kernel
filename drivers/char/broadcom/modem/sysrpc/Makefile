#EXTRA_CFLAGS := -Werror

ifdef CONFIG_MFD_MAX8986
EXTRA_CFLAGS += -DPMU_MAX8986
else
EXTRA_CFLAGS += -DPMU_BCM59038
endif

include drivers/char/broadcom/modem/CAPI2_CIB/capi2_include_paths.gmk

EXTRA_CFLAGS += -I drivers/char/broadcom/modem/ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc/


EXTRA_CFLAGS += -D_BSDTYPES_DEFINED -DTYPEDEF_U_CHAR -DTYPEDEF_U_SHORT
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR 
EXTRA_CFLAGS += -DFUSE_DUAL_PROCESSOR_ARCHITECTURE
EXTRA_CFLAGS += -DCONFIG_BRCM_FUSE_RIL_CIB


EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX

obj-y += \
	sysrpc.o \
	bcm_kril_sysrpc_handler.o \
	sysrpc-capi/ \

