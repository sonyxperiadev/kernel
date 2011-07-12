EXTRA_CFLAGS := -Werror

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/halaudio_drivers/bcm_island/audioh

.PHONY: all

else

obj-$(CONFIG_BCM_HALAUDIO_AUDIOH_ISLAND) += \
	halaudio_audioh.o

endif
