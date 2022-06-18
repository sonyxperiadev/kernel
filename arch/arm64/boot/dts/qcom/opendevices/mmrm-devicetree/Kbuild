ifeq ($(CONFIG_ARCH_WAIPIO), y)
dtbo-y += waipio-mmrm.dtbo
dtbo-y += waipio-mmrm-test.dtbo
dtbo-y += waipio-v2-mmrm.dtbo
dtbo-y += waipio-v2-mmrm-test.dtbo
endif

always-y	:= $(dtb-y) $(dtbo-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
