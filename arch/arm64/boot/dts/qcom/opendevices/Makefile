ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
# TRINKET - SOMC Seine
dtbo-$(CONFIG_MACH_SONY_PDX201) += \
        trinket-seine-pdx201_generic-overlay.dtbo

trinket-seine-pdx201_generic-overlay.dtbo-base := trinket.dtb

# SM8150 - SoMC Kumano
dtbo-$(CONFIG_MACH_SONY_GRIFFIN) += \
		sm8150-kumano-griffin_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_BAHAMUT) += \
		sm8150-kumano-bahamut_generic-overlay.dtbo

sm8150-kumano-griffin_generic-overlay.dtbo-base := sm8150.dtb sm8150-v2.dtb
sm8150-kumano-bahamut_generic-overlay.dtbo-base := sm8150.dtb sm8150-v2.dtb


# SDM845 - SoMC Tama
dtbo-$(CONFIG_MACH_SONY_AKARI) += \
		sdm845-tama-akari_generic-overlay.dtbo \
		sdm845-v2-tama-akari_generic-overlay.dtbo \
		sdm845-v2.1-tama-akari_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_AKATSUKI) += \
		sdm845-tama-akatsuki_generic-overlay.dtbo \
		sdm845-v2-tama-akatsuki_generic-overlay.dtbo \
		sdm845-v2.1-tama-akatsuki_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_APOLLO) += \
		sdm845-tama-apollo_generic-overlay.dtbo \
		sdm845-v2-tama-apollo_generic-overlay.dtbo \
		sdm845-v2.1-tama-apollo_generic-overlay.dtbo

sdm845-tama-akari_generic-overlay.dtbo-base := sdm845.dtb
sdm845-v2-tama-akari_generic-overlay.dtbo-base := sdm845-v2.dtb
sdm845-v2.1-tama-akari_generic-overlay.dtbo-base := sdm845-v2.1.dtb

sdm845-tama-akatsuki_generic-overlay.dtbo-base := sdm845.dtb
sdm845-v2-tama-akatsuki_generic-overlay.dtbo-base := sdm845-v2.dtb
sdm845-v2.1-tama-akatsuki_generic-overlay.dtbo-base := sdm845-v2.1.dtb

sdm845-tama-apollo_generic-overlay.dtbo-base := sdm845.dtb
sdm845-v2-tama-apollo_generic-overlay.dtbo-base := sdm845-v2.dtb
sdm845-v2.1-tama-apollo_generic-overlay.dtbo-base := sdm845-v2.1.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
