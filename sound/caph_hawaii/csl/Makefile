#/*******************************************************************************************
#Copyright 2010 Broadcom Corporation.  All rights reserved.                                */
#
#Unless you and Broadcom execute a separate written software license agreement
#governing use of this software, this software is licensed to you under the
#terms of the GNU General Public License version 2, available at
#http://www.gnu.org/copyleft/gpl.html (the "GPL")
#
#Notwithstanding the above, under no circumstances may you combine this software
#in any way with any other Broadcom software provided under a license other than
#the GPL, without Broadcom's express prior written consent.
#*******************************************************************************************/

# Makefile for the audio caph driver
# use $(srctree) for kernel top (see below) and
# use $(src) in a relative path which points to the directory

#CSL Source Files
ifdef CONFIG_BCM_ALSA_SOUND
BUILD_CSL=y
else ifdef CONFIG_SND_BCM_SOC
BUILD_CSL=y
endif
obj-$(BUILD_CSL) 	+= csl_caph_switch.o
obj-$(BUILD_CSL) 	+= csl_caph_srcmixer.o
obj-$(BUILD_CSL) 	+= csl_caph_render.o
obj-$(BUILD_CSL) 	+= csl_caph_pcm_sspi.o
obj-$(BUILD_CSL) 	+= csl_caph_i2s_sspi.o
obj-$(BUILD_CSL) 	+= csl_caph_hwctrl.o
obj-$(BUILD_CSL) 	+= csl_caph_dma.o
obj-$(BUILD_CSL) 	+= csl_caph_cfifo.o
obj-$(BUILD_CSL) 	+= csl_caph_capture.o
obj-$(BUILD_CSL) 	+= csl_caph_audioh.o
obj-$(BUILD_CSL) 	+= csl_aud_queue.o

