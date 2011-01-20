#
# Makefile for the linux kernel.
#

EXTRA_CFLAGS += -DVCHIQ_ARM_SIDE -DVCOS_VERIFY_BKPTS=1 -I../vc4/interface/vchiq_arm -I../vc4 -I../vc4/interface/vcos/linuxkernel -Ilinuxkernel

obj-$(CONFIG_MACH_BCM2708) 	+= clock.o bcm2708.o armctrl.o vcio.o ipc_chip.o
obj-$(CONFIG_VCHIQ)		+= vchiq_arm.o core.o vcos_linuxkernel.o
obj-$(CONFIG_DUMMY_TOUCHSCREEN)		+= dummy_touchscreen.o
