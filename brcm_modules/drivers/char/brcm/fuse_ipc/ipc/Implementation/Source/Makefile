#Makefile for IPC sources
EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/ipc/Implementation/Headers/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/

EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR

obj-$(CONFIG_BRCM_FUSE_IPC_2153SDB) += \
	IPC_ApPowersaving.o	\
	IPC_Buffer.o		\
	IPC_BufferPool.o	\
	IPC_Queues.o		\
	IPC_SharedMemory.o	\
	IPC_Trace.o
