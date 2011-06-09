#Makefile for RPC sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I drivers/char/broadcom/modem/ipc/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/public
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc

EXTRA_CFLAGS += -I drivers/char/broadcom/modem/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/public/sysinterface/util/public/

EXTRA_CFLAGS += -Ifs/cifs/
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -DLINUX_RPC_KERNEL
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -D_RHEA_

#	fuse_rpc_main.o \
#	rpc_ipc_kernel.o


obj-$(CONFIG_BCM_MODEM) += \
	rpc_stubs.o \
	rpc_CIB/arpc/src/rpc_msg.o \
	rpc_CIB/arpc/src/rpc_lookup.o \
	rpc_CIB/arpc/src/rpc_api.o \
	rpc_CIB/arpc/src/rpc_ipc.o \
	rpc_CIB/arpc/src/rpc_sys.o \
	rpc_CIB/srpc/src/rpc_sync.o \
	rpc_CIB/xdr/src/xdr.o \
	rpc_CIB/xdr/src/xdr_array.o \
	rpc_CIB/xdr/src/xdr_mem.o \
	rpc_CIB/xdr/src/xdr_float.o \
	rpc_CIB/xdr/src/xdr_reference.o	\
	rpc_ipc_kernel.o	\
	fuse_rpc_main.o





