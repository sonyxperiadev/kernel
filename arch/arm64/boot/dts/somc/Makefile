ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

ifeq ($(CONFIG_MACH_SONY_PDX215),y)
dtbo-$(CONFIG_ARCH_SONY_SAGAMI) += lahaina-sagami-pdx215_generic-overlay.dtbo
else
dtbo-$(CONFIG_ARCH_SONY_SAGAMI) += lahaina-sagami-pdx215_generic-crossflash-overlay.dtbo
endif

ifeq ($(CONFIG_MACH_SONY_PDX215),y)
lahaina-sagami-pdx215_generic-overlay.dtbo-base := ../qcom/lahaina.dtb ../qcom/lahaina-v2.dtb ../qcom/lahaina-v2.1.dtb
else
lahaina-sagami-pdx215_generic-crossflash-overlay.dtbo-base := ../qcom/lahaina.dtb ../qcom/lahaina-v2.dtb ../qcom/lahaina-v2.1.dtb
endif
else
dtb-$(CONFIG_MACH_SONY_PDX215) += \
	lahaina-v2-sagami-pdx215_generic.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
