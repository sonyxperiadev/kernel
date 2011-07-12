EXTRA_CFLAGS := -Werror
ifeq (1,$(strip $(CFG_GLOBAL_SND_AUDIO_FIXED_FREQ)))
   EXTRA_CFLAGS += -DSND_HAL_AUDIO_FIXED_FREQ
endif

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/halaudio_drivers/alsa

.PHONY: all

else

obj-$(CONFIG_BCM_HALAUDIO_ALSA) += \
	snd_halaudio.o

endif
