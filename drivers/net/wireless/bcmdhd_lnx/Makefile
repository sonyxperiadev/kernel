# BCMDHD_LNX

DHDCFLAGS = -Dlinux -DLINUX -DBCMDRIVER -DBCMDONGLEHOST -DDHDTHREAD
DHDCFLAGS += -DUNRELEASEDCHIP -DBCMDMA32 -DDHD_SCHED
DHDCFLAGS += -DDHD_DEBUG -DBDC -DTOE -DDHD_BCMEVENTS -DSHOW_EVENTS
DHDCFLAGS += -DDHD_SDALIGN=64 -DMAX_HDR_READ=64 -DDHD_FIRSTREAD=64
DHDCFLAGS += -DBCMSDIO -DDHD_GPL -DBCMLXSDMMC -DBCMPLATFORM_BUS
DHDCFLAGS += -DWL_CFG80211 -DWLP2P -DOEM_EMBEDDED_LINUX
DHDCFLAGS += -DENABLE_INSMOD_NO_FW_LOAD
DHDCFLAGS += -DARP_OFFLOAD_SUPPORT -DPKT_FILTER_SUPPORT -DKEEP_ALIVE
DHDCFLAGS += -DPNO_SUPPORT
#DHDCFLAGS += -DPNO_SET_DEBUG
DHDCFLAGS += -Wall -Wstrict-prototypes -Werror

#NOTE: change the gpio # below to reflect your HW design
DHDCFLAGS += -DCUSTOMER_HW3 -DCUSTOM_OOB_GPIO_NUM=74

# WAPI support (enable with nl80211 patch)
#DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI

# In-Band Interrupt
#DHDCFLAGS += -DSDIO_ISR_THREAD -DMMC_SDIO_ABORT

# HW-OOB Interrupt
#DHDCFLAGS += -DOOB_INTR_ONLY -DHW_OOB -DMMC_SDIO_ABORT

# SW-OOB Interrupt -- SELECT THIS FOR RHEABERRI LMP
DHDCFLAGS += -DOOB_INTR_ONLY -DMMC_SDIO_ABORT


DHDCFLAGS += -Idrivers/net/wireless/bcmdhd_lnx -Idrivers/net/wireless/bcmdhd_lnx/include

#------------------------------------------------------------------------

DHDOFILES = \
	aiutils.o bcmevent.o bcmsdh.o bcmsdh_linux.o bcmsdh_sdmmc.o \
	bcmsdh_sdmmc_linux.o bcmutils.o bcmwifi.o dhd_cdc.o dhd_cfg80211.o \
	dhd_common.o dhd_custom_gpio.o dhd_linux.o dhd_linux_sched.o \
	dhd_sdio.o hndpmu.o linux_osl.o sbutils.o siutils.o wl_cfg80211.o \
	wl_cfgp2p.o wldev_common.o wl_elinux.o wl_linux_mon.o

obj-$(CONFIG_BCMDHD_43362_LINUX) += bcmdhd_lnx.o
bcmdhd_lnx-objs += $(DHDOFILES)
EXTRA_CFLAGS = $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug
