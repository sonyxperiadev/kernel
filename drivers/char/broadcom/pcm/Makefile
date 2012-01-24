# Don't compile VCE driver by default
CONFIG_BCM_HW_PCM	?= n

#Build flags
EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2

EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -D__linux__
EXTRA_CFLAGS += -I./include/linux/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/chal/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/csl/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/mach
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/mach/
ifdef CONFIG_ARCH_RHEA_B0
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/mach/rdb_B0/
else
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/mach/rdb_A0/
endif
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/chal/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/csl/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/osabstract/

OBJ :=  hw_pcm.o csl_hw_pcm_sspi.o

obj-$(CONFIG_BCM_HW_PCM) := hw_pcm-driver.o
	hw_pcm-driver-objs := $(OBJ)

$(info ++++++++++++++++++++++)
$(info CONFIG_BCM_HW_PCM = $(CONFIG_BCM_HW_PCM))
$(info obj = $(obj))

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

install:
	@echo " Install dirs. $(INSTALL_MOD_DIR) $(INSTALL_MOD_PATH) $(KMODPATH)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order *~*
	-rm -rf .tmp_versions
