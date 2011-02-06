#Makefile for xscript kernel
#EXTRA_CFLAGS := -Werror

EXTRA_CFLAGS += -DUNDER_LINUX

obj-$(CONFIG_BRCM_XSCRIPT) += xscript_kernel.o
obj-$(CONFIG_BRCM_XSCRIPT_GPIO) += gpio/
obj-$(CONFIG_BRCM_XSCRIPT_DMA) += dma/
obj-$(CONFIG_BRCM_XSCRIPT_REGULATOR) += regulator/
obj-$(CONFIG_BRCM_XSCRIPT_I2S) += i2s/
