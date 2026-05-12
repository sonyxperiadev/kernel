# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-mmrm.dtbo \
	waipio-mmrm-test.dtbo \
	waipio-v2-mmrm.dtbo \
	waipio-v2-mmrm-test.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
