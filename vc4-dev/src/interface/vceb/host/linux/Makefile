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

#obj-y := vceb_linux_boot_mode.o

#obj-$(CONFIG_VC_VCHIQ_BUSDRV_JAZZ) += vceb-pif.o
#vceb-pif-objs := jazz/vceb-host-interface.o 

obj-$(CONFIG_VC_VCHIQ_MEMDRV_HANA) 	+= vceb-hana.o
#obj-$(CONFIG_VC_VCHIQ_BUSDRV_SHAREDMEM) += vceb-hana.o

vceb-hana-objs := hana/vceb-host-interface-hana.o

#obj-$(CONFIG_VC_VCHIQ_BUSDRV_MPHI) += vceb-mphi.o
#vceb-mphi-objs := mphi/vceb-host-interface-mphi.o
#vceb-mphi-objs += mphi/vchost-log.o
# We're in the vc4/src/interface/vceb/host/linux directory and want to add in
# arch/arm/mach-bcmhana/csp/mphi/chal_mphi.c
#vceb-mphi-objs += ../../../../../../arch/arm/mach-bcmhana/csp/mphi/chal_mphi_new.o


#ifneq ($(CONFIG_VC_VCHIQ_BUSDRV_VESPER),)
#   OBJ += vesper_ep4/vceb_host_interface.o vesper_ep4/dma.o \
#          vesper_ep4/images.o
#endif

#ifneq ($(CONFIG_VC_VCHIQ_BUSDRV_RAINBOW),)
#   OBJ += rainbow/vceb_host_interface.o rainbow/dma.o rainbow/images.o
#endif

obj-m += vc-vceb.o
vc-vceb-objs := $(OBJ)

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
