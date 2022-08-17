ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

dtbo-$(CONFIG_MACH_SONY_PDX223) += waipio-nagara-pdx223_generic-overlay.dtbo

waipio-nagara-pdx223_generic-overlay.dtbo-base := ../qcom/waipio.dtb ../qcom/waipiop.dtb ../qcom/waipio-v2.dtb ../qcom/waipiop-v2.dtb ../qcom/waipio-lte.dtb

else # !CONFIG_BUILD_ARM64_DT_OVERLAY

dtb-$(CONFIG_MACH_SONY_PDX223) += waipio-v2-nagara-pdx223_generic.dtb
endif

always-y	:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
