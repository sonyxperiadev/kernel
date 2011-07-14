#Default compile as module
CONFIG_BCM2835_TVINTF	?= n

#Build flags
EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -O2 

EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -D__linux__

OBJ :=  tv_intf_core.o tv_intf_driver.o

obj-$(CONFIG_BCM2835_TVINTF) := vc-tv_intf.o
	vc-tv_intf-objs := $(OBJ)

$(info ++++++++++++++++++++++)
$(info CONFIG_BCM2835_TVINTF = $(CONFIG_BCM2835_TVINTF))
$(info obj = $(obj))

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

install: 
	@echo " Install dirs. $(INSTALL_MOD_DIR) $(INSTALL_MOD_PATH) $(KMODPATH)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order *~*
	-rm -rf .tmp_versions
