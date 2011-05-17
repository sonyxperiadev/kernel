#Makefile for xdr sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc/
#EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/sysinterface/util/public


EXTRA_CFLAGS += -DUNDER_LINUX

obj-y += \
	xdr.o \
	xdr_array.o \
	xdr_mem.o \
	xdr_float.o \
	xdr_reference.o\
