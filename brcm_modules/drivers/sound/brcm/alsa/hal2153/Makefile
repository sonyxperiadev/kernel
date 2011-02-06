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

## Makefile for ALSA BRCM OMX
#
# -D USE_HAL_AUDIO_THREAD

EXTRA_CFLAGS := -Wno-error -D USE_HAL_AUDIO_THREAD -D FUSE_DUAL_PROCESSOR_ARCHITECTURE -D FUSE_APPS_PROCESSOR -U FUSE_COMMS_PROCESSOR -D _BCM2153_ -D CHIP_REVISION=51 

EXTRA_CFLAGS += -I $(src)/../

obj-$(CONFIG_BRCM_SOUND_ALSA) += 	hal_audio_core.o \
									hal_audio_access.o
