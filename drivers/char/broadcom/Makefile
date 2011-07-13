
EXTRA_CFLAGS := -fno-inline -Wno-multichar -O0

obj-y += v3d/
obj-y += vce/
ifneq ($(CONFIG_MACH_RHEA_RAY_EDN1X),y)
	obj-y += fuse_log/
	obj-y += modem/
endif
obj-y += isp/
obj-y += unicam/
obj-$(CONFIG_BCM_GPS)    	 += gps.o
obj-$(CONFIG_GPIO_USER_MODE_SUPPORT)	+= user-gpio.o
