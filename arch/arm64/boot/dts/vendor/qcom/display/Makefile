# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-sde.dtbo \
	waipio-sde-display-qrd-overlay.dtbo

dtbo-$(CONFIG_ARCH_PARROT) += \
	parrot-sde.dtbo \
	parrot-sde-display-qrd-overlay.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
