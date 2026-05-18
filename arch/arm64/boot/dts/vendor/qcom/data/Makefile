# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-ipa.dtbo

dtbo-$(CONFIG_ARCH_KALAMA) += \
	kalama-ipa.dtbo

dtbo-$(CONFIG_ARCH_PARROT) += \
	parrot-ipa.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
