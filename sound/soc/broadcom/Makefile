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
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DENABLE_DMA_VOICE
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DVOLTE_SUPPORT
#subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DENABLE_VOIF
ifdef CONFIG_ARCH_RHEA_BX
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -DCAPH_48K_MONO_PASSTHRU
endif

#Linux header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./include/linux/
# For pmu audio driver
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./include/linux/broadcom
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/misc/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/modem/public/soc/debug/public
#Audio Controller header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/caph -I./sound/caph/include
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/caph/audio_controller
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/caph/audio_driver/

#CSL header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/caph/csl/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/caph/dsp/
#CHAL header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I$(src)/chal/B0/inc/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./sound/caph/chal/B0/inc/
#RPC Header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/modem/rpc/rpc_CIB/public/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/fuse_log
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/modem/public/

#Platform header files
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/mach
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./drivers/char/broadcom/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/mach-rhea/include/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/mach-rhea/include/mach/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/mach-rhea/include/mach/rdb_Bx/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/chal/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/csl/
subdir-ccflags-$(CONFIG_SND_BCM_SOC) += -I./arch/arm/plat-kona/include/plat/osabstract/

#
# Broadcom 21654 Platform Support
#
snd-soc-caph-objs := caph-pcm.o
snd-soc-caph-i2s-objs := caph-i2s.o

obj-$(CONFIG_SND_BCM_SOC) += snd-soc-caph.o
obj-$(CONFIG_SND_BCM_SOC_I2S) += snd-soc-caph-i2s.o

# Broadcom 21654 Machine Support
snd-soc-rhea-objs := rhea.o

obj-$(CONFIG_SND_BCM_SOC_RHEA) += snd-soc-rhea.o

obj-$(CONFIG_SND_BCM_SOC) += ../../caph/chal/B0/src/
obj-$(CONFIG_SND_BCM_SOC) += ../../caph/csl/
obj-$(CONFIG_SND_BCM_SOC) += ../../caph/dsp/
