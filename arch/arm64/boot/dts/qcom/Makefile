# SPDX-License-Identifier: GPL-2.0

# SoMC Nagara
WAIPIO_BASE_DTB += waipio-v2.dtb

WAIPIO_SOMC_BOARDS += \
	waipio-nagara-pdx223_generic-overlay.dtbo \
	waipio-nagara-pdx224_generic-overlay.dtbo

waipio-nagara-dtbs := $(WAIPIO_BASE_DTB) $(WAIPIO_SOMC_BOARDS)
dtb-$(CONFIG_ARCH_SONY_NAGARA) += waipio-nagara.dtb

# SoMC Yodo
KALAMA_BASE_DTB += kalama-v2.dtb

KALAMA_SOMC_BOARDS += \
	kalama-yodo-pdx234_generic-overlay.dtbo

kalama-yodo-dtbs := $(KALAMA_BASE_DTB) $(KALAMA_SOMC_BOARDS)
dtb-$(CONFIG_ARCH_SONY_YODO) += kalama-yodo.dtb

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
