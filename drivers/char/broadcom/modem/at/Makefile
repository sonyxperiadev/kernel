#Makefile for at kernel
#EXTRA_CFLAGS := -Werror

# for capi2_phonectrl_api.h
# common definitons for all capi2 related include paths
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/public/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/CAPI2_CIB/capi2/gen/
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/at/at_CIB/public
EXTRA_CFLAGS += -I drivers/char/broadcom/modem/rpc/rpc_CIB/public/

EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX

obj-y += atc_kernel.o
