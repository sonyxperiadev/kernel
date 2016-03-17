#Default compile as module
CONFIG_ISP2	?= n

#Build flags
ccflags-y  += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2

ccflags-y  += -D__KERNEL__
ccflags-y  += -D__linux__

OBJ :=  isp2.o

obj-$(CONFIG_ISP2) := isp2-driver.o
	isp2-driver-objs := $(OBJ)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

install:
	@echo " Install dirs. $(INSTALL_MOD_DIR) $(INSTALL_MOD_PATH) $(KMODPATH)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order *~*
	-rm -rf .tmp_versions
