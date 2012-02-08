# BCMDHD_LNX

DHDCFLAGS = -Dlinux -DLINUX -DBCMDRIVER -DBCMDONGLEHOST -DDHDTHREAD
DHDCFLAGS += -DUNRELEASEDCHIP -DBCMDMA32 -DDHD_SCHED
DHDCFLAGS += -DDHD_DEBUG -DBDC -DTOE -DDHD_BCMEVENTS -DSHOW_EVENTS
DHDCFLAGS += -DDHD_SDALIGN=64 -DMAX_HDR_READ=64 -DDHD_FIRSTREAD=64
DHDCFLAGS += -DBCMSDIO -DDHD_GPL -DBCMLXSDMMC -DBCMPLATFORM_BUS
DHDCFLAGS += -DWL_CFG80211 -DWLP2P -DOEM_EMBEDDED_LINUX
#DHDCFLAGS += -DARP_OFFLOAD_SUPPORT -DPKT_FILTER_SUPPORT -DKEEP_ALIVE
#DHDCFLAGS += -DPNO_SUPPORT 
DHDCFLAGS += -Wall -Wstrict-prototypes -Werror
#DHDCFLAGS += -DCUSTOMER_HW3 -DCUSTOM_OOB_GPIO_NUM=85
DHDCFLAGS += -DCUSTOMER_HW3

# WAPI support (enable with nl80211 patch)
#DHDCFLAGS += -DBCMWAPI_WPI -DBCMWAPI_WAI

# In-Band Interrupt -- UNCOMMENT BELOW TO TEST ON RHEARAY, OR COMMENT OUT FOR OOB
DHDCFLAGS += -DSDIO_ISR_THREAD -DMMC_SDIO_ABORT

# FOR Rhea OOB INT - UNCOMMENT BELOW AND REPLACE "85" FOR HW-OOB + COMMENT ABOVE
#DHDCFLAGS += -DOOB_INTR_ONLY -DMMC_SDIO_ABORT -DCUSTOM_OOB_GPIO_NUM=85

# HW-OOB Interrupt - BELOW NOT USED i.e. HW_OOB (KEEP COMMENTED OUT)
#DHDCFLAGS += -DOOB_INTR_ONLY -DHW_OOB -DMMC_SDIO_ABORT -DCUSTOM_OOB_GPIO_NUM=85

# SW-OOB Interrupt -- BELOW NOT USED (KEEP COMMENTED OUT)
#DHDCFLAGS += -DOOB_INTR_ONLY -DMMC_SDIO_ABORT


DHDCFLAGS += -Idrivers/net/wireless/bcmdhd_lnx -Idrivers/net/wireless/bcmdhd_lnx/include

#------------------------------------------------------------------------

DHDOFILES = \
	aiutils.o bcmevent.o bcmsdh.o bcmsdh_linux.o bcmsdh_sdmmc.o \
	bcmsdh_sdmmc_linux.o bcmutils.o bcmwifi.o dhd_cdc.o dhd_cfg80211.o \
	dhd_common.o dhd_custom_gpio.o dhd_linux.o dhd_linux_sched.o \
	dhd_sdio.o hndpmu.o linux_osl.o sbutils.o siutils.o wl_cfg80211.o \
	wl_cfgp2p.o wldev_common.o wl_elinux.o wl_linux_mon.o

obj-$(CONFIG_BCMDHD_43362_LINUX) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)
EXTRA_CFLAGS = $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug
