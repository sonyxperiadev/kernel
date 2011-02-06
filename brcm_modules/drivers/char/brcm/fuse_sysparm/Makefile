#Makefile for IPC driver
EXTRA_CFLAGS := -Werror

ifdef CONFIG_BRCM_FUSE_IPC_2153SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc/Implementation/Headers/
else 
ifdef CONFIG_BRCM_FUSE_IPC_2157SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_2157/Implementation/Headers/
endif
endif

EXTRA_CFLAGS += -DUNDER_LINUX
ifeq ($(CONFIG_ARCH_BCM2157), y)
EXTRA_CFLAGS += -D BCM2153_FAMILY_BB_CHIP_BOND
endif
obj-$(CONFIG_BRCM_FUSE_SYSPARM) += \
	bcm_fuse_sysparm.o

