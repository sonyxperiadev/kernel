EXTRA_CFLAGS := -Werror

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/haptics

.PHONY: all

else

obj-$(CONFIG_BCM_HAPTICS) += \
	bcm_haptics.o

endif
