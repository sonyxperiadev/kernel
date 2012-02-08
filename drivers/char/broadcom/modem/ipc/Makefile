EXTRA_CFLAGS += -Werror

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DFUSE_DUAL_PROCESSOR_ARCHITECTURE
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR
EXTRA_CFLAGS += -D_RHEA_
EXTRA_CFLAGS += -DCONFIG_BRCM_FUSE_IPC_CIB
EXTRA_CFLAGS += -DBCM_INT_ID_RESERVED36=36

EXTRA_CFLAGS += -I$(srctree)/drivers/char/broadcom/fuse_log/

obj-$(CONFIG_BCM_MODEM) += \
	ipc_buffer.o \
	ipc_bufferpool.o \
	ipc_powersaving.o \
	ipc_queues.o \
	ipc_sharedmemory.o \
	ipc_trace.o \
	ipc_server.o \
	ipc_server_ccb.o \
	ipc_crashsupport.o \
	ipc_stubs.o \
	lnx_support.o \
	irq_apcp.o
