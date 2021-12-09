ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
dtbo-$(CONFIG_MACH_SONY_PDX203) += \
	kona-edo-pdx203_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_PDX206) += \
	kona-edo-pdx206_generic-overlay.dtbo

dtbo-$(CONFIG_MACH_SONY_PDX213) += \
	lagoon-lena-pdx213_generic-overlay.dtbo

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

dtb-$(CONFIG_MACH_SONY_KIRIN) += \
	sdm630-ganges-kirin.dtb
dtb-$(CONFIG_MACH_SONY_MERMAID) += \
	sdm636-ganges-mermaid.dtb

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
