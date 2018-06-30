dtb-$(CONFIG_ARCH_QCOM)	+= apq8016-sbc.dtb msm8916-mtp.dtb
dtb-$(CONFIG_ARCH_QCOM)	+= msm8996-mtp.dtb
dtb-$(CONFIG_ARCH_QCOM)	+= apq8096-db820c.dtb

ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
dtbo-$(CONFIG_MACH_SONY_AKARI) += \
		sdm845-tama-akari_generic-overlay.dtbo \
		sdm845-v2-tama-akari_generic-overlay.dtbo \
		sdm845-v2.1-tama-akari_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_APOLLO) += \
		sdm845-tama-apollo_generic-overlay.dtbo \
		sdm845-v2-tama-apollo_generic-overlay.dtbo \
		sdm845-v2.1-tama-apollo_generic-overlay.dtbo

sdm845-tama-akari_generic-overlay.dtbo-base := sdm845.dtb
sdm845-v2-tama-akari_generic-overlay.dtbo-base := sdm845-v2.dtb
sdm845-v2.1-tama-akari_generic-overlay.dtbo-base := sdm845-v2.1.dtb

sdm845-tama-apollo_generic-overlay.dtbo-base := sdm845.dtb
sdm845-v2-tama-apollo_generic-overlay.dtbo-base := sdm845-v2.dtb
sdm845-v2.1-tama-apollo_generic-overlay.dtbo-base := sdm845-v2.1.dtb
else
# SDM845
dtb-$(CONFIG_MACH_SONY_AKARI)		+= sdm845-tama-akari_generic.dtb
dtb-$(CONFIG_MACH_SONY_AKARI)		+= sdm845-v2-tama-akari_generic.dtb
dtb-$(CONFIG_MACH_SONY_AKARI)		+= sdm845-v2.1-tama-akari_generic.dtb
dtb-$(CONFIG_MACH_SONY_APOLLO)		+= sdm845-tama-apollo_generic.dtb
dtb-$(CONFIG_MACH_SONY_APOLLO)		+= sdm845-v2-tama-apollo_generic.dtb
dtb-$(CONFIG_MACH_SONY_APOLLO)		+= sdm845-v2.1-tama-apollo_generic.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb
