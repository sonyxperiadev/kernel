#
# Makefile for the Synopsis DWC OTG common portable library 
#

EXTRA_CFLAGS	+= -DDWC_LINUX

obj-$(CONFIG_USB_DWC_OTG)	:= dwc_common_port_lib.o

dwc_common_port_lib-objs	:= dwc_cc.o dwc_modpow.o dwc_dh.o \
				   dwc_crypto.o dwc_notifier.o \
				   dwc_common_linux.o dwc_mem.o
