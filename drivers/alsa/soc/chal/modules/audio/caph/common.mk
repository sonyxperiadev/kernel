
ifeq ($(BASEBAND_CHIP),RHEA)

SRCS =	chal_caph_cfifo.c \
		chal_caph_intc.c \
		chal_caph_switch.c \
		chal_caph_dma.c \
		chal_caph_srcmixer.c \
		chal_caph_other.c \
		chal_caph_audioh_analogmic.c \
		chal_caph_audioh_analogtest.c \
		chal_caph_audioh_audio.c \
		chal_caph_audioh_eanc.c \
		chal_caph_audioh_hs.c \
		chal_caph_audioh_ihf.c \
		chal_caph_audioh_nvin.c \
		chal_caph_audioh_sidetone.c \
		chal_caph_audioh_vibra.c \
		chal_caph_audioh_vin.c \
		chal_caph_audioh_vout.c 

else

SRCS = 	chal_caph_audioh_vout.c 

endif

INCPATH =	$(STD_CHAL_MODULE_INCPATH)

CCFLAGS = $(STD_CCFLAGS)

TARGET_LIB_PREFIX = lib_chal_
