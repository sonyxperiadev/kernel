# SPDX-License-Identifier: GPL-2.0

ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
# SoMC Columbia
dtbo-$(CONFIG_ARCH_SONY_COLUMBIA) += parrot-columbia-pdx246_generic-overlay.dtbo
parrot-columbia-pdx246_generic-overlay.dtbo-base := parrot.dtb
else
# SoMC Columbia
dtb-$(CONFIG_ARCH_SONY_COLUMBIA) += parrot-columbia-pdx246_generic.dtb
endif

always-y	:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
