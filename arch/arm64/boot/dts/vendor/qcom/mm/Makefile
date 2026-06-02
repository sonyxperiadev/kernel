# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_KALAMA) += \
	kalama-mm-qrd-overlay.dtbo

dtbo-$(CONFIG_ARCH_PINEAPPLE) += \
	pineapple-mm-qrd-overlay.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
