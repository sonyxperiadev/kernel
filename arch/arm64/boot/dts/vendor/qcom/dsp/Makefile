# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-dsp.dtbo

dtbo-$(CONFIG_ARCH_PARROT) += \
	parrot-dsp.dtbo

dtbo-$(CONFIG_ARCH_PINEAPPLE) += \
	pineapple-dsp.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
