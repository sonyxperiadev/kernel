#
# Makefile for SiW touch monitor
#

BASE_DIR = $(shell pwd)

EXT_MODULE_SYMBOL = $(BASE_DIR)/../Module.symvers

INSTALL_DIR = ./mod

CC=arm-eabi-gcc
#CC=arm-linux-gnueabihf-gcc

KERNEL_DIR = /home/kh2/work_od_and/kernel/samsung/exynos5422

BUILD_FLAGS = -I$(KERNEL_DIR)

EXTRA_CFLAGS = -D__KERNEL__ -DLINUX

CONFIG_TOUCHSCREEN_SIWMON=y
ifeq ($(CONFIG_TOUCHSCREEN_SIWMON), y)
EXTRA_CFLAGS += -DCONFIG_TOUCHSCREEN_SIWMON
endif

LIBS=-lm

MODULE_NAME = siwmon

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := siw_touch_mon.o
$(MODULE_NAME)-objs += siw_touch_mon_prt.o

module:
	$(MAKE) -C $(KERNEL_DIR) M=$(BASE_DIR) KBUILD_EXTRA_SYMBOLS+=$(EXT_MODULE_SYMBOL) modules ARCH=arm
#	cp $(MODULE_NAME).ko $(INSTALL_DIR)/

clean:
	rm -rf .tmp_versions
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf *.order
	rm -rf *.symvers
	rm -rf modules.builtin
	rm -rf $(MODULE_NAME)


