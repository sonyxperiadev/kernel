#
# Makefile for Synaptics TCM
#

synaptics_tcm-y += synaptics_tcm_core.o
synaptics_tcm-y += synaptics_tcm_device.o
synaptics_tcm-y += synaptics_tcm_i2c.o
synaptics_tcm-y += synaptics_tcm_touch.o

obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_TCM_FWUPG)	+= synaptics_tcm_reflash.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_TCM_DIAG)	+= synaptics_tcm_diagnostics.o
obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_TCM_TST)	+= synaptics_tcm_testing.o

obj-$(CONFIG_TOUCHSCREEN_SYNAPTICS_TCM) += synaptics_tcm.o
