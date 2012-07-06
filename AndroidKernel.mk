#Android makefile to build kernel as a part of Android Build

ifeq ($(TARGET_PREBUILT_KERNEL),)

KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
KERNEL_CONFIG := $(KERNEL_OUT)/.config
TARGET_PREBUILT_INT_KERNEL := $(KERNEL_OUT)/arch/arm/boot/zImage
KERNEL_HEADERS_INSTALL := $(KERNEL_OUT)/usr
KERNEL_MODULES_INSTALL := system
KERNEL_MODULES_OUT := $(TARGET_OUT)/lib/modules

ifeq ($(TARGET_USES_UNCOMPRESSED_KERNEL),true)
$(info Using uncompressed kernel)
TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/piggy
else
TARGET_PREBUILT_KERNEL := $(TARGET_PREBUILT_INT_KERNEL)
endif

define mv-modules
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`;\
ko=`find $$mpath/kernel -type f -name *.ko`;\
for i in $$ko; do mv $$i $(KERNEL_MODULES_OUT)/; done;\
fi
endef

define clean-module-folder
mdpath=`find $(KERNEL_MODULES_OUT) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`; rm -rf $$mpath;\
fi
endef

$(KERNEL_OUT):
	mkdir -p $(KERNEL_OUT)

$(KERNEL_CONFIG): $(KERNEL_OUT)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- $(KERNEL_DEFCONFIG)

$(KERNEL_OUT)/piggy : $(TARGET_PREBUILT_INT_KERNEL)
	$(hide) gunzip -c $(KERNEL_OUT)/arch/arm/boot/compressed/piggy.gzip > $(KERNEL_OUT)/piggy

$(TARGET_PREBUILT_INT_KERNEL): $(KERNEL_OUT) $(KERNEL_CONFIG) $(KERNEL_HEADERS_INSTALL)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi-
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- modules
	$(MAKE) -C kernel O=../$(KERNEL_OUT) INSTALL_MOD_PATH=../../$(KERNEL_MODULES_INSTALL) ARCH=arm CROSS_COMPILE=arm-eabi- modules_install
	$(mv-modules)
	$(clean-module-folder)

$(KERNEL_HEADERS_INSTALL): $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- headers_install

kerneltags: $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- tags

kernelconfig: $(KERNEL_OUT) $(KERNEL_CONFIG)
	env KCONFIG_NOTIMESTAMP=true \
	     $(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- menuconfig
	env KCONFIG_NOTIMESTAMP=true \
	     $(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi- savedefconfig
	cp $(KERNEL_OUT)/defconfig kernel/arch/arm/configs/$(KERNEL_DEFCONFIG)

endif

#
# Rules for packing kernel into elf and sin
#
$(PRODUCT_OUT)/cmdline.txt: device/semc/$(SEMC_PLATFORM)/BoardConfig.mk
	$(hide) echo -n '$(BOARD_KERNEL_CMDLINE)' > $@

$(PRODUCT_OUT)/kernel-unsigned.elf: $(TARGET_PREBUILT_KERNEL) $(PRODUCT_OUT)/ramdisk.img $(PRODUCT_OUT)/RPM.bin $(PRODUCT_OUT)/cmdline.txt | sin-tools
	$(hide) $(HOST_OUT_EXECUTABLES)/mkelf.py -o $@ \
		$(TARGET_PREBUILT_KERNEL)@$(BOARD_KERNEL_ADDR) \
		$(PRODUCT_OUT)/ramdisk.img@$(BOARD_RAMDISK_ADDR),ramdisk \
		$(PRODUCT_OUT)/RPM.bin@$(BOARD_RPM_ADDR),rpm

$(PRODUCT_OUT)/kernel-signed.elf: $(PRODUCT_OUT)/kernel-unsigned.elf $(PRODUCT_PARTITION_CONFIG) | sin-tools
	$(hide) $(SEMCSC) -c $(PRODUCT_PARTITION_CONFIG) -p Kernel -t internal -i $< -o $@

$(PRODUCT_OUT)/kernel.si_: $(PRODUCT_OUT)/kernel-signed.elf | sin-tools
	$(hide) $(HOST_OUT_EXECUTABLES)/create_sin_header Kernel $(PRODUCT_PARTITION_CONFIG) $@
	$(hide) cat $< >> $@

$(PRODUCT_OUT)/kernel.sin: $(PRODUCT_OUT)/kernel.si_ $(PRODUCT_PARTITION_CONFIG) | sin-tools
	@echo target SIN: $(notdir $@)
	$(hide) $(SEMCSC) -c $(PRODUCT_PARTITION_CONFIG) -p Kernel -t external -i $< -o $@

#
# Add kernel to system wide PHONY target sin
#
.PHONY: sin

sin: $(PRODUCT_OUT)/kernel.sin

