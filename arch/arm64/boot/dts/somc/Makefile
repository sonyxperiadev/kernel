ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

ifeq ($(CONFIG_MACH_SONY_PDX214),y)
dtbo-$(CONFIG_ARCH_SONY_SAGAMI) += lahaina-sagami-pdx214_generic-overlay.dtbo
endif
ifeq ($(CONFIG_MACH_SONY_PDX215),y)
dtbo-$(CONFIG_ARCH_SONY_SAGAMI) += lahaina-sagami-pdx215_generic-overlay.dtbo
endif
ifeq ($(CONFIG_MACH_SONY_PDX225),y)
dtbo-$(CONFIG_MACH_SONY_PDX225) += blair-murray-pdx225_sm5038_generic-overlay.dtbo
endif

ifeq ($(CONFIG_MACH_SONY_PDX214),y)
lahaina-sagami-pdx214_generic-overlay.dtbo-base := ../qcom/lahaina.dtb ../qcom/lahaina-v2.dtb ../qcom/lahaina-v2.1.dtb
endif
ifeq ($(CONFIG_MACH_SONY_PDX215),y)
lahaina-sagami-pdx215_generic-overlay.dtbo-base := ../qcom/lahaina.dtb ../qcom/lahaina-v2.dtb ../qcom/lahaina-v2.1.dtb
endif
ifeq ($(CONFIG_MACH_SONY_PDX225),y)
blair-murray-pdx225_sm5038_generic-overlay.dtbo-base := ../qcom/blair.dtb
endif

else
dtb-$(CONFIG_MACH_SONY_PDX214) += \
	lahaina-v2-sagami-pdx214_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX215) += \
	lahaina-v2-sagami-pdx215_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX225) += \
	blair-murray-pdx225_sm5038_generic.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
