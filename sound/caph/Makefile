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
## Makefile for audio CAPH driver

# -D USE_HAL_AUDIO_THREAD

ifdef CONFIG_ARCH_RHEA
include $(src)/Rhea_Makefile
endif

<<<<<<< HEAD
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -DUNDER_LINUX_MODEM
EXTRA_CFLAGS += -DLINUX_RPC_KERNEL
EXTRA_CFLAGS += -D _RHEA_  -D MSP -D FUSE_DUAL_PROCESSOR_ARCHITECTURE -D FUSE_APPS_PROCESSOR -D PMU_BCM59055 -D CHAL_NDEBUG_BUILD -D VPU_INCLUDED -D DEVELOPMENT_ONLY -D NDEBUG
#Linux header files
EXTRA_CFLAGS += -I ./include/linux/
EXTRA_CFLAGS += -I ./include/linux/broadcom
#Audio Controller header files
EXTRA_CFLAGS += -I $(src)/common -I $(src)/audio/audio_controller -I $(src)/audio/audio_controller/public
EXTRA_CFLAGS += -I $(src)/audio/audio_vdriver/public/
EXTRA_CFLAGS += -I $(src)/audio/public/
EXTRA_CFLAGS += -I $(src)/audio/audio_datadriver/public/
#CSL header files
EXTRA_CFLAGS += -I $(src)/soc/csl/audio/public/
EXTRA_CFLAGS += -I $(src)/soc/csl/dsp/public/
EXTRA_CFLAGS += -I $(src)/soc/csl/bsp/public/
#CHAL header files
EXTRA_CFLAGS += -I $(src)/soc/chal/modules/audio/inc/
EXTRA_CFLAGS += -I $(src)/soc/chal/modules/audio/caph/
EXTRA_CFLAGS += -I $(src)/soc/chal/modules/bsp/inc/
#RDB header files
EXTRA_CFLAGS += -I $(src)/rdb/rhea/A0
#DSP header files
EXTRA_CFLAGS += -I $(src)/dsp/public/
EXTRA_CFLAGS += -I $(src)/dsp/rhea/include/
#SYSPARM Header files
#EXTRA_CFLAGS += -I $(src)/sysinterface/sysparms/
#RPC Header files
EXTRA_CFLAGS += -I ./drivers/char/broadcom/modem/rpc/rpc_CIB/public/
#LOG Header files
EXTRA_CFLAGS += -I $(src)/soc/debug/public
#PMU Header files
#EXTRA_CFLAGS += -I $(src)/peripherals/pmu/public
#EXTRA_CFLAGS += -I $(src)/sysinterface/hal/pmu/public
=======
ifdef CONFIG_ARCH_ISLAND
include $(src)/Island_Makefile
endif
>>>>>>> ea35777f3b21201096165bb5b67e63962b9cf197

ifdef CONFIG_ARCH_SAMOA
include $(src)/Samoa_Makefile
endif
