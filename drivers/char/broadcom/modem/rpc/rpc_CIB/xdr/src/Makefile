#Makefile for xdr sources
EXTRA_CFLAGS += -Werror

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/ipc/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/public
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/sysinterface/util/public/

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/fuse_log/

EXTRA_CFLAGS += -Ifs/cifs/
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -DLINUX_RPC_KERNEL
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -D_RHEA_


obj-y += \
	xdr.o \
	xdr_array.o \
	xdr_mem.o \
	xdr_reference.o\
