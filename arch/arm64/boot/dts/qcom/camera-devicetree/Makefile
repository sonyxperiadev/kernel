ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY), y)

dtbo-$(CONFIG_ARCH_WAIPIO) += waipio-camera.dtbo
dtbo-$(CONFIG_ARCH_WAIPIO) += waipio-camera-overlay-v2.dtbo
dtbo-$(CONFIG_ARCH_WAIPIO) += waipio-camera-sensor-nagara-pdx223.dtbo

else
$(error CONFIG_BUILD_ARM64_DT_OVERLAY is: $(CONFIG_BUILD_ARM64_DT_OVERLAY))

endif # ($(CONFIG_BUILD_ARM64_DT_OVERLAY), y)

always-y	:= $(dtbo-y) $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo