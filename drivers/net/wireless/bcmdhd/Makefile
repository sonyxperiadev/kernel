# bcmdhd
#	-DDHDTHREAD -DDHD_GPL -DDHD_SCHED -DDHD_DEBUG -DSDTEST -DBDC -DTOE    \
#	-DDHD_BCMEVENTS -DSHOW_EVENTS -DDONGLEOVERLAYS -DBCMDBG               \
#	-DCUSTOMER_HW2 -DOOB_INTR_ONLY -DHW_OOB       \
#	-DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DWLP2P     \
#	-DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT          \
#	-DKEEP_ALIVE -DCSCAN -DGET_CUSTOM_MAC_ENABLE -DPKT_FILTER_SUPPORT     \
#	-DEMBEDDED_PLATFORM -DENABLE_INSMOD_NO_FW_LOAD -DPNO_SUPPORT          \

DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER                     \
	-DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DBCMFILEIMAGE            \
	-DDHDTHREAD -DDHD_DEBUG -DSDTEST -DBDC -DTOE                          \
	-DDHD_BCMEVENTS -DSHOW_EVENTS -DPROP_TXSTATUS -DBCMDBG                \
	-DCUSTOMER_HW2 -DOOB_INTR_ONLY -DHW_OOB                               \
	-DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DWLP2P     \
	-DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT                                \
	-DKEEP_ALIVE -DGET_CUSTOM_MAC_ENABLE -DPKT_FILTER_SUPPORT             \
	-DEMBEDDED_PLATFORM -DENABLE_INSMOD_NO_FW_LOAD -DPNO_SUPPORT          \
	-Idrivers/net/wireless/bcmdhd -Idrivers/net/wireless/bcmdhd/include


# For Static Buffer
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DCONFIG_DHD_USE_STATIC_BUF
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

ifeq ($(CONFIG_MACH_RHEA_STONE_EDN2X),y)
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
	dhd_linux_sched.o bcmwifi.o dhd_sdio.o bcmwifi_channels.o bcmevent.o hndpmu.o   \
	bcmsdh.o dhd_cdc.o bcmsdh_linux.o dhd_common.o linux_osl.o            \
	bcmsdh_sdmmc.o dhd_custom_gpio.o sbutils.o wldev_common.o wl_android.o \
	dhd_cfg80211.o

obj-$(CONFIG_BCMDHD) += bcmdhd.o
bcmdhd-objs += $(DHDOFILES)
ifneq ($(CONFIG_WIRELESS_EXT),)
bcmdhd-objs += wl_iw.o
DHDCFLAGS += -DSOFTAP -DUSE_IW
endif
ifneq ($(CONFIG_CFG80211),)
bcmdhd-objs += wl_cfg80211.o wl_cfgp2p.o wl_linux_mon.o dhd_cfg80211.o
DHDCFLAGS += -DWL_CFG80211 -DWL_CFG80211_STA_EVENT -DWL_ENABLE_P2P_IF
endif
EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_BCMDHD),m)
EXTRA_LDFLAGS += --strip-debug
endif
