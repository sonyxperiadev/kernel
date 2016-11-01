/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __MSM_CLOCKS_8916_H
#define __MSM_CLOCKS_8916_H

/* GPLLs */
#define clk_gpll0_clk_src			0x5933b69f
#define clk_gpll0_ao_clk_src			0x6b2fb034
#define clk_gpll1_clk_src			0x916f8847
#define clk_gpll2_clk_src			0x7c34503b

/* SR2PLL */
#define clk_a53sspll				0xf761da94

/* SRCs */
#define clk_apss_ahb_clk_src			0x36f8495f
#define clk_blsp1_qup1_i2c_apps_clk_src		0x17f78f5e
#define clk_blsp1_qup1_spi_apps_clk_src		0xf534c4fa
#define clk_blsp1_qup2_i2c_apps_clk_src		0x8de71c79
#define clk_blsp1_qup2_spi_apps_clk_src		0x33cf809a
#define clk_blsp1_qup3_i2c_apps_clk_src		0xf161b902
#define clk_blsp1_qup3_spi_apps_clk_src		0x5e95683f
#define clk_blsp1_qup4_i2c_apps_clk_src		0xb2ecce68
#define clk_blsp1_qup4_spi_apps_clk_src		0xddb5bbdb
#define clk_blsp1_qup5_i2c_apps_clk_src		0x71ea7804
#define clk_blsp1_qup5_spi_apps_clk_src		0x9752f35f
#define clk_blsp1_qup6_i2c_apps_clk_src		0x28806803
#define clk_blsp1_qup6_spi_apps_clk_src		0x44a1edc4
#define clk_blsp1_uart1_apps_clk_src		0xf8146114
#define clk_blsp1_uart2_apps_clk_src		0xfc9c2f73
#define clk_byte0_clk_src			0x75cc885b
#define clk_cci_clk_src				0x822f3d97
#define clk_cpp_clk_src				0x8382f56d
#define clk_camss_ahb_clk_src			0xa68afe9c
#define clk_camss_gp0_clk_src			0x43b063e9
#define clk_camss_gp1_clk_src			0xa3315f1b
#define clk_crypto_clk_src			0x37a21414
#define clk_csi0_clk_src			0x227e65bc
#define clk_csi1_clk_src			0x6a2a6c36
#define clk_csi0phytimer_clk_src		0xc8a309be
#define clk_csi1phytimer_clk_src		0x7c0fe23a
#define clk_esc0_clk_src			0xb41d7c38
#define clk_gfx3d_clk_src			0x917f76ef
#define clk_gp1_clk_src				0xad85b97a
#define clk_gp2_clk_src				0xfb1f0065
#define clk_gp3_clk_src				0x63b693d6
#define clk_jpeg0_clk_src			0x9a0a0ac3
#define clk_mdp_clk_src				0x6dc1f8f1
#define clk_mclk0_clk_src			0x266b3853
#define clk_mclk1_clk_src			0xa73cad0c
#define clk_pclk0_clk_src			0xccac1f35
#define clk_pdm2_clk_src			0x31e494fd
#define clk_sdcc1_apps_clk_src			0xd4975db2
#define clk_sdcc2_apps_clk_src			0xfc46c821
#define clk_usb_hs_system_clk_src		0x28385546
#define clk_vsync_clk_src			0xecb43940
#define clk_vfe0_clk_src			0xa0c2bd8f
#define clk_vcodec0_clk_src			0xbc193019

/* BRANCHEs*/
#define clk_gcc_blsp1_ahb_clk			0x8caa5b4f
#define clk_gcc_boot_rom_ahb_clk		0xde2adeb1
#define clk_gcc_crypto_ahb_clk			0x94de4919
#define clk_gcc_crypto_axi_clk			0xd4415c9b
#define clk_gcc_crypto_clk			0x00d390d2
#define clk_gcc_prng_ahb_clk			0x397e7eaa
#define clk_gcc_apss_tcu_clk			0xaf56a329
#define clk_gcc_gfx_tbu_clk			0x18bb9a90
#define clk_gcc_gtcu_ahb_clk			0xb432168e
#define clk_gcc_jpeg_tbu_clk			0xcf8fd944
#define clk_gcc_mdp_tbu_clk			0x82287f76
#define clk_gcc_smmu_cfg_clk			0x75eaefa5
#define clk_gcc_venus_tbu_clk			0x7e0b97ce
#define clk_gcc_vfe_tbu_clk			0x061f2f95
#define clk_gcc_blsp1_qup1_i2c_apps_clk		0xc303fae9
#define clk_gcc_blsp1_qup1_spi_apps_clk		0x759a76b0
#define clk_gcc_blsp1_qup2_i2c_apps_clk		0x1076f220
#define clk_gcc_blsp1_qup2_spi_apps_clk		0x3e77d48f
#define clk_gcc_blsp1_qup3_i2c_apps_clk		0x9e25ac82
#define clk_gcc_blsp1_qup3_spi_apps_clk		0xfb978880
#define clk_gcc_blsp1_qup4_i2c_apps_clk		0xd7f40f6f
#define clk_gcc_blsp1_qup4_spi_apps_clk		0x80f8722f
#define clk_gcc_blsp1_qup5_i2c_apps_clk		0xacae5604
#define clk_gcc_blsp1_qup5_spi_apps_clk		0xbf3e15d7
#define clk_gcc_blsp1_qup6_i2c_apps_clk		0x5c6ad820
#define clk_gcc_blsp1_qup6_spi_apps_clk		0x780d9f85
#define clk_gcc_blsp1_uart1_apps_clk		0xc7c62f90
#define clk_gcc_blsp1_uart2_apps_clk		0xf8a61c96
#define clk_gcc_camss_cci_ahb_clk		0xa81c11ba
#define clk_gcc_camss_cci_clk			0xb7dd8824
#define clk_gcc_camss_csi0_ahb_clk		0x175d672a
#define clk_gcc_camss_csi0_clk			0x6b01b3e1
#define clk_gcc_camss_csi0phy_clk		0x06a41ff7
#define clk_gcc_camss_csi0pix_clk		0x61a8a930
#define clk_gcc_camss_csi0rdi_clk		0x7053c7ae
#define clk_gcc_camss_csi1_ahb_clk		0x2c2dc261
#define clk_gcc_camss_csi1_clk			0x1aba4a8c
#define clk_gcc_camss_csi1phy_clk		0x0fd1d1fa
#define clk_gcc_camss_csi1pix_clk		0x87fc98d8
#define clk_gcc_camss_csi1rdi_clk		0x6ac996fe
#define clk_gcc_camss_csi_vfe0_clk		0xcc73453c
#define clk_gcc_camss_gp0_clk			0xd2bc3892
#define clk_gcc_camss_gp1_clk			0xe4c013e1
#define clk_gcc_camss_ispif_ahb_clk		0x3c0a858f
#define clk_gcc_camss_jpeg0_clk			0x1ed3f032
#define clk_gcc_camss_jpeg_ahb_clk		0x3bfa7603
#define clk_gcc_camss_jpeg_axi_clk		0x3e278896
#define clk_gcc_camss_mclk0_clk			0x80902deb
#define clk_gcc_camss_mclk1_clk			0x5002d85f
#define clk_gcc_camss_micro_ahb_clk		0xfbbee8cf
#define clk_gcc_camss_csi0phytimer_clk		0xf8897589
#define clk_gcc_camss_csi1phytimer_clk		0x4d26438f
#define clk_gcc_camss_ahb_clk			0x9894b414
#define clk_gcc_camss_top_ahb_clk		0x4e814a78
#define clk_gcc_camss_cpp_ahb_clk		0x4ac95e14
#define clk_gcc_camss_cpp_clk			0x7118a0de
#define clk_gcc_camss_vfe0_clk			0xaaa3cd97
#define clk_gcc_camss_vfe_ahb_clk		0x4050f47a
#define clk_gcc_camss_vfe_axi_clk		0x77fe2384
#define clk_gcc_oxili_gmem_clk			0x5620913a
#define clk_gcc_gp1_clk				0x057f7b69
#define clk_gcc_gp2_clk				0x9bf83ffd
#define clk_gcc_gp3_clk				0xec6539ee
#define clk_gcc_mdss_ahb_clk			0xbfb92ed3
#define clk_gcc_mdss_axi_clk			0x668f51de
#define clk_gcc_mdss_byte0_clk			0x35da7862
#define clk_gcc_mdss_esc0_clk			0xaec5cb25
#define clk_gcc_mdss_mdp_clk			0x22f3521f
#define clk_gcc_mdss_pclk0_clk			0xcc5c5c77
#define clk_gcc_mdss_vsync_clk			0x32a09f1f
#define clk_gcc_mss_cfg_ahb_clk			0x111cde81
#define clk_gcc_mss_q6_bimc_axi_clk		0x67544d62
#define clk_gcc_oxili_ahb_clk			0xd15c8a00
#define clk_gcc_oxili_gfx3d_clk			0x49a51fd9
#define clk_gcc_pdm2_clk			0x99d55711
#define clk_gcc_pdm_ahb_clk			0x365664f6
#define clk_gcc_sdcc1_ahb_clk			0x691e0caa
#define clk_gcc_sdcc1_apps_clk			0x9ad6fb96
#define clk_gcc_sdcc2_ahb_clk			0x23d5727f
#define clk_gcc_sdcc2_apps_clk			0x861b20ac
#define clk_gcc_usb2a_phy_sleep_clk		0x6caa736f
#define clk_gcc_usb_hs_ahb_clk			0x72ce8032
#define clk_gcc_usb_hs_system_clk		0xa11972e5
#define clk_gcc_venus0_ahb_clk			0x08d778c6
#define clk_gcc_venus0_axi_clk			0xcdf4c8f6
#define clk_gcc_venus0_vcodec0_clk		0xf76a02bb
#define clk_gcc_gfx_tcu_clk			0x59505e55
#define clk_gcc_gtcu_ahb_bridge_clk		0x19d2c5fe
#define clk_gcc_bimc_gpu_clk			0x19922503
#define clk_gcc_bimc_gfx_clk			0x3edd69ad
#define clk_gcc_snoc_qosgen_clk			0x37d40ce2

#define clk_pixel_clk_src			0x8b6f83d8
#define clk_byte_clk_src			0x3a911c53

/* RPM */
#define clk_pcnoc_clk				0xc1296d0f
#define clk_pcnoc_a_clk				0x9bcffee4
#define clk_pcnoc_msmbus_clk			0x2b53b688
#define clk_pcnoc_msmbus_a_clk			0x9753a54f
#define clk_pcnoc_keepalive_a_clk		0x9464f720
#define clk_pcnoc_sps_clk			0x23d3f584
#define clk_pcnoc_usb_a_clk			0x11d6a74e
#define clk_snoc_clk				0x2c341aa0
#define clk_snoc_a_clk				0x8fcef2af
#define clk_snoc_msmbus_clk			0xe6900bb6
#define clk_snoc_msmbus_a_clk			0x5d4683bd
#define clk_snoc_mmnoc_axi_clk			0xfedd4bd5
#define clk_snoc_mmnoc_ahb_clk			0xd2149dbb
#define clk_snoc_usb_a_clk			0x34b7821b
#define clk_bimc_clk				0x4b80bf00
#define clk_bimc_a_clk				0x4b25668a
#define clk_bimc_acpu_a_clk			0x4446311b
#define clk_bimc_msmbus_clk			0xd212feea
#define clk_bimc_msmbus_a_clk			0x71d1a499
#define clk_bimc_usb_a_clk			0xea410834
#define clk_qdss_clk				0x1492202a
#define clk_qdss_a_clk				0xdd121669
#define clk_xo_clk_src				0x23f5649f
#define clk_xo_a_clk_src			0x2fdd2c7c
#define clk_xo_otg_clk				0x79bca5cc
#define clk_xo_a2				0xeba5a83d
#define clk_xo_dwc3_clk				0xfad488ce
#define clk_xo_ehci_host_clk			0xc7c340b1
#define clk_xo_lpm_clk				0x2be48257
#define clk_xo_pil_mss_clk			0xe97a8354
#define clk_xo_pil_pronto_clk			0x89dae6d0
#define clk_xo_wlan_clk				0x0116b76f

#define clk_bb_clk1				0xf5304268
#define clk_bb_clk1_pin				0x6dd0a779
#define clk_bb_clk2				0xfe15cb87
#define clk_bb_clk2_pin				0x498938e5
#define clk_rf_clk1				0xaabeea5a
#define clk_rf_clk1_pin				0x8f463562
#define clk_rf_clk2				0x24a30992
#define clk_rf_clk2_pin				0xa7c5602a

/* DEBUG */
#define clk_gcc_debug_mux			0x8121ac15
#define clk_rpm_debug_mux			0x25cd1f3a
#define clk_wcnss_m_clk				0x709f430b
#define clk_apss_debug_pri_mux			0xc691ff55
#define clk_apss_debug_sec_mux			0xc0b680f9
#define clk_apss_debug_ter_mux			0x32041c48
#define clk_apc0_m_clk				0xce1e9473
#define clk_apc1_m_clk				0x990fbaf7
#define clk_apc2_m_clk				0x252cd4ae
#define clk_apc3_m_clk				0x78c64486
#define clk_l2_m_clk				0x4bedf4d0


#endif
