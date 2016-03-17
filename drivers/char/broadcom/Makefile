
EXTRA_CFLAGS := -fno-inline -Wno-multichar -O2

obj-$(CONFIG_HAWAII_MM) += mm/
obj-$(CONFIG_V3D) += v3d/
obj-$(CONFIG_MMDMA) += mmdma/
obj-$(CONFIG_VCE) += vce/
obj-y += fuse_log/
obj-$(CONFIG_BCM_MODEM) += modem/
obj-$(CONFIG_ISP) += isp/
obj-$(CONFIG_ISP2) += isp2/
obj-$(CONFIG_JPEGENC) += jpegenc/
obj-y += h264/
obj-y += unicam/
obj-$(CONFIG_BCM_MMUTIL) += mmutil/
obj-$(CONFIG_GPS_IRQ)		 += brcm_gps_irq.o
obj-$(CONFIG_BCM_FRAME_PROFILER_SUPPORT)   += frame_profiler.o
obj-$(CONFIG_BCM_IDLE_PROFILER)		       += idle_profiler/
obj-$(CONFIG_BCM_BSC)                      += bsc.o
obj-$(CONFIG_BCM_HEADSET_SW)               += headset.o

ifdef CONFIG_RHEA_PANDA
obj-$(CONFIG_BCM_MDMGPIO)              += mdmgpio/

endif

obj-$(CONFIG_BCM_RTC_CAL)           += rtc_cal/
obj-$(CONFIG_BCM_GPS_CNTIN) += gps/
obj-$(CONFIG_BCM_LOGRAM_HELPER) += logram.o
obj-$(CONFIG_BCM_MMS_BLOCK) += mmsblock/
obj-$(CONFIG_BCM_EMAIL_BLOCK) += emailblock/
obj-$(CONFIG_BCM_TRACE_LOG) += trace_log.o
