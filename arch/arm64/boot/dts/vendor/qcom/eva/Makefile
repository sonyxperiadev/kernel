# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-eva.dtbo

dtbo-$(CONFIG_ARCH_KALAMA) += \
	kalama-eva.dtbo

dtbo-$(CONFIG_ARCH_PINEAPPLE) += \
	pineapple-eva.dtbo \
	pineapple-eva-v2.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
