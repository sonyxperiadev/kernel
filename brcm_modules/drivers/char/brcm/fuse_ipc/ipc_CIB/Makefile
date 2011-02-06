#Makefile for IPC sources
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_CIB/ap/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/ipc_CIB/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_ipc/
EXTRA_CFLAGS += -I $(BRCM_MODULES_DIR)/drivers/char/brcm/fuse_log/

EXTRA_CFLAGS += -DUNDER_LINUX -DFUSE_APPS_PROCESSOR -DFUSE_DUAL_PROCESSOR_ARCHITECTURE 

obj-$(CONFIG_BRCM_FUSE_IPC_CIB) += \
	src/ipc_buffer.o	\
	src/ipc_bufferpool.o		\
	src/ipc_powersaving.o	\
	src/ipc_queues.o		\
	src/ipc_sharedmemory.o	\
	src/ipc_trace.o
