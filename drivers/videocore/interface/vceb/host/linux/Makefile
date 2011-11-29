#
# IMPORTANT: The order of these is important.
#
#	Withint this directory, the order that objects are added to obj-y
#	will determine the order that the init functions are called.
#
#

# Default dir settings are within kernel source tree

EXTRA_CFLAGS  += -I"drivers/videocore"
EXTRA_CFLAGS  += -I"drivers/videocore/host_support/include"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vceb/host/linux"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vceb/host/common"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vceb/common"
EXTRA_CFLAGS  += -I"drivers/videocore/interface/vceb/host"

EXTRA_CFLAGS += -Wno-declaration-after-statement

# We need to build vc-vceb BEFORE building the sub-drivers. This ensures that
# the vceb module init routine will be called first.

obj-$(CONFIG_VC_VCEB) += vc-vceb.o
vc-vceb-objs := vceb_linux_os.o \
		vceb_linux_wrapper.o \
		vceb_linux_host_interface.o \
		vceb_bin_file.o \
		../vceb.o \
		../../common/vceb_common.o \
		../common/vceb_interface.o \
		vceb_app_code.o \
		vceb_2nd_stage_code.o

obj-$(CONFIG_VC_VCHIQ_MEMDRV_HANA) += vc-vceb-hana.o
vc-vceb-hana-objs := hana/vceb-host-interface-hana.o

obj-$(CONFIG_VC_VCHIQ_MEMDRV_KONA) += vc-vceb-kona.o
vc-vceb-kona-objs := kona/vceb-host-interface-kona.o

