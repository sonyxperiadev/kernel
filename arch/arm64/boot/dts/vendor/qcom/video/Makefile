# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-vidc.dtbo

dtbo-$(CONFIG_ARCH_KALAMA) += \
	kalama-vidc.dtbo

dtbo-$(CONFIG_ARCH_PARROT) += \
	parrot-vidc.dtbo

dtbo-$(CONFIG_ARCH_PINEAPPLE) += \
	pineapple-vidc.dtbo \
	pineapple-vidc-v2.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
