#Makefile for RPC sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/public
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc
#EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/public/

EXTRA_CFLAGS += -Ifs/cifs/
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED

obj-y += \
	fuse_rpc_main.o \
	rpc_stubs.o \
    rpc_CIB/arpc/src/ \
    rpc_CIB/srpc/src/ \
    rpc_CIB/xdr/src/  

