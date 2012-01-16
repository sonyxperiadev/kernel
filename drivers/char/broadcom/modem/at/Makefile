#Makefile for at kernel
#EXTRA_CFLAGS := -Werror

# for capi2_phonectrl_api.h
# common definitons for all capi2 related include paths
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/CAPI2_CIB/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/CAPI2_CIB/capi2/gen/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/at/at_CIB/public
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/modem/public/
EXTRA_CFLAGS += -I$(srctree)/include/linux/broadcom/

EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DLINUX_RPC_KERNEL

obj-$(CONFIG_BCM_MODEM) += \
	atc_kernel.o
