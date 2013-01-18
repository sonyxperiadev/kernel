#ifndef __MACH_HAWAII_DEVICE_H
#define __MACH_HAWAII_DEVICE_H

void __init hawaii_add_common_devices(void);
void __init hawaii_reserve(void);

#ifdef CONFIG_KONA_AVS
extern u32 svt_pmos_bin[3 + 1];
extern u32 svt_nmos_bin[3 + 1];
extern u32 lvt_pmos_bin[3 + 1];
extern u32 lvt_nmos_bin[3 + 1];
extern u32 svt_silicon_type_lut[3 * 3];
extern u32 lvt_silicon_type_lut[3 * 3];
#endif
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
extern struct platform_device kona_avs_device;
#endif
#ifdef CONFIG_KONA_MEMC
extern struct platform_device kona_memc_device;
#endif
#ifdef CONFIG_KONA_TMON
extern struct platform_device kona_tmon_device;
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
#ifdef CONFIG_ANDROID_PMEM
extern struct android_pmem_platform_data android_pmem_data;
extern struct platform_device android_pmem;
#endif
#ifdef CONFIG_ION
extern struct platform_device ion_system_device;
extern struct ion_platform_data ion_carveout_data;
extern struct platform_device ion_carveout_device;
#ifdef CONFIG_CMA
extern struct ion_platform_data ion_cma_data;
extern struct platform_device ion_cma_device;
#endif
#endif

#endif /* __MACH_HAWAII_DEVICE_H */
