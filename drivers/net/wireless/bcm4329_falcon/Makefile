# BCM4329

DHDCFLAGS = -DBCMWPA2 -DSDTEST -DDONGLEOVERLAYS \
	    -DLINUX -DBCMDRIVER -DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 \
	    -DEMBEDDED_PLATFORM -DARP_OFFLOAD_SUPPORT -DPKT_FILTER_SUPPORT \
	    -DSOFTAP -DBCMFILEIMAGE -DCONFIG_WIRELESS_EXT -DDHDTHREAD \
	    -DDHD_GPL -DDHD_SCHED -DBDC -DTOE -DDHD_BCMEVENTS \
	    -DSHOW_EVENTS -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS \
	    -DSDIO_ISR_THREAD -DWIFI_ACT_FRAME -DDEFAULT_HW \
	    -Wall -Wstrict-prototypes -Werror \
	    -DWLPFN -DWLCNT -DWLBTAMP -DRWL_SOCKET -DRWL_WIFI -DRWL_DONGLE \
	    -Idrivers/net/wireless/bcm4329_falcon -Idrivers/net/wireless/bcm4329_falcon/include

DHDOFILES = aiutils.o bcmevent.o bcmsdh_linux.o bcmsdh.o bcmsdh_sdmmc_linux.o \
	    bcmsdh_sdmmc.o bcmutils.o bcmwifi.o dhd_bta.o dhd_cdc.o \
	    dhd_common.o dhd_custom_gpio.o dhd_linux.o dhd_linux_sched.o \
	    dhd_sdio.o hndpmu.o linux_osl.o sbutils.o \
	    siutils.o wl_iw.o

obj-$(CONFIG_BCM4329_FALCON) += bcm4329_falcon.o
bcm4329_falcon-objs += $(DHDOFILES)
EXTRA_CFLAGS = $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug
