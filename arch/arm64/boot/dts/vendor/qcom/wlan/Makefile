# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-qca6490.dtbo

dtbo-$(CONFIG_ARCH_KALAMA) += \
	kalama-cnss.dtbo

dtbo-$(CONFIG_ARCH_PARROT) += \
	parrot-qrd-wcn3990.dtbo \
	parrot-qrd-wcn6750.dtbo

dtbo-$(CONFIG_ARCH_PINEAPPLE) += \
	pineapple-kiwi-cnss.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
