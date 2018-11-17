#
# Makefile for the unified DRM display driver for SoMC Open Devices
#
ccflags-y := -Iinclude/drm -Idrivers/gpu/drm/msm -Idrivers/gpu/drm/msm/dsi-staging -Idrivers/gpu/drm/msm/dp

somc_panel-$(CONFIG_DRM_MSM_DSI_SOMC_PANEL)	+= common.o
somc_panel-$(CONFIG_DRM_MSM_DSI_SOMC_PANEL)	+= dsi_panel_driver.o
somc_panel-$(CONFIG_DRM_MSM_DSI_SOMC_PANEL)	+= panel_detect.o
#somc_panel-$(CONFIG_DRM_MSM_DSI_SOMC_PANEL)	+= panel_polling.o
somc_panel-$(CONFIG_DRM_MSM_DSI_SOMC_PANEL)	+= panel_fps_manager.o
somc_panel-$(CONFIG_DRM_MSM_DSI_SOMC_PANEL)	+= panel_color_manager.o panel_adaptive_color.o
somc_panel-$(CONFIG_SOMC_PANEL_LABIBB)		+= panel_regulators.o
somc_panel-$(CONFIG_SOMC_PANEL_INCELL)		+= incell.o

obj-$(CONFIG_DRM_MSM_DSI_SOMC_PANEL)	+= somc_panel.o
