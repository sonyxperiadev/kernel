EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2 

EXTRA_CFLAGS  += -I"vc4-dev/src/"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/generic"
EXTRA_CFLAGS  += -I"vc4-dev/src/interface/vcos/linuxkernel"

EXTRA_CFLAGS  += -DOS_ASSERT_FAILURE
EXTRA_CFLAGS  += -D__STDC_VERSION=199901L
EXTRA_CFLAGS  += -D__STDC_VERSION__=199901L 
EXTRA_CFLAGS  += -D__VCCOREVER__=0
EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -D__linux__

obj-$(CONFIG_VC_VMCS_FB) := vc-vmcs_fb.o
vc-vmcs_fb-objs := \
    vmcs_fb.o \
    vc_vchi_fb.o

