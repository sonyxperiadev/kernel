#
# Makefile for industrial I/O Magnetometer sensors
#

obj-$(CONFIG_SENSORS_AK8975)	+= ak8975.o
obj-$(CONFIG_SENSORS_HMC5843)	+= hmc5843.o
obj-$(CONFIG_SENSORS_YAS532) += yas532.o
CFLAGS_yas532.o += -Idrivers/staging/iio
obj-$(CONFIG_SENSORS_BMA250E) += bma250e.o
CFLAGS_bma250e.o += -Idrivers/staging/iio
obj-$(CONFIG_SENSORS_KXTJ2) +=kxtj2.o
CFLAGS_kxtj2.o += -Idrivers/staging/iio