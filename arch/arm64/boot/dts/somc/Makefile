ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

# SoMC Sagami
dtbo-$(CONFIG_MACH_SONY_PDX214) += lahaina-sagami-pdx214_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_PDX215) += lahaina-sagami-pdx215_generic-overlay.dtbo
lahaina-sagami-pdx214_generic-overlay.dtbo-base := ../qcom/lahaina.dtb ../qcom/lahaina-v2.dtb ../qcom/lahaina-v2.1.dtb
lahaina-sagami-pdx215_generic-overlay.dtbo-base := ../qcom/lahaina.dtb ../qcom/lahaina-v2.dtb ../qcom/lahaina-v2.1.dtb

# SoMC Murray
dtbo-$(CONFIG_MACH_SONY_PDX225) += blair-murray-pdx225_sm5038_generic-overlay.dtbo
blair-murray-pdx225_sm5038_generic-overlay.dtbo-base := ../qcom/blair.dtb

# Somc Zambezi
dtbo-$(CONFIG_MACH_SONY_PDX235) += blair-zambezi-pdx235_generic-overlay.dtbo
blair-zambezi-pdx235_generic-overlay.dtbo-base := ../qcom/blair.dtb

else

# SoMC Sagami
dtb-$(CONFIG_MACH_SONY_PDX214) += \
	lahaina-v2-sagami-pdx214_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX215) += \
	lahaina-v2-sagami-pdx215_generic.dtb

# SoMC Murray
dtb-$(CONFIG_MACH_SONY_PDX225) += \
	blair-murray-pdx225_sm5038_generic.dtb

# Somc Zambezi
dtb-$(CONFIG_MACH_SONY_PDX235) += \
	blair-zambezi-pdx235_generic.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
