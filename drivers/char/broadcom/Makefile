
EXTRA_CFLAGS := -fno-inline -Wno-multichar -O0

obj-y += v3d/
obj-y += fuse_log/
obj-y += modem/
obj-y += isp/
obj-y += unicam/
obj-$(CONFIG_BCM_GPS)    	 += gps.o
obj-$(CONFIG_GPIO_USER_MODE_SUPPORT)	+= user-gpio.o
obj-$(CONFIG_BCM_HALAUDIO_CORE)        += halaudio/
obj-$(CONFIG_BCM_HALAUDIO)             += halaudio/ halaudio_drivers/
obj-$(CONFIG_BCM_AMXR)                 += amxr_drv.o
obj-$(CONFIG_BCM_AMXR)                 += amxr_core/
obj-$(CONFIG_BCM_AAA)                  += aaa.o

