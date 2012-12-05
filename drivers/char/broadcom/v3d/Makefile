#Default compile as module
CONFIG_V3D	?= n

#Build flags
EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2

EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -D__linux__

OBJ :=  wrapper.o session.o driver.o device.o statistics.o status.o
#OBJ :=  v3d.o

obj-$(CONFIG_V3D) := v3d-driver.o
	v3d-driver-objs := $(OBJ)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

install:
	@echo " Install dirs. $(INSTALL_MOD_DIR) $(INSTALL_MOD_PATH) $(KMODPATH)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order *~*
	-rm -rf .tmp_versions
