EXTRA_CFLAGS := -Werror

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/halaudio_drivers/bcm_island/pcm

.PHONY: all

else

obj-$(CONFIG_BCM_HALAUDIO_PCM_BCM_ISLAND) += \
	halaudio_pcm.o \

endif
