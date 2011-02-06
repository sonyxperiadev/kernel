#Makefile for RPC sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ril/CAPI2_2157/inc/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_2157/inc/
EXTRA_CFLAGS += -Ifs/cifs/

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED

obj-$(CONFIG_BRCM_FUSE_RPC_2157SDB) += \
          rpc_msg.o \
          rpc_lookup.o \
		  rpc_api.o \
		  rpc_ipc.o \
		  rpc_sys.o \
          xdr.o \
          xdr_array.o \
          xdr_mem.o \
          xdr_float.o \
          xdr_reference.o\
