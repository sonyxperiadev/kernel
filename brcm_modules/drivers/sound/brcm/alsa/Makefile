#/*******************************************************************************************
#Copyright 2010 Broadcom Corporation.  All rights reserved.
#
#Unless you and Broadcom execute a separate written software license agreement 
#governing use of this software, this software is licensed to you under the 
#terms of the GNU General Public License version 2, available at 
#http://www.gnu.org/copyleft/gpl.html (the "GPL"). 
#
#Notwithstanding the above, under no circumstances may you combine this software 
#in any way with any other Broadcom software provided under a license other than 
#the GPL, without Broadcom's express prior written consent.
#*******************************************************************************************/

#
## Makefile for ALSA BRCM OMX
#
# -D USE_HAL_AUDIO_THREAD

EXTRA_CFLAGS := -Wno-error -D USE_HAL_AUDIO_THREAD -D FUSE_DUAL_PROCESSOR_ARCHITECTURE -D FUSE_APPS_PROCESSOR -U FUSE_COMMS_PROCESSOR -D _BCM2153_ -D CHIP_REVISION=51

ifeq ($(CONFIG_ARCH_BCM2157), y)
EXTRA_CFLAGS += -D BCM2153_FAMILY_BB_CHIP_BOND -I $(src)/hal2157 -I $(src)
ifeq ($(CONFIG_BOARD_BONDRAY),y)
EXTRA_CFLAGS += -D BUILD_BONDRAY
endif
else
EXTRA_CFLAGS += -I $(src)/hal2153 -I $(src)
endif

EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DSTACK_wedge

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ril/CAPI2/inc/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc/inc/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/ipc/Implementation/Headers/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/inc/

obj-$(CONFIG_BRCM_SOUND_ALSA) += snd-brcm-omx.o
snd-brcm-omx-objs := brcm_alsa_dev.o brcm_alsa_pcm.o
snd-brcm-omx-objs += brcm_alsa_ctl.o
snd-brcm-omx-objs += audvoc_dsp.o
snd-brcm-omx-objs += audvoc_ipc.o
snd-brcm-omx-objs += audvoc_drv.o
snd-brcm-omx-objs += audvoc_coeff.o
#snd-brcm-omx-objs += audvoc_ctrl.o
snd-brcm-omx-objs += audio_drv.o
#snd-brcm-omx-objs += hal_audio_core.o
#snd-brcm-omx-objs += hal_audio_access.o
snd-brcm-omx-objs += hal_audio_thread.o
snd-brcm-omx-objs += capi2_gen_cemu.o

#vpu
snd-brcm-omx-objs += vsharedmem.o
snd-brcm-omx-objs += vpu.o
snd-brcm-omx-objs += vpripcmdq.o
snd-brcm-omx-objs += sharedmem.o

# AAC-LC aacenc_dev.o  cirbuf.o
snd-brcm-omx-objs += aacenc_dev.o cirbuf.o

obj-$(CONFIG_AMP_MAX9877) += max9877.o

obj-$(CONFIG_AMP_NCP2704) += ncp2704.o

ifeq ($(CONFIG_ARCH_BCM2157), y)
obj-$(CONFIG_BRCM_SOUND_ALSA) += hal2157/
else
obj-$(CONFIG_BRCM_SOUND_ALSA) += hal2153/
endif
