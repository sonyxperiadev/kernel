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
EXTRA_CFLAGS += -I"./drivers/char/broadcom/modem/public/soc/debug/public"
#Audio Controller header files
EXTRA_CFLAGS += -I./sound/caph -I./sound/caph/include
EXTRA_CFLAGS += -I./sound/caph/audio_controller
EXTRA_CFLAGS += -I./sound/caph/audio_driver/

#CSL header files
EXTRA_CFLAGS += -I./sound/caph/csl/
EXTRA_CFLAGS += -I./sound/caph/dsp/
#CHAL header files
ifdef CONFIG_ARCH_RHEA_B0
EXTRA_CFLAGS += -I./sound/caph/chal/B0/inc/
else
EXTRA_CFLAGS += -I./sound/caph/chal/A0/inc/
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

# DSP Source Files
obj-$(CONFIG_BCM_MODEM) 	+= csl_apcmd.o
obj-$(CONFIG_BCM_MODEM) 	+= csl_dsp.o
obj-$(CONFIG_BCM_MODEM) 	+= csl_arm2sp.o 
obj-$(CONFIG_BCM_MODEM) 	+= csl_bttap.o 
obj-$(CONFIG_BCM_MODEM) 	+= csl_vpu.o 
obj-$(CONFIG_BCM_MODEM) 	+= csl_log.o 
obj-$(CONFIG_BCM_MODEM) 	+= csl_voip.o 
obj-$(CONFIG_BCM_MODEM) 	+= csl_voif.o 
obj-$(CONFIG_BCM_MODEM) 	+= osdw_dsp_drv.o 
obj-$(CONFIG_BCM_MODEM) 	+= csl_dsp_caph_control_api.o 
obj-$(CONFIG_BCM_MODEM) 	+= csl_dsp_cneon_api.o
obj-$(CONFIG_BCM_MODEM) 	+= csl_dsp_common_util.o
