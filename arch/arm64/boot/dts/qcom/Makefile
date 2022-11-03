ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

dtbo-$(CONFIG_MACH_SONY_PDX214) += lahaina-sagami-pdx214_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_PDX215) += lahaina-sagami-pdx215_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_PDX223) += waipio-nagara-pdx223_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_PDX224) += waipio-nagara-pdx224_generic-overlay.dtbo

lahaina-sagami-pdx214_generic-overlay.dtbo-base	:= lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahaina-sagami-pdx215_generic-overlay.dtbo-base	:= lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
waipio-nagara-pdx223_generic-overlay.dtbo-base	:= waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-nagara-pdx224_generic-overlay.dtbo-base	:= waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb

else # !CONFIG_BUILD_ARM64_DT_OVERLAY

dtb-$(CONFIG_MACH_SONY_PDX214) += \
	lahaina-v2-sagami-pdx214_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX215) += \
	lahaina-v2-sagami-pdx215_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX223) += \
	waipio-v2-nagara-pdx223_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX224) += \
	waipio-v2-nagara-pdx224_generic.dtb

endif


always-y	:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
