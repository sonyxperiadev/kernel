#Makefile for VSP sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/ipc
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public
EXTRA_CFLAGS += -I$(srctree)/include/linux/broadcom/

# used to prevent redefinition of U_CHAR in various CIB header files
EXTRA_CFLAGS += -D_BSDTYPES_DEFINED -DTYPEDEF_U_CHAR -DTYPEDEF_U_SHORT
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR -DFUSE_DUAL_PROCESSOR_ARCHITECTURE

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED
#EXTRA_CFLAGS += -DCSD_LOOPBACK_TEST

EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -DLINUX_RPC_KERNEL

obj-$(CONFIG_BRCM_FUSE_VSP) += \
          fuse_vsp.o


