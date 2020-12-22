ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
dtbo-$(CONFIG_MACH_SONY_PDX203) += \
	kona-edo-pdx203_generic-overlay.dtbo
dtbo-$(CONFIG_MACH_SONY_PDX206) += \
	kona-edo-pdx206_generic-overlay.dtbo

ifeq ($(CONFIG_MACH_SONY_PDX203),y)
kona-edo-pdx203_generic-overlay.dtbo-base := ../qcom/kona.dtb ../qcom/kona-v2.dtb ../qcom/kona-v2.1.dtb
endif
ifeq ($(CONFIG_MACH_SONY_PDX206),y)
kona-edo-pdx206_generic-overlay.dtbo-base := ../qcom/kona.dtb ../qcom/kona-v2.dtb ../qcom/kona-v2.1.dtb
endif

else
dtb-$(CONFIG_MACH_SONY_PDX203) += \
	kona-edo-pdx203_generic.dtb \
	kona-v2-edo-pdx203_generic.dtb \
	kona-v2.1-edo-pdx203_generic.dtb
dtb-$(CONFIG_MACH_SONY_PDX206) += \
	kona-edo-pdx206_generic.dtb \
	kona-v2-edo-pdx206_generic.dtb \
	kona-v2.1-edo-pdx206_generic.dtb
endif

ifeq ($(CONFIG_ARM64),y)
always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
else
targets += dtbs
targets += $(addprefix ../, $(dtb-y))

$(obj)/../%.dtb: $(src)/%.dts FORCE
	$(call if_changed_dep,dtc)

dtbs: $(addprefix $(obj)/../,$(dtb-y))
endif
clean-files	:= *.dtb *.dtbo
