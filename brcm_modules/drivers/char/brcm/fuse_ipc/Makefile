#Makefile for IPC driver
EXTRA_CFLAGS := -Werror

ifdef CONFIG_BRCM_FUSE_IPC_2153SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc/Implementation/Headers/
else 
ifdef CONFIG_BRCM_FUSE_IPC_2157SDB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_2157/Implementation/Headers/
else
ifdef CONFIG_BRCM_FUSE_IPC_CIB
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
endif
endif
endif

EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_log/
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX

obj-$(CONFIG_BRCM_FUSE_IPC_2153SDB) += \
	lnx_support.o \
	ipc_server_ccb.o \
	ipc_server.o \
	ipc_stubs.o \
	ipc_crashsupport.o \
	ipc/Implementation/Source/ 

obj-$(CONFIG_BRCM_FUSE_IPC_2157SDB) += \
	lnx_support.o \
	ipc_server_ccb.o \
	ipc_server.o \
	ipc_stubs.o \
	ipc_crashsupport.o \
	ipc_2157/Implementation/Source/ 

obj-$(CONFIG_BRCM_FUSE_IPC_CIB) += \
	lnx_support.o \
	ipc_server_ccb.o \
	ipc_server.o \
	ipc_stubs.o \
	ipc_crashsupport.o \
	ipc_CIB/

