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
#the GPL, without Broadcom's express prior written consent
#*******************************************************************************************/

# Makefile for the audio caph driver
# use $(srctree) for kernel top (see below) and
# use $(src) in a relative path which points to the directory

KBUILD_CFLAGS += -Werror

#audio driver Source Files
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= at_aud_ctl.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= bcm_auddrv_test.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= audio_ddriver.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= audio_vdriver.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= audio_rpc.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= audio_vdriver_audlog.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= voif_handler.o
obj-$(CONFIG_BCM_AUDIO_SELFTEST) += bcm_auddrv_selftest.o

