# BCMDHD_43362_ANDROID
DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER                     \
	-DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP -DBCMFILEIMAGE  \
	-DDHDTHREAD -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DSDTEST -DBDC -DTOE    \
	-DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG               \
	-DOOB_INTR_ONLY       \
	-DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DWLP2P     \
	-DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT          \
	-DKEEP_ALIVE -DCSCAN -DPKT_FILTER_SUPPORT     \
	-DEMBEDDED_PLATFORM -DPNO_SUPPORT          \
	-DOEM_ANDROID -DLINUX -DWL_ENABLE_P2P_IF 	  \
	-Idrivers/net/wireless/bcmdhd_43362 -Idrivers/net/wireless/bcmdhd_43362/include

# the below flags have been REMOVED from the release Makefile (not supported)
# -DHW_OOB, -DGET_CUSTOM_MAC_ENABLE , -DWL_CFG80211_STA_EVENT, -DENABLE_INSMOD_NO_FW_LOAD

# the below flags have been MODIFIED from the release Makefile
DHDCFLAGS += -DCUSTOMER_HW3 -DCUSTOM_OOB_GPIO_NUM=74

# below flags have been ADDED for Rheaberri

# Uncomment to enable firmware dongle debug messages.
# DHDCFLAGS += -DDHD_DEBUG -DSRCBASE=\"$(SRCBASE)\"

# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
DHDCFLAGS += -DDHD_USE_STATIC_BUF
endif



DHDOFILES = aiutils.o bcmsdh_sdmmc_linux.o dhd_linux.o siutils.o bcmutils.o   \
	dhd_linux_sched.o bcmwifi.o dhd_sdio.o bcmevent.o dhd_bta.o hndpmu.o  \
	bcmsdh.o dhd_cdc.o bcmsdh_linux.o dhd_common.o linux_osl.o            \
	bcmsdh_sdmmc.o dhd_custom_gpio.o sbutils.o wldev_common.o wl_android.o dhd_cfg80211.o

obj-$(CONFIG_BCMDHD_43362_ANDROID) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)
ifneq ($(CONFIG_WIRELESS_EXT),)
bcmdhd-objs += wl_iw.o
DHDCFLAGS += -DSOFTAP
endif
ifneq ($(CONFIG_CFG80211),)
bcmdhd-objs += wl_cfg80211.o wl_cfgp2p.o wl_linux_mon.o
DHDCFLAGS += -DWL_CFG80211
endif
EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_BCMDHD_43362_ANDROID),m)
EXTRA_LDFLAGS += --strip-debug
endif
