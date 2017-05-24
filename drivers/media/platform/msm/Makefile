#
# Makefile for the QCOM specific video device drivers
# based on V4L2.
#
obj-$(CONFIG_MSMB_CAMERA) += camera_v2/
obj-$(CONFIG_MSM_VIDC_V4L2) += vidc/
obj-y += sde/
obj-y += broadcast/
obj-$(CONFIG_DVB_MPQ) += dvb/
obj-$(CONFIG_MSM_AIS) += ais/

# TODO: remove me b/62057517
subdir-ccflags-y += \
	-Wno-parentheses-equality
