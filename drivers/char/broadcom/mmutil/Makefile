# Module by default
CONFIG_BCM_MMUTIL ?= m

#Build flags
EXTRA_CFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2
EXTRA_CFLAGS += -Wno-parentheses

EXTRA_CFLAGS += -D__KERNEL__
EXTRA_CFLAGS += -D__linux__

OBJ := bcm_mmutil.o

obj-$(CONFIG_BCM_MMUTIL) := bcm_mmutil-driver.o
	bcm_mmutil-driver-objs := $(OBJ)

$(info ++++++++++++++++++++++)
$(info CONFIG_BCM_MMUTIL = $(CONFIG_BCM_MMUTIL))
$(info obj = $(obj))

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

install:
	@echo " Install dirs. $(INSTALL_MOD_DIR) $(INSTALL_MOD_PATH) $(KMODPATH)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order *~*
	-rm -rf .tmp_versions
