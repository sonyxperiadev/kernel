ccflags-y := -I$(srctree)/drivers/devfreq/ \
	-I$(srctree)/drivers/media/platform/msm/vidc_3x/ \
        -I$(srctree)/drivers/media/platform/msm/vidc_3x/governors/

msm-vidc-dyn-gov-objs := msm_vidc_dyn_gov.o

msm-vidc-table-gov-objs := msm_vidc_table_gov.o

obj-$(CONFIG_MSM_VIDC_3X_GOVERNORS) := msm-vidc-dyn-gov.o msm-vidc-table-gov.o
