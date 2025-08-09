ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

dtbo-y += waipio-nagara-pdx223_generic-overlay.dtbo
dtbo-y += waipio-nagara-pdx224_generic-overlay.dtbo

lahaina-sagami-pdx214_generic-overlay.dtbo-base	:= lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahaina-sagami-pdx215_generic-overlay.dtbo-base	:= lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
waipio-nagara-pdx223_generic-overlay.dtbo-base	:= waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-nagara-pdx224_generic-overlay.dtbo-base	:= waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb

else # !CONFIG_BUILD_ARM64_DT_OVERLAY

dtb-y += \
	waipio-v2-nagara-pdx223_generic.dtb \
	waipio-v2-nagara-pdx224_generic.dtb

endif


always-y	:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
