
/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
 * Unicam camera host exports
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/videobuf2-dma-contig.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>
#include <mach/rdb_A0/brcm_rdb_cam.h>
#include <mach/rdb/brcm_rdb_mm_cfg.h>
#include <mach/rdb/brcm_rdb_mm_clk_mgr_reg.h>


#define MM_CSI0_BASE 0x3C008000

enum host_mode {
	CSI2,
	CSI1CCP2,
	INVALID,
};

enum afe_num {
	AFE0,
	AFE1,
};

enum csi1ccp2_clock_mode {
	DATA_STROBE,
	DATA_CLOCK,
};

enum csi2_lanes {
	LANE_INVALID,
	CSI2_SINGLE_LANE,
	CSI2_DUAL_LANE,
};

enum buffer_type {
	IMAGE_BUFFER,
	DATA_BUFFER,
};

enum buffer_mode {
	BUFFER_SINGLE,
	BUFFER_DOUBLE,
	BUFFER_TRIGGER,
};

enum dpcm_encode_mode {
	DPCM_ENC_NONE,
	DPCM_10_8,
	DPCM_12_8,
	DPCM_14_8,
};

enum dpcm_decode_mode {
	DPCM_DEC_NONE,
	DPCM_8_10,
	DPCM_7_10,
	DPCM_6_10,
	DPCM_8_12,
	DPCM_7_12,
	DPCM_6_12,
	DPCM_10_14,
	DPCM_8_14,
	DPCM_12_16,
	DPCM_10_16,
	DPCM_8_16,
};

enum pix_pack_mode {
	PIX_PACK_NONE,
	PIX_PACK_8,
	PIX_PACK_10,
	PIX_PACK_12,
	PIX_PACK_14,
	PIX_PACK_16,
	PIX_PACK_RAW10,
	PIX_PACK_RAW12,
};

enum pix_unpack_mode {
	PIX_UNPACK_NONE,
	PIX_UNPACK_RAW6,
	PIX_UNPACK_RAW7,
	PIX_UNPACK_RAW8,
	PIX_UNPACK_RAW10,
	PIX_UNPACK_RAW12,
	PIX_UNPACK_RAW14,
	PIX_UNPACK_RAW16,
};

enum lane_trans {
	ULPS = 0x4,
	ULPS_REQ = 0x5,
	LANE_ERR = 0xd,
};

enum reset_type {
	RESET_RX,
	RESET_ANALOG,
};

struct int_desc {
	u8 fsi;
	u8 fei;
	u32 lci;
	u8 die;
	u32 dataline;
};

struct rx_stat_list {
	u8 syn;
	u8 clk_hs_present;
	u8 sbe;
	u8 pbe;
	u8 hoe;
	u8 ple;
	u8 ssc;
	u8 crce;
	u8 oes;
	u8 ifo;
	u8 ofo;
	u8 bfo;
	u8 dl;
	u8 ps;
	u8 is;
	u8 pi0;
	u8 pi1;
};

struct lane_timing {
	u32 hs_settle_time;
	u32 hs_term_time;
};

/* Not at all sure if single bit errors are corrected and reported
   Do we need to consider this as an error */
#define RX_CSI2_ERROR_MASK 0x3EBC
#define RX_CCP2_ERROR_MASK 0x3ECC

struct buffer_desc {
	u32 start;
	u32 size;
	u8 wrap_en;
	u32 ls;
};

/* Packet compare and capture */

/* This feature applies only to CSI2 */
/* Will scan all incoming packets and captures a packet with
   matching credentials. Used to debug and recieve if sensor/ISP
   sends generic short packets */

enum packet_comp_cap {
	COMP_CAP_INVALID,
	COMP_CAP_0,
	COMP_CAP_1,
};

struct packet_compare {
	bool enable;
	bool capture_header;
	bool gen_int;
	u8 vc;
	u8 dt;
};

struct packet_capture {
	bool valid;
	u8 ecc;
	u8 word_count;
	u8 vc;
	u8 dt;
};

/* MM_CSI0 API As per recommended sequence */
/* Get handle first */
/* Power manager setup -- mm_csi0_init*/
/* Analog PHY setup mm_csi0_set_afe */
/* Digital PHY setup */
/* CSI2 or CCP2/CSI1 mode */
/* Config Image IDs  */
/* Vertical windowing  */
/* Unpack  */
/* Decode  */
/* Horizontal windowing  */
/* Encode  */
/* Pack  */
/* FSP decode  */
/* Output engine  */


void *get_mm_csi0_handle(enum host_mode mode, enum afe_num afe,
		enum csi2_lanes lanes);

/* Power manager set up over here */
int mm_csi0_init(void);

/* Power manager turn-off */
int mm_csi0_teardown(void);

/* Mostly AFE selection and clocks */
int mm_csi0_set_afe(void);

/* Currently void as values are fixed
   Requires clock and data lane timers
   Requires data lane sync matching
   Packet framer timeout mapped to lane speed
   For CSI2, there are no PHY transitions and
   all lane terminations are manual and forced */
int mm_csi0_set_dig_phy(struct lane_timing *timing);

/* int mm_csi0_power_iface(void); */
/* Input CCP2 clocking scheme */

int mm_csi0_set_mode(enum csi1ccp2_clock_mode clk_mode);

/* APIs below are protocol agnostic
   Used to configure the pixel pack and unpack modes
   Used to configure DPCM compression decompression modes */

int mm_csi0_enc_blk_length(int len);
int mm_csi0_cfg_pipeline_dpcm_enc(enum dpcm_encode_mode enc);
int mm_csi0_cfg_pipeline_dpcm_dec(enum dpcm_decode_mode dec);
int mm_csi0_cfg_pipeline_pack(enum pix_pack_mode pack);
int mm_csi0_cfg_pipeline_unpack(enum pix_unpack_mode unpack);

/* Set Horizontal and Vertical windowing */
int mm_csi0_set_windowing_horizontal(int h_pix_start, int h_pix_end);
int mm_csi0_set_windowing_vertical(int v_line_start, int v_line_end);

int mm_csi0_enable_fsp_ccp2(void);

/* vc applies only to CSI2, ignored for CCP2 handles
   Id is the data type for CSI2 and logical channel for CCP2
   Processing depends on CSI2 or CCP2 */

int mm_csi0_cfg_image_id(int vc, int id);

/* Output engine setup
   Single/double buffer or trigger
   Address programming
   Packtimer based on lane speed
   Burst length and spacing
   Panic priority*/


/* For now hard-coded values, to be seen later
   Set burst length, spacing and panic priorities */
int mm_csi0_rx_burst(void);

int mm_csi0_buffering_mode(enum buffer_mode bmode);
int mm_csi0_update_addr(struct buffer_desc *im0, struct buffer_desc *im1,
		struct buffer_desc *dat0, struct buffer_desc *dat1);
int mm_csi0_update_one(struct buffer_desc *im, int buf_num,
		enum buffer_type type);

int mm_csi0_trigger_cap(void);

/* Final level APIs to start and stop RX */
int mm_csi0_enable_unicam(void);
int mm_csi0_start_rx(void);
int mm_csi0_stop_rx(void);

/* Reset API */
int mm_csi0_reset(enum reset_type type);

/* Status and Interrupt APIs
   _ack will also acknowledge and the other will only report
   These APIs return the raw values and the interpretation would
   be in the structure passed. To quickly determine an error, AND
   the raw return with the ERROR_MASK in appropriate mode */
u32 mm_csi0_get_rx_stat(struct rx_stat_list *list, int ack);

/* ISTA register ... only FS, FE and LCI are allowed */
int mm_csi0_get_int_stat(struct int_desc *desc, int ack);

int mm_csi0_get_data_stat(struct int_desc *desc, int ack);

/* Configure interrupts */
int mm_csi0_config_int(struct int_desc *desc, enum buffer_type type);

void mm_csi0_ibwp(void);

/* Packet compare and capture */

/* This feature applies only to CSI2 */
/* Will scan all incoming packets and captures a packet with
   matching credentials. Used to debug and recieve if sensor/ISP
   sends generic short packets */

enum packet_comp_cap mm_csi0_enable_packet_compare(
		struct packet_compare *compare);
enum packet_comp_cap mm_csi0_get_captured_packet(enum packet_comp_cap num,
		struct packet_capture *cap);

/* Get lane trans states */
int mm_csi0_get_trans(void);

/* STA register bit PS will be high to denote a panic was signalled */
bool mm_csi0_get_panic_state(void);

extern void unicam_reg_dump(void);

extern void unicam_reg_dump_dbg(void);

