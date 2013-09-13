#ifndef __MACH_JAVA_DEVICE_H
#define __MACH_JAVA_DEVICE_H

void __init hawaii_add_common_devices(void);
void __init hawaii_reserve(void);
int configure_sdio_pullup(bool pull_up);

extern struct platform_device hawaii_serial_device;
extern struct platform_device hawaii_i2c_adap_devices[];
extern struct platform_device pmu_device;
extern struct platform_device hawaii_pwm_device;
extern struct platform_device hawaii_ssp0_device;
extern struct platform_device hawaii_ssp1_device;
extern struct platform_device hawaii_sdio1_device;
extern struct platform_device hawaii_sdio2_device;
extern struct platform_device hawaii_sdio3_device;
extern struct platform_device hawaii_kp_device;
extern struct platform_device hawaii_headset_device;
extern struct platform_device hawaii_pl330_dmac_device;
extern struct platform_device hawaii_backlight_device;
#ifdef CONFIG_BRCM_CDC
extern struct platform_device brcm_cdc_device;
#endif
#ifdef CONFIG_SENSORS_KONA
extern struct platform_device tmon_device;
extern struct platform_device thermal_device;
#endif
#ifdef CONFIG_STM_TRACE
extern struct platform_device hawaii_stm_device;
#endif
#ifdef CONFIG_HW_RANDOM_KONA
extern struct platform_device rng_device;
#endif
#ifdef CONFIG_USB_DWC_OTG
extern struct platform_device hawaii_hsotgctrl_platform_device;
extern struct platform_device hawaii_otg_platform_device;
extern struct platform_device hawaii_usb_phy_platform_device;
#endif
#ifdef CONFIG_KONA_AVS
extern struct platform_device avs_device;
#endif
#ifdef CONFIG_KONA_MEMC
extern struct platform_device kona_memc_device;
#endif
#ifdef CONFIG_KONA_TMON
extern struct platform_device kona_tmon_device;
#endif
#ifdef CONFIG_KONA_SECURE_MEMC
extern struct platform_device kona_secure_memc_device;
extern struct kona_secure_memc_pdata k_s_memc_plat_data;
#endif
#ifdef CONFIG_KONA_CPU_FREQ_DRV
extern struct platform_device kona_cpufreq_device;
#endif
#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH
extern struct platform_device hawaii_spum_device;
#endif
#ifdef CONFIG_CRYPTO_DEV_BRCM_SPUM_AES
extern struct platform_device hawaii_spum_aes_device;
#endif
#ifdef CONFIG_UNICAM
extern struct platform_device hawaii_unicam_device;
#endif
#ifdef CONFIG_VIDEO_UNICAM_CAMERA
extern struct platform_device hawaii_camera_device;
#endif
#ifdef CONFIG_SND_BCM_SOC
extern struct platform_device caph_i2s_device;
extern struct platform_device caph_pcm_device;
#endif
#ifdef CONFIG_ION_BCM_NO_DT
#ifdef CONFIG_IOMMU_API
extern struct platform_device iommu_mm_device;
#endif
#ifdef CONFIG_BCM_IOVMM
extern struct platform_device iovmm_mm_device;
extern struct platform_device iovmm_mm_256mb_device;
#endif
extern struct platform_device ion_system_device;
extern struct platform_device ion_system_extra_device;
extern struct ion_platform_data ion_carveout_data;
extern struct platform_device ion_carveout_device;
#ifdef CONFIG_CMA
extern struct ion_platform_data ion_cma_data;
extern struct platform_device ion_cma_device;
#endif /* CONFIG_CMA */
#if defined(CONFIG_MM_SECURE_DRIVER)
extern struct ion_platform_data ion_secure_data;
extern struct platform_device ion_secure_device;
#endif /* CONFIG_MM_SECURE_DRIVER */
#endif /* CONFIG_ION_BCM_NO_DT */

#ifdef CONFIG_MOBICORE_DRIVER
extern struct mobicore_data mobicore_plat_data;
extern struct platform_device mobicore_device;
#endif

#endif /* __MACH_JAVA_DEVICE_H */
