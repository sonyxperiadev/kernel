# SPDX-License-Identifier: GPL-2.0

dtbo-$(CONFIG_ARCH_WAIPIO) += \
	waipio-mmrm.dtbo \
	waipio-mmrm-test.dtbo \
	waipio-v2-mmrm.dtbo \
	waipio-v2-mmrm-test.dtbo

dtbo-$(CONFIG_ARCH_KALAMA) += \
	kalama-mmrm.dtbo \
	kalama-mmrm-test.dtbo \
	kalama-v2-mmrm.dtbo \
	kalama-v2-mmrm-test.dtbo

dtbo-$(CONFIG_ARCH_PINEAPPLE) += \
	pineapple-mmrm.dtbo \
	pineapple-mmrm-test.dtbo \
	pineapple-mmrm-test-v2.dtbo

always-y    := $(dtb-y) $(dtbo-y)
subdir-y    := $(dts-dirs)
clean-files := *.dtb *.dtbo
