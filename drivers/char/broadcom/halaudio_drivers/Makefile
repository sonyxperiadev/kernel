EXTRA_CFLAGS := -Werror

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/halaudio_drivers/

.PHONY: all

else

obj-m                                        += bcm_island/
obj-$(CONFIG_BCM_HALAUDIO_ALSA)              += alsa/
obj-$(CONFIG_BCM_HALAUDIO_USB)               += usb/

endif
