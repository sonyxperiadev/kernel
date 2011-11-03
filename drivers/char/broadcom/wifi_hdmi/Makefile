EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -O2
EXTRA_CFLAGS  += -DOS_ASSERT_FAILURE
EXTRA_CFLAGS  += -D__STDC_VERSION=199901L
EXTRA_CFLAGS  += -D__STDC_VERSION__=199901L 
EXTRA_CFLAGS  += -D__VCCOREVER__=0
EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -D__linux__
EXTRA_CFLAGS  += -Werror

EXTRA_CFLAGS  += -I"vc4-dev/src/"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/generic"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/linuxkernel"
EXTRA_CFLAGS  += -I"drivers/misc/vc04_services/vmcs_sm"
EXTRA_CFLAGS  += -I"drivers/char/broadcom/wifi_hdmi/vcsvc"

obj-$(CONFIG_BCM_WIFI_HDMI)      := wifihdmi.o
obj-y                         	:= vcsvc_test/

wifihdmi-objs := \
   wifi_hdmi.o \
   vcsvc/vc_vchi_wifihdmi.o

