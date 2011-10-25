EXTRA_CFLAGS := -Werror

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/hdmi

.PHONY: all

else

obj-$(CONFIG_BCM_HDMI_DET) += \
	hdmi.o

endif
