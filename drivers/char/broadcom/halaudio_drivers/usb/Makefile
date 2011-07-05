EXTRA_CFLAGS := -Werror

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/halaudio_drivers/usb

.PHONY: all

else

obj-$(CONFIG_BCM_HALAUDIO_USB) += \
	halaudio_usbcore.o halaudio_usb.o

endif
