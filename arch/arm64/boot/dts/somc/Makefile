ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
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

dtbo-$(CONFIG_MACH_SONY_GRIFFIN) += \
	sm8150-kumano-griffin_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_BAHAMUT) += \
	sm8150-kumano-bahamut_generic-overlay.dtbo

dtbo-$(CONFIG_MACH_SONY_PDX201) += \
	trinket-seine-pdx201_generic-overlay.dtbo

dtbo-$(CONFIG_MACH_SONY_PDX203) += \
	kona-edo-pdx203_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_PDX206) += \
	kona-edo-pdx206_generic-overlay.dtbo

dtbo-$(CONFIG_MACH_SONY_PDX213) += \
	lagoon-lena-pdx213_generic-overlay.dtbo

ifeq ($(CONFIG_MACH_SONY_AKARI),y)
sdm845-tama-akari_generic-overlay.dtbo-base := ../qcom/sdm845.dtb
sdm845-v2-tama-akari_generic-overlay.dtbo-base := ../qcom/sdm845-v2.dtb
sdm845-v2.1-tama-akari_generic-overlay.dtbo-base := ../qcom/sdm845-v2.1.dtb
endif
ifeq ($(CONFIG_MACH_SONY_AKATSUKI),y)
sdm845-tama-akatsuki_generic-overlay.dtbo-base := ../qcom/sdm845.dtb
sdm845-v2-tama-akatsuki_generic-overlay.dtbo-base := ../qcom/sdm845-v2.dtb
sdm845-v2.1-tama-akatsuki_generic-overlay.dtbo-base := ../qcom/sdm845-v2.1.dtb
endif
ifeq ($(CONFIG_MACH_SONY_APOLLO),y)
sdm845-tama-apollo_generic-overlay.dtbo-base := ../qcom/sdm845.dtb
sdm845-v2-tama-apollo_generic-overlay.dtbo-base := ../qcom/sdm845-v2.dtb
sdm845-v2.1-tama-apollo_generic-overlay.dtbo-base := ../qcom/sdm845-v2.1.dtb
endif

ifeq ($(CONFIG_MACH_SONY_GRIFFIN),y)
sm8150-kumano-griffin_generic-overlay.dtbo-base := ../qcom/sm8150.dtb ../qcom/sm8150-v2.dtb
endif
ifeq ($(CONFIG_MACH_SONY_BAHAMUT),y)
sm8150-kumano-bahamut_generic-overlay.dtbo-base := ../qcom/sm8150.dtb ../qcom/sm8150-v2.dtb
endif

ifeq ($(CONFIG_MACH_SONY_PDX201),y)
trinket-seine-pdx201_generic-overlay.dtbo-base := ../qcom/trinket.dtb
endif

ifeq ($(CONFIG_MACH_SONY_PDX203),y)
kona-edo-pdx203_generic-overlay.dtbo-base := ../qcom/kona.dtb ../qcom/kona-v2.dtb ../qcom/kona-v2.1.dtb
endif
ifeq ($(CONFIG_MACH_SONY_PDX206),y)
kona-edo-pdx206_generic-overlay.dtbo-base := ../qcom/kona.dtb ../qcom/kona-v2.dtb ../qcom/kona-v2.1.dtb
endif
ifeq ($(CONFIG_MACH_SONY_PDX213),y)
lagoon-lena-pdx213_generic-overlay.dtbo-base := ../qcom/lagoon.dtb
endif

else
dtb-$(CONFIG_MACH_SONY_PIONEER) += \
	sdm630-nile-pioneer.dtb
dtb-$(CONFIG_MACH_SONY_DISCOVERY) += \
	sdm630-nile-discovery.dtb
dtb-$(CONFIG_MACH_SONY_VOYAGER) += \
	sdm630-nile-voyager.dtb

dtb-$(CONFIG_MACH_SONY_AKARI) += \
	sdm845-tama-akari_generic.dtb \
	sdm845-v2-tama-akari_generic.dtb \
	sdm845-v2.1-tama-akari_generic.dtb
dtb-$(CONFIG_MACH_SONY_AKATSUKI) += \
	sdm845-tama-akatsuki_generic.dtb \
	sdm845-v2-tama-akatsuki_generic.dtb \
	sdm845-v2.1-tama-akatsuki_generic.dtb
dtb-$(CONFIG_MACH_SONY_APOLLO) += \
	sdm845-tama-apollo_generic.dtb \
	sdm845-v2-tama-apollo_generic.dtb \
	sdm845-v2.1-tama-apollo_generic.dtb

dtb-$(CONFIG_MACH_SONY_KIRIN) += \
	sdm630-ganges-kirin.dtb
dtb-$(CONFIG_MACH_SONY_MERMAID) += \
	sdm636-ganges-mermaid.dtb

dtb-$(CONFIG_MACH_SONY_GRIFFIN) += \
	sm8150-kumano-griffin_generic.dtb \
	sm8150-v2-kumano-griffin_generic.dtb
dtb-$(CONFIG_MACH_SONY_BAHAMUT) += \
	sm8150-kumano-bahamut_generic.dtb \
	sm8150-v2-kumano-bahamut_generic.dtb

dtb-$(CONFIG_MACH_SONY_PDX201) += \
	trinket-seine-pdx201_generic.dtb

dtb-$(CONFIG_MACH_SONY_PDX203) += \
	kona-edo-pdx203_generic.dtb \
	kona-v2-edo-pdx203_generic.dtb \
	kona-v2.1-edo-pdx203_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX206) += \
	kona-edo-pdx206_generic.dtb \
	kona-v2-edo-pdx206_generic.dtb \
	kona-v2.1-edo-pdx206_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX213) += \
	lagoon-lena-pdx213_generic.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
