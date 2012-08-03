#/*******************************************************************************************
#Copyright 2010 Broadcom Corporation.  All rights reserved.                                */
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

# Makefile for the audio caph driver
# use $(srctree) for kernel top (see below) and
# use $(src) in a relative path which points to the directory

KBUILD_CFLAGS += -Werror

#the options below are used by dsp headers
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DMSP -DFUSE_DUAL_PROCESSOR_ARCHITECTURE -DDEVELOPMENT_ONLY -D_RHEA_
#the options below are used by capi headers
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DUNDER_LINUX -DUNDER_LINUX_MODEM -DLINUX_RPC_KERNEL
#the options below are used by audio
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DENABLE_DMA_VOICE
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DENABLE_BT16
#subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DDYNAMIC_DMA_PLAYBACK
#subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DVOLTE_SUPPORT
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DCONFIG_CAPH_DYNAMIC_SRC_CLK
ifdef CONFIG_ARCH_RHEA_BX
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DCAPH_48K_MONO_PASSTHRU
endif
# enable the SS multicast for Rhea Stone variant builds
ifneq ($(filter y,$(CONFIG_MACH_RHEA_STONE) $(CONFIG_MACH_RHEA_STONE_EDN2X)),)
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DCONFIG_ENABLE_SSMULTICAST
endif
ifdef CONFIG_ARCH_HAWAII
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -DCAPH_48K_MONO_PASSTHRU
endif

#Linux header files
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./include/linux/
# For pmu audio driver
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./include/linux/broadcom
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./drivers/misc/
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./drivers/char/broadcom/modem/public/soc/debug/public
#Audio Controller header files
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./sound/caph -I./sound/caph/include
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./sound/caph/audio_controller
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./sound/caph/audio_driver/

#CSL header files
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./sound/caph/csl/
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./sound/caph/dsp/
#CHAL header files
ifdef CONFIG_ARCH_RHEA_BX
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I$(src)/chal/B0/inc/
else
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I$(src)/chal/A0/inc/
endif
ifdef CONFIG_ARCH_HAWAII
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I$(src)/chal/B0/inc/
endif
#RPC Header files
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./drivers/char/broadcom/modem/rpc/rpc_CIB/public/
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./drivers/char/broadcom/fuse_log
subdir-ccflags-$(CONFIG_BCM_ALSA_SOUND) += -I./drivers/char/broadcom/modem/public/

obj-$(CONFIG_BCM_ALSA_SOUND) += caph_module.o
obj-$(CONFIG_BCM_ALSA_SOUND) += caph_ctl.o
obj-$(CONFIG_BCM_ALSA_SOUND) += caph_pcm.o
obj-$(CONFIG_BCM_ALSA_SOUND) += caph_hwdep.o
obj-$(CONFIG_BCM_ALSA_SOUND) += caph_ptt_hwdep.o
obj-$(CONFIG_BCM_ALSA_SOUND) += audio_controller/ audio_driver/
ifdef CONFIG_ARCH_RHEA_BX
obj-$(CONFIG_BCM_ALSA_SOUND) += chal/B0/src/
else
obj-$(CONFIG_BCM_ALSA_SOUND) += chal/A0/src/
endif
ifdef CONFIG_ARCH_HAWAII
obj-$(CONFIG_BCM_ALSA_SOUND) += chal/B0/src/
endif
obj-$(CONFIG_BCM_ALSA_SOUND) += csl/
#obj-$(CONFIG_BCM_MODEM)    += dsp/
obj-$(CONFIG_BCM_ALSA_SOUND) += dsp/
