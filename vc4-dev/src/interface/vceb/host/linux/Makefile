#
# IMPORTANT: The order of these is important.
#
#	Withint this directory, the order that objects are added to obj-y
#	will determine the order that the init functions are called.
#
#

# Default dir settings are within kernel source tree

EXTRA_CFLAGS  += -I"vc4-dev/src/"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vceb/host/linux"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vceb/host/common"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vceb/common"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vceb/host"

EXTRA_CFLAGS += -Wno-declaration-after-statement

#OBJ	:= vceb_linux_os.o \
#	vceb_linux_wrapper.o vceb_linux_host_interface.o vceb_bin_file.o \
#	../vceb.o ../../common/vceb_common.o ../common/vceb_interface.o \
#	vceb_app_code.o vceb_2nd_stage_code.o

OBJ  := vceb_linux_os.o \
   vceb_linux_wrapper.o vceb_linux_host_interface.o vceb_bin_file.o \
   ../vceb.o ../../common/vceb_common.o ../common/vceb_interface.o \
	vceb_app_code.o vceb_2nd_stage_code.o

#vc-vceb.o needs to be before any other sub-modules
obj-$(CONFIG_VC_VCEB) += vc-vceb.o
vc-vceb-objs := $(OBJ)

obj-$(CONFIG_VC_VCHIQ_MEMDRV_HANA) 	+= vceb-hana.o

vceb-hana-objs := hana/vceb-host-interface-hana.o

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

install:
	@echo " Install dirs. $(INSTALL_MOD_DIR) $(INSTALL_MOD_PATH) $(KMODPATH)"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) INSTALL_MOD_DIR=$(INSTALL_MOD_DIR) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers Module.markers modules.order *~*
	-rm -rf .tmp_versions
	-rm -f ../../../vmcs/src/interface/vmcs_host/vc_vchi_dispmanx.o ../../../vmcs/src/interface/vmcs_host/.vc_vchi_dispmanx.o.cmd
	-rm -f ../*.o ../.*.cmd vesper_ep4/*.o vesper_ep4/.*.cmd ../../common/*.o ../../common/.*.cmd
	-rm -f icestorm/*.o icestorm/.*.cmd
	-rm -f ../common/*.o ../common/.*.cmd
