#
# Makefile for the Synopsis DWC OTG driver
#
DWC_COMMON_PORT_DIR	:= drivers/usb/gadget/dwc_otg/dwc_common_port

EXTRA_CFLAGS	+=  -DDWC_LINUX
EXTRA_CFLAGS	+=   -I$(DWC_COMMON_PORT_DIR)
ccflags-$(CONFIG_USB_BCM_OTG) += -DDWC_DEV_SRPCAP
ccflags-$(CONFIG_USB_DWC_OTG_DEVICE_ONLY) += -DDWC_DEVICE_ONLY

obj-$(CONFIG_USB_DWC_OTG)	+= dwc_otg.o

dwc_otg-objs    := dwc_otg_driver.o dwc_otg_attr.o
dwc_otg-objs    += dwc_otg_cil.o dwc_otg_cil_intr.o
dwc_otg-objs    += dwc_otg_pcd_linux.o dwc_otg_pcd.o dwc_otg_pcd_intr.o
dwc_otg-objs    += dwc_otg_hcd.o dwc_otg_hcd_linux.o dwc_otg_hcd_intr.o dwc_otg_hcd_queue.o dwc_otg_hcd_ddma.o
dwc_otg-objs    += dwc_otg_adp.o

ifneq ($(CFI),)
	dwc_otg-objs    += dwc_otg_cfi.o
endif
