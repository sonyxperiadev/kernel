#Makefile for IPC sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/arpc/inc/
#EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/srpc/inc/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/soc/debug/public/


EXTRA_CFLAGS += -DUNDER_LINUX

obj-y += \
	rpc_msg.o \
	rpc_lookup.o \
	rpc_api.o \
	rpc_ipc.o \
	rpc_sys.o \
