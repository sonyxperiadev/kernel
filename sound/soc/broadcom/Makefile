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

#the options below are used by dsp headers
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DMSP -DFUSE_DUAL_PROCESSOR_ARCHITECTURE -DDEVELOPMENT_ONLY -D_RHEA_
#the options below are used by capi headers
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DUNDER_LINUX -DUNDER_LINUX_MODEM -DLINUX_RPC_KERNEL
#the options below are used by audio
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DENABLE_DMA_VOICE
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DVOLTE_SUPPORT
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DENABLE_VOIF
ifdef CONFIG_ARCH_HAWAII_BX
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DCAPH_48K_MONO_PASSTHRU
endif

#Linux header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./include/linux/
# For pmu audio driver
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./include/linux/broadcom
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/misc/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/modem/public/soc/debug/public
#Audio Controller header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/soc/broadcom -I./sound/soc/broadcom/include
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/soc/broadcom/audio_controller
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/soc/broadcom/audio_driver/

#CSL header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/soc/broadcom/csl/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/soc/broadcom/dsp/
#CHAL header files
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I$(src)/chal/capri/inc/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/soc/broadcom/chal/capri/inc/
#RPC Header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/modem/rpc/rpc_CIB/public/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/fuse_log
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/modem/public/

#Platform header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/mach
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/mach-hawaii/include/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/mach-hawaii/include/mach/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/mach-hawaii/include/mach/rdb_A0/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/chal/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/csl/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/osabstract/

#
# Broadcom 21664 Platform Support
#
snd-soc-caph-objs := caph-pcm.o
snd-soc-caph-i2s-objs := caph-i2s.o

obj-$(CONFIG_SND_BCM_SOC) += snd-soc-caph.o
obj-$(CONFIG_SND_BCM_SOC_I2S) += snd-soc-caph-i2s.o

# Broadcom 21664 Machine Support
snd-soc-hawaii-objs := hawaii.o

obj-$(CONFIG_SND_BCM_SOC_HAWAII) += snd-soc-hawaii.o

obj-$(CONFIG_SND_BCM_SOC) += chal/capri/src/
obj-$(CONFIG_SND_BCM_SOC) += csl/
obj-$(CONFIG_SND_BCM_SOC) += dsp/
