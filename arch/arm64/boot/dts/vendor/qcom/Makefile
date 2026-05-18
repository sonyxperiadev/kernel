# SPDX-License-Identifier: GPL-2.0

# add-overlay defines the target with following naming convention:
# <base>-<board>-dtbs = base.dtb board.dtbo
#
# Combined dtb target is also generated using the fdt_overlay tool.
# dtb-y += <base>-<board>.dtb

add-overlays = $(foreach o,$1,$(foreach b,$2,$(eval $(basename $b)-$(basename $o)-dtbs = $b $o) $(basename $b)-$(basename $o).dtb))

# sun-dtb-y is list of DTBs to install when doing non-overlay build
# sun-overlays-dtb-y is list of DTBs and DTBOs to install when doing overlay-supported build
# see DTB_TYPES from scripts/Makefile.dtbinst and install_dtbs from build.config.msm.common
#

# SoC Waipio
WAIPIO_BASE_DTB += waipio.dtb waipio-v2.dtb

WAIPIO_BOARDS += \
	waipio-v2-qrd-pm8010.dtbo

waipio-dtb-$(CONFIG_ARCH_WAIPIO) += \
	$(call add-overlays, $(WAIPIO_BOARDS),$(WAIPIO_BASE_DTB))
waipio-overlays-dtb-$(CONFIG_ARCH_WAIPIO) += $(WAIPIO_BOARDS) $(WAIPIO_BASE_DTB)
dtb-y += $(waipio-dtb-y)

# SoC Kalama
KALAMA_BASE_DTB += kalama.dtb kalama-v2.dtb

KALAMA_BOARDS += \
	kalama-qrd-overlay.dtbo

kalama-dtb-$(CONFIG_ARCH_KALAMA) += \
	$(call add-overlays, $(KALAMA_BOARDS),$(KALAMA_BASE_DTB))
kalama-overlays-dtb-$(CONFIG_ARCH_KALAMA) += $(KALAMA_BOARDS) $(KALAMA_BASE_DTB)
dtb-y += $(kalama-dtb-y)

# SoC Parrot
PARROT_BASE_DTB += parrot.dtb parrot-gen1.dtb

PARROT_BOARDS += \
	parrot-qrd-overlay.dtbo \
	parrot-qrd-wcn6750-overlay.dtbo

parrot-dtb-$(CONFIG_ARCH_PARROT) += \
	$(call add-overlays, $(PARROT_BOARDS),$(PARROT_BASE_DTB))
parrot-overlays-dtb-$(CONFIG_ARCH_PARROT) += $(PARROT_BOARDS) $(PARROT_BASE_DTB)
dtb-y += $(parrot-dtb-y)

always-y    := $(dtb-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
