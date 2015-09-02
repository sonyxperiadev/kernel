obj-y += mhl_platform_base.o
obj-y += mhl_platform_power.o

obj-y +=  mhl_sii8620_8061_drv.o

mhl_sii8620_8061_drv-objs := mhl_sii8620_8061_device.o mhl_sii8620_device_edid.o  mhl_platform.o  mhl_tx.o mhl_platform_i2c.o mhl_lib_edid.o mhl_cbus_control.o mhl_tx_rcp.o mhl_lib_infoframe.o mhl_lib_timer.o mhl_cbus_write_burst.o

#Make ko object for development
debug: mhl_sii8620_8061_device.c mhl_sii8620_device_edid.c mhl_platform.c mhl_tx.c mhl_platform_i2c.c mhl_lib_edid.c mhl_cbus_control.c mhl_tx_rcp.c mhl_lib_infoframe.c mhl_lib_timer.c mhl_cbus_write_burst.c
	$(MAKE) -C ${KERNELPATH} M=$(PWD) modules
clean:
	$(MAKE) -C ${KERNELPATH} M=$(PWD) clean

