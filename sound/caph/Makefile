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

EXTRA_CFLAGS +=  -DMSP -DFUSE_DUAL_PROCESSOR_ARCHITECTURE -DFUSE_APPS_PROCESSOR -DDEVELOPMENT_ONLY -D_RHEA_ -DUNDER_LINUX -DUNDER_LINUX_MODEM -DLINUX_RPC_KERNEL 
#EXTRA_CFLAGS += -DENABLE_DMA_VOICE
#EXTRA_CFLAGS += -DVOLTE_SUPPORT
EXTRA_CFLAGS += -DCONFIG_BCMPMU_AUDIO
ifdef CONFIG_ARCH_RHEA_B0
EXTRA_CFLAGS += -DCAPH_48K_MONO_PASSTHRU
endif

#Linux header files
EXTRA_CFLAGS += -I./include/linux/
# For pmu audio driver
EXTRA_CFLAGS += -I./include/linux/broadcom
EXTRA_CFLAGS += -I./drivers/misc/
EXTRA_CFLAGS += -I./drivers/char/broadcom/modem/public/soc/debug/public
#Audio Controller header files
EXTRA_CFLAGS += -I./sound/caph -I./sound/caph/include
EXTRA_CFLAGS += -I./sound/caph/audio_controller
EXTRA_CFLAGS += -I./sound/caph/audio_driver/

#CSL header files
EXTRA_CFLAGS += -I./sound/caph/csl/
EXTRA_CFLAGS += -I./sound/caph/dsp/
#CHAL header files
ifdef CONFIG_ARCH_RHEA_B0
EXTRA_CFLAGS += -I$(src)/chal/B0/inc/
else
EXTRA_CFLAGS += -I$(src)/chal/A0/inc/
endif
#RPC Header files
EXTRA_CFLAGS += -I./drivers/char/broadcom/modem/rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I./drivers/char/broadcom/modem/CAPI2_CIB/soc/debug/public/
EXTRA_CFLAGS += -I./drivers/char/broadcom/modem/CAPI2_CIB/public/
EXTRA_CFLAGS += -I./drivers/char/broadcom/fuse_log

#Platform header files
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/mach
EXTRA_CFLAGS += -I./drivers/char/broadcom/
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/mach/
ifdef CONFIG_ARCH_RHEA_B0
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/mach/rdb_B0/
else
EXTRA_CFLAGS += -I./arch/arm/mach-rhea/include/mach/rdb_A0/
endif
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/chal/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/csl/
EXTRA_CFLAGS += -I./arch/arm/plat-kona/include/plat/osabstract/

obj-$(CONFIG_BCM_ALSA_SOUND) 	+= caph_module.o 
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= caph_ctl.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= caph_pcm.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= caph_hwdep.o
obj-$(CONFIG_BCM_ALSA_SOUND)	+= audio_controller/ audio_driver/
ifdef CONFIG_ARCH_RHEA_B0
obj-$(CONFIG_BCM_ALSA_SOUND) += chal/B0/src/
else
obj-$(CONFIG_BCM_ALSA_SOUND) += chal/A0/src/
endif
obj-$(CONFIG_BCM_ALSA_SOUND)	+= csl/
obj-$(CONFIG_BCM_MODEM)    += dsp/

