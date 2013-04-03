
# Kernel makefile for motions sensors
#
#

# MPU
obj-$(CONFIG_SENSORS_MPU3050)	+= mpu3050.o
obj-$(CONFIG_MPU_SENSORS_TIMERIRQ)+= timerirq.o
mpu3050-objs += mpuirq.o \
	slaveirq.o \
	mpu-dev.o \
	mpu-i2c.o \
	mlsl-kernel.o \
	mlos-kernel.o \
	$(MLLITE_DIR)mldl_cfg.o

#
# Accel options
#
ifdef CONFIG_SENSORS_BMA250
mpu3050-objs += $(MLLITE_DIR)accel/bma250.o
endif

#
# Compass options
#
ifdef CONFIG_SENSORS_AKM8975
mpu3050-objs += $(MLLITE_DIR)compass/ak8975.o
endif

ifdef CONFIG_SENSORS_AKM8972
mpu3050-objs += $(MLLITE_DIR)compass/ak8972.o
endif

ifdef CONFIG_SENSORS_AKM8963
mpu3050-objs += $(MLLITE_DIR)compass/ak89xx.o
endif

EXTRA_CFLAGS += -I$(M)/$(MLLITE_DIR) \
                -I$(M)/../../include \
		-Idrivers/misc/mpu3050 \
                -Iinclude/linux

