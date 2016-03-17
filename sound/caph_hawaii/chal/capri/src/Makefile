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

#CAPH CHAL Capri Source Files
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_caph_switch.o 
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_caph_srcmixer.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_caph_intc.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_caph_dma.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_caph_cfifo.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_vout.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_vin.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_vibra.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_sidetone.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_nvin.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_ihf.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_hs.o
#eanc file copied from Rhea cHAL. Not in capri cHAL
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_eanc.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_audio.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_analogtest.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_analogmic.o
obj-$(CONFIG_BCM_ALSA_SOUND) 	+= chal_sspi_hawaii.o
	
