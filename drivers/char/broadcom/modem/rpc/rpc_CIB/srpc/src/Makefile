#Makefile for IPC sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/xdr/inc/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/public/


EXTRA_CFLAGS += -DUNDER_LINUX -DFUSE_APPS_PROCESSOR 

obj-y += \
	rpc_sync.o \
