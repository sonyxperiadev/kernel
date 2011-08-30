CONFIG_BCM_VCP ?= n

$(info ++++++++++++++++++++++)
$(info CONFIG_BCM_VCP = $(CONFIG_BCM_VCP))

obj-$(CONFIG_BCM_VCP) += vcp.o

vcp-objs := vcpdrv.o vcpdrv_user.o

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order *~*
	-rm -rf .tmp_versions
