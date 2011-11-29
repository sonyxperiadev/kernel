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

#
## Makefile for audio CAPH driver

EXTRA_CFLAGS +=  -DMSP -DFUSE_DUAL_PROCESSOR_ARCHITECTURE -DFUSE_APPS_PROCESSOR -DDEVELOPMENT_ONLY -D_RHEA_ -DUNDER_LINUX -DUNDER_LINUX_MODEM -DLINUX_RPC_KERNEL 
#EXTRA_CFLAGS += -DENABLE_DMA_VOICE
#EXTRA_CFLAGS += -DVOLTE_SUPPORT
EXTRA_CFLAGS += -DINTERNAL_VOIF_TEST


#Linux header files
EXTRA_CFLAGS += -I./include/linux/
# For pmu audio driver
EXTRA_CFLAGS += -I./include/linux/broadcom
EXTRA_CFLAGS += -I./drivers/misc/
#Audio Controller header files
EXTRA_CFLAGS += -I$(src) -I$(src)/include -I$(src)/audio_controller
EXTRA_CFLAGS += -I$(src)/audio_driver/

#CSL header files
EXTRA_CFLAGS += -I$(src)/csl/
EXTRA_CFLAGS += -I$(src)/dsp/
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

obj-$(CONFIG_BCM_ALSA_SOUND)		+= snd-brcm-omx.o
snd-brcm-omx-objs := caph_module.o 
snd-brcm-omx-objs += caph_ctl.o
snd-brcm-omx-objs += caph_pcm.o
snd-brcm-omx-objs += caph_hwdep.o
snd-brcm-omx-objs += audio_controller/audio_caph.o
snd-brcm-omx-objs += audio_controller/audio_controller.o
snd-brcm-omx-objs += audio_controller/audio_pmu_adapt.o

#audio driver Source Files
snd-brcm-omx-objs += audio_driver/at_aud_ctl.o
snd-brcm-omx-objs += audio_driver/bcm_auddrv_test.o
snd-brcm-omx-objs += audio_driver/audio_ddriver.o
snd-brcm-omx-objs += audio_driver/audio_vdriver.o
snd-brcm-omx-objs += audio_driver/audio_rpc.o
snd-brcm-omx-objs += audio_driver/audio_vdriver_audlog.o
snd-brcm-omx-objs += audio_driver/voif_handler.o

#CHAL Source Files
ifdef CONFIG_ARCH_RHEA_B0
	snd-brcm-omx-objs += chal/B0/src/chal_caph_switch.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_srcmixer.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_other.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_intc.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_dma.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_dma_rhea_B0.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_cfifo.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_vout.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_vin.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_vibra.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_sidetone.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_nvin.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_ihf.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_hs.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_eanc.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_audio.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_analogtest.o
	snd-brcm-omx-objs += chal/B0/src/chal_caph_audioh_analogmic.o
else
	snd-brcm-omx-objs += chal/A0/src/chal_caph_switch.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_srcmixer.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_other.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_intc.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_dma.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_cfifo.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_vout.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_vin.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_vibra.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_sidetone.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_nvin.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_ihf.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_hs.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_eanc.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_audio.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_analogtest.o
	snd-brcm-omx-objs += chal/A0/src/chal_caph_audioh_analogmic.o
endif

#CSL Source Files
snd-brcm-omx-objs += csl/csl_caph_switch.o
snd-brcm-omx-objs += csl/csl_caph_srcmixer.o
snd-brcm-omx-objs += csl/csl_caph_render.o
snd-brcm-omx-objs += csl/csl_caph_pcm_sspi.o
snd-brcm-omx-objs += csl/csl_caph_i2s_sspi.o
snd-brcm-omx-objs += csl/csl_caph_hwctrl.o
snd-brcm-omx-objs += csl/csl_caph_dma.o
snd-brcm-omx-objs += csl/csl_caph_cfifo.o
snd-brcm-omx-objs += csl/csl_caph_capture.o
snd-brcm-omx-objs += csl/csl_caph_audioh.o
snd-brcm-omx-objs += csl/csl_aud_queue.o


#dsp source files (put to the end)
obj-$(CONFIG_BCM_MODEM) += snd-brcm-dsp.o
snd-brcm-dsp-objs += dsp/csl_apcmd.o
snd-brcm-dsp-objs += dsp/csl_dsp.o
snd-brcm-dsp-objs += dsp/csl_dsp_common_util.o
snd-brcm-dsp-objs += dsp/csl_arm2sp.o
snd-brcm-dsp-objs += dsp/csl_bttap.o
snd-brcm-dsp-objs += dsp/csl_vpu.o
snd-brcm-dsp-objs += dsp/csl_log.o
snd-brcm-dsp-objs += dsp/csl_voip.o
snd-brcm-dsp-objs += dsp/csl_voif.o
snd-brcm-dsp-objs += dsp/osdw_dsp_drv.o
snd-brcm-dsp-objs += dsp/csl_dsp_caph_control_api.o
snd-brcm-dsp-objs += dsp/csl_dsp_cneon_api.o




