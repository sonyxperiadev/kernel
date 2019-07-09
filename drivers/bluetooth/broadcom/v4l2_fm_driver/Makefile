# Makefile for V4L2 FM driver

fm_drv-objs := fmdrv_main.o fmdrv_v4l2.o fmdrv_rx.o
obj-$(CONFIG_V4L2_FM_DRIVER) := fm_drv.o

KBUILD_CFLAGS += -w
EXTRA_CFLAGS := -I$(PWD)/../include/

ccflags-y += -w -DVERSION="\"1.2\""
