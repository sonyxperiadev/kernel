EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -DPMU_BCM59055

include ./drivers/char/broadcom/modem/public/msp_include_paths.gmk

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/sysrpc/sysrpc-capi/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/sysrpc/sysrpc-capi/gen/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/sysrpc/sysrpc-capi/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/sysrpc/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/fuse_log/


EXTRA_CFLAGS += -D_BSDTYPES_DEFINED -DTYPEDEF_U_CHAR -DTYPEDEF_U_SHORT
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR 
EXTRA_CFLAGS += -DFUSE_DUAL_PROCESSOR_ARCHITECTURE
EXTRA_CFLAGS += -DCONFIG_BRCM_FUSE_RIL_CIB


EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DLINUX_RPC_KERNEL

obj-y += sys_gen_rpc.o sys_hal_rpc.o sys_msg.o sys_rpc.o

