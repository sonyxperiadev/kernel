# BCMDHD

DHDCFLAGS = -Dlinux -DLINUX -DBCMDRIVER -DBCMDONGLEHOST -DDHDTHREAD
DHDCFLAGS += -DUNRELEASEDCHIP -DBCMDMA32 -DPROP_TXSTATUS -DDHD_SCHED
DHDCFLAGS += -DDHD_DEBUG -DBDC -DTOE -DDHD_BCMEVENTS -DSHOW_EVENTS
DHDCFLAGS += -DBCMDBG -DBCMSDIO -DDHD_GPL -DBCMLXSDMMC -DBCMPLATFORM_BUS
DHDCFLAGS += -DMMC_SDIO_ABORT -DDHD_SDALIGN=64 -DMAX_HDR_READ=64 -DDHD_FIRSTREAD=64
DHDCFLAGS += -DEMBEDDED_PLATFORM -DSOFTAP -DCSCAN -DARP_OFFLOAD_SUPPORT
DHDCFLAGS += -DPKT_FILTER_SUPPORT -DKEEP_ALIVE -DPNO_SUPPORT
DHDCFLAGS += -Wall -Wstrict-prototypes -Werror
DHDCFLAGS += -DCUSTOMER_HW3
#DHDCFLAGS += -DCONFIG_WIFI_CONTROL_FUNC -DGET_CUSTOM_MAC_ENABLE
#DHDCFLAGS += -DWLBTAMP -DWLP2P	-DNEW_COMPAT_WIRELESS -DWIFI_ACT_FRAME

# In-Band Interrupt
DHDCFLAGS += -DSDIO_ISR_THREAD 

# HW-OOB Interrupt
#DHDCFLAGS += -DOOB_INTR_ONLY -DHW_OOB 

#rhearay EDN1X
#ifeq ($(CONFIG_MACH_RHEA_RAY_EDN1X),y)
#DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=85
#endif

#RheaBerri EDNxx
#ifeq ($(CONFIG_MACH_RHEA_BERRI),y)
#DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=1
#endif

#RheaRay  
#ifeq ($(CONFIG_MACH_RHEA_RAY),y)
#DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=85
#endif

#RheaBerri EDN1X
#ifeq ($(CONFIG_MACH_RHEA_BERRI),y)
#DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=1
#endif

#Island Stone
#ifeq ($(CONFIG_MACH_ISLAND_FF),y)
#DHDCFLAGS += -DCUSTOM_OOB_GPIO_NUM=179
#endif
  
  


# SW-OOB Interrupt
#DHDCFLAGS += -DOOB_INTR_ONLY -DCUSTOM_OOB_GPIO_NUM=85

DHDCFLAGS += \
	-Idrivers/net/wireless/bcm4329_falcon/dhd/sys \
	-Idrivers/net/wireless/bcm4329_falcon/bcmsdio/sys \
	-Idrivers/net/wireless/bcm4329_falcon/shared \
	-Idrivers/net/wireless/bcm4329_falcon/dongle \
	-Idrivers/net/wireless/bcm4329_falcon/wl/exe \
	-Idrivers/net/wireless/bcm4329_falcon/wl/sys \
	-Idrivers/net/wireless/bcm4329_falcon/include

#------------------------------------------------------------------------

DHDO_BCMSDIO_SYS = \
	bcmsdio/sys/bcmsdh.o bcmsdio/sys/bcmsdh_linux.o \
	bcmsdio/sys/bcmsdh_sdmmc.o bcmsdio/sys/bcmsdh_sdmmc_linux.o

DHDO_DHD_SYS = \
	dhd/sys/dhd_bta.o dhd/sys/dhd_cdc.o \
	dhd/sys/dhd_common.o dhd/sys/dhd_custom_gpio.o \
	dhd/sys/dhd_linux.o dhd/sys/dhd_linux_sched.o \
	dhd/sys/dhd_sdio.o

DHDO_SHARED = \
	shared/aiutils.o shared/bcmevent.o \
	shared/bcmutils.o shared/bcmwifi.o \
	shared/hndpmu.o shared/linux_osl.o \
	shared/sbutils.o shared/siutils.o

DHDO_WL_SYS = wl/sys/wl_iw.o wl/sys/wl_android.o wl/sys/wldev_common.o

DHDOFILES = $(DHDO_BCMSDIO_SYS)
DHDOFILES += $(DHDO_DHD_SYS)
DHDOFILES += $(DHDO_SHARED)
DHDOFILES += $(DHDO_WL_SYS)





obj-$(CONFIG_BCM4329_FALCON) += bcm4329_falcon.o
bcm4329_falcon-objs += $(DHDOFILES)
EXTRA_CFLAGS = $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug


obj-$(CONFIG_BCM4330_FALCON) += bcm4330_falcon.o
bcm4330_falcon-objs += $(DHDOFILES)
EXTRA_CFLAGS = $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug



