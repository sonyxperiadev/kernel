
# Kernel makefile for motions sensors
#
# 

# MPU
obj-$(CONFIG_INV_SENSORS)+=
ifdef CONFIG_MPU_SENSORS_MPU3050
INV_MODULE_NAME := mpu3050
endif

ifdef CONFIG_MPU_SENSORS_MPU6050A2
INV_MODULE_NAME := mpu6050
else
ifdef CONFIG_MPU_SENSORS_MPU6050B1
INV_MODULE_NAME := mpu6050
endif
endif

obj-$(CONFIG_INV_SENSORS)	+= $(INV_MODULE_NAME).o

$(INV_MODULE_NAME)-objs += mpuirq.o \
	slaveirq.o \
	mpu-dev.o \
	mpu-i2c.o \
	mlsl-kernel.o \
	mlos-kernel.o \
	$(MLLITE_DIR)mldl_cfg.o

#
# Accel options
#
ifdef CONFIG_MPU_SENSORS_ADXL34X
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/adxl34x.o
endif

ifdef CONFIG_MPU_SENSORS_BMA150
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/bma150.o
endif

ifdef CONFIG_MPU_SENSORS_BMA222
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/bma222.o
endif

ifdef CONFIG_MPU_SENSORS_BMA250
mpu3050-objs += $(MLLITE_DIR)accel/bma250.o
endif

ifdef CONFIG_MPU_SENSORS_KXSD9
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/kxsd9.o
endif

ifdef CONFIG_MPU_SENSORS_KXTF9
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/kxtf9.o
endif

ifdef CONFIG_MPU_SENSORS_LIS331DLH
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/lis331.o
endif

ifdef CONFIG_MPU_SENSORS_LIS3DH
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/lis3dh.o
endif

ifdef CONFIG_MPU_SENSORS_LSM303DLH_A
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/lsm303_a.o
endif

ifdef CONFIG_MPU_SENSORS_LSM303DLM_A
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/lsm303_a.o
endif

ifdef CONFIG_MPU_SENSORS_MMA8450
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/mma8450.o
endif

ifdef CONFIG_MPU_SENSORS_MMA845X
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/mma845x.o
endif

ifeq ($(INV_MODULE_NAME), mpu6050)
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)accel/mantis.o
endif

#
# Compass options
#
ifdef CONFIG_MPU_SENSORS_AK8975
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/ak8975.o
endif

ifdef CONFIG_MPU_SENSORS_AMI30X
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/ami30x.o
endif

ifdef CONFIG_MPU_SENSORS_AMI306
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/ami306.o
endif

ifdef CONFIG_MPU_SENSORS_HMC5883
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/hmc5883.o
endif

ifdef CONFIG_MPU_SENSORS_LSM303DLH_M
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/lsm303dlh_m.o
endif

ifdef CONFIG_MPU_SENSORS_LSM303DLM_M
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/lsm303dlm_m.o
endif

ifdef CONFIG_MPU_SENSORS_MMC314X
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/mmc314x.o
endif

ifdef CONFIG_MPU_SENSORS_YAS529
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/yas529-kernel.o
endif

ifdef CONFIG_MPU_SENSORS_YAS530
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/yas530.o
endif

ifdef CONFIG_MPU_SENSORS_HSCDTD002B
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/hscdtd002b.o
endif

ifdef CONFIG_MPU_SENSORS_HSCDTD004A
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)compass/hscdtd004a.o
endif
#
# Pressure options
#
ifdef CONFIG_MPU_SENSORS_BMA085
$(INV_MODULE_NAME)-objs += $(MLLITE_DIR)pressure/bma085.o
endif

EXTRA_CFLAGS += -I$(M)/$(MLLITE_DIR) \
                -I$(M)/../../include \
                -Idrivers/misc/mpu3050 \
                -Iinclude/linux

obj-$(CONFIG_MPU_SENSORS_TIMERIRQ)+= timerirq.o

