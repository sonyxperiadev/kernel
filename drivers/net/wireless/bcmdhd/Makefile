# bcmdhd
DHDCFLAGS = -Wall -Wstrict-prototypes -Werror -Dlinux -DBCMDRIVER             \
	-DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP -DBCMFILEIMAGE  \
	-DDHDTHREAD -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DSDTEST -DBDC -DTOE    \
	-DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG               \
	-DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS             \
	-DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT          \
	-DKEEP_ALIVE -DPKT_FILTER_SUPPORT                                     \
	-DEMBEDDED_PLATFORM -DPNO_SUPPORT                                     \
	-DDHD_BCM_WIFI_HDMI                                                   \
	-DCUSTOMER_HW3							      \
	-DOEMANDROID                                                          \
	-Idrivers/net/wireless/bcmdhd -Idrivers/net/wireless/bcmdhd/include


# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DDHD_USE_STATIC_BUF
endif


ifeq ($(CONFIG_MACH_RHEA_BERRI_EDN40),y)
# SW-OOB Interrupt
DHDCFLAGS += -DOOB_INTR_ONLY -DCUSTOM_OOB_GPIO_NUM=74
DHDCFLAGS += -DDHD_SELECTIVE_FW_43362
endif

ifeq ($(CONFIG_MACH_RHEA_STONE),y)
# SW-OOB Interrupt
DHDCFLAGS += -DOOB_INTR_ONLY -DCUSTOM_OOB_GPIO_NUM=74
endif

ifeq ($(CONFIG_MACH_RHEA_SS),y)
# SW-OOB Interrupt
DHDCFLAGS += -DOOB_INTR_ONLY -DCUSTOM_OOB_GPIO_NUM=89
endif

#ifneq ($(CONFIG_MACH_RHEA_BERRI_EDN40),y)
#DHDCFLAGS += -DSDIO_ISR_THREAD                                                     
#endif

ifeq ($(CONFIG_MACH_RHEA_RAY_EDN2X),y)
DHDCFLAGS += -DSDIO_ISR_THREAD                                                     
endif

DHDOFILES = aiutils.o bcmsdh_sdmmc_linux.o dhd_linux.o siutils.o bcmutils.o   \
	dhd_linux_sched.o bcmwifi.o dhd_sdio.o bcmevent.o dhd_bta.o hndpmu.o  \
	bcmsdh.o dhd_cdc.o bcmsdh_linux.o dhd_common.o linux_osl.o            \
	bcmsdh_sdmmc.o dhd_custom_gpio.o sbutils.o wldev_common.o wl_android.o

# Uncomment to enable firmware dongle debug messages.
# DHDCFLAGS += -DDHD_DEBUG -DSRCBASE=\"$(SRCBASE)\"

obj-$(CONFIG_BCMDHD) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)
ifneq ($(CONFIG_WIRELESS_EXT),)
bcmdhd-objs += wl_iw.o
DHDCFLAGS += -DSOFTAP
endif

# Use CFG80211 for Android ICS only.
### Until if fix bmake, this is on so ICS gets WiFi.
###ifneq ($(CONFIG_ANDROID_CFG80211),)
ifneq ($(CONFIG_CFG80211),)
bcmdhd-objs += wl_cfg80211.o wl_cfgp2p.o dhd_linux_mon.o
DHDCFLAGS += -DWL_CFG80211 -DWLP2P
endif
###endif 

EXTRA_CFLAGS = $(DHDCFLAGS)

# Android needs CSCAN to update AP list.
ifeq ($(CONFIG_ANDROID),y)
DHDCFLAGS += -DCSCAN
endif

ifeq ($(CONFIG_BCMDHD),m)
EXTRA_LDFLAGS += --strip-debug
endif
