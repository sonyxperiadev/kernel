#Makefile for RPC sources
EXTRA_CFLAGS += -Werror

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/ipc/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/public
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc
ccflags-y += -I$(srctree)/drivers/char/broadcom/modem/sysrpc/sysrpc-capi
ccflags-y += -I$(srctree)/drivers/char/broadcom/modem/sysrpc/sysrpc-capi/gen

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/sysinterface/util/public/

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/fuse_log/
EXTRA_CFLAGS += -I$(srctree)/include/linux/broadcom/

EXTRA_CFLAGS += -Ifs/cifs/
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -DLINUX_RPC_KERNEL
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -D_RHEA_

obj-$(CONFIG_BCM_MODEM) += \
	rpc_stubs.o \
	rpc_CIB/ \
	rpc_ipc_kernel.o	\
	mqueue.o	\
	rpc_wakelock.o	\
	rpc_debug.o	\
	rpc_client_msgs.o	\
	fuse_rpc_main.o
