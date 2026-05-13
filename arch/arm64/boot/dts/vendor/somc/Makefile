# SPDX-License-Identifier: GPL-2.0

# SoMC Columbia
dtbo-$(CONFIG_ARCH_SONY_COLUMBIA) += \
	parrot-columbia-pdx246_generic-overlay.dtbo

# SoMC Elbe
dtbo-$(CONFIG_ARCH_SONY_ELBE) += \
	parrot-elbe-pdx257_generic-overlay.dtbo

# SoMC Nagara
dtbo-$(CONFIG_ARCH_SONY_NAGARA) += \
	waipio-nagara-pdx223_generic-overlay.dtbo \
	waipio-nagara-pdx224_generic-overlay.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
