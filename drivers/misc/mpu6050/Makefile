# Kernel makefile for motions sensors
#
#

# MPU
obj-$(CONFIG_SENSORS_MPU6050) += mpu6050.o

mpu6050-objs += mpuirq.o \
	slaveirq.o \
	mpu-dev.o \
	mlsl-kernel.o \
	mldl_cfg.o \
	mldl_print_cfg.o \
	accel/mpu6050.o

EXTRA_CFLAGS += -Idrivers/misc/mpu6050
EXTRA_CFLAGS += -D__C99_DESIGNATED_INITIALIZER
EXTRA_CFLAGS += -DINV_CACHE_DMP=1

obj-$(CONFIG_MPU_SENSORS_TIMERIRQ)+= timerirq.o

ifdef CONFIG_MPU_SENSORS_AK8972
mpu6050-objs +=compass/ak8972.o
endif
ifdef CONFIG_MPU_SENSORS_AK8963
mpu6050-objs +=compass/ak89xx.o
endif
