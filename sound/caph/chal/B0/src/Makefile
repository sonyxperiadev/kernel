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
#CAPH CHAL B0 Source Files
ifdef CONFIG_BCM_ALSA_SOUND
BUILD_CHAL=y
else ifdef CONFIG_SND_BCM_SOC
BUILD_CHAL=y
endif

obj-$(BUILD_CHAL) 	+= chal_caph_switch.o
obj-$(BUILD_CHAL) 	+= chal_caph_srcmixer.o
obj-$(BUILD_CHAL) 	+= chal_caph_other.o
obj-$(BUILD_CHAL) 	+= chal_caph_intc.o
obj-$(BUILD_CHAL) 	+= chal_caph_dma.o
obj-$(BUILD_CHAL) 	+= chal_caph_dma_rhea_B0.o
obj-$(BUILD_CHAL) 	+= chal_caph_cfifo.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_vout.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_vin.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_vibra.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_sidetone.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_nvin.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_ihf.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_hs.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_eanc.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_audio.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_analogtest.o
obj-$(BUILD_CHAL) 	+= chal_caph_audioh_analogmic.o

