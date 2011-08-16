EXTRA_CFLAGS := -Werror

ifeq ($(srctree),)

#
# We weren't called from the linux make environment, redirect
#

all:
	$(MAKE) -C ../../../../drivers/char/broadcom/halaudio/halaudio_core

.PHONY: all

else

obj-$(CONFIG_BCM_HALAUDIO_CORE) += \
   halaudio_core.o

endif
