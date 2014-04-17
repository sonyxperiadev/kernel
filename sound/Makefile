# Makefile for the Linux sound card driver
#
ifeq ($(CONFIG_ARCH_RHEA),y)
obj-$(CONFIG_BCM_ALSA_SOUND) += caph/
endif
ifeq ($(CONFIG_ARCH_HAWAII),y)
obj-$(CONFIG_BCM_ALSA_SOUND) += caph_hawaii/
endif
ifeq ($(CONFIG_ARCH_JAVA),y)
obj-$(CONFIG_BCM_ALSA_SOUND) += caph_hawaii/
endif

obj-$(CONFIG_SOUND) += soundcore.o
obj-$(CONFIG_SOUND_PRIME) += sound_firmware.o
obj-$(CONFIG_SOUND_PRIME) += oss/
obj-$(CONFIG_DMASOUND) += oss/
obj-$(CONFIG_SND) += core/ i2c/ drivers/ isa/ pci/ ppc/ arm/ sh/ synth/ usb/ \
	firewire/ sparc/ spi/ parisc/ pcmcia/ mips/ soc/ atmel/
obj-$(CONFIG_SND_AOA) += aoa/

# This one must be compilable even if sound is configured out
obj-$(CONFIG_AC97_BUS) += ac97_bus.o

ifeq ($(CONFIG_SND),y)
  obj-y += last.o
endif

soundcore-objs  := sound_core.o
