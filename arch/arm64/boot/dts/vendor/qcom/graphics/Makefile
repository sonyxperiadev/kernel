# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-gpu.dtbo \
	waipio-v2-gpu.dtbo

dtbo-$(CONFIG_ARCH_KALAMA) += \
	kalama-gpu.dtbo \
	kalama-v2-gpu.dtbo

dtbo-$(CONFIG_ARCH_PARROT) += \
	parrot-gpu.dtbo

dtbo-$(CONFIG_ARCH_PINEAPPLE) += \
	pineapple-gpu.dtbo \
	pineapple-v2-gpu.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
