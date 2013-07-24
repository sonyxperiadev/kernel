/****************************************************************************
*
*  SC7798_CS02.h
*
*  PURPOSE:
*    This is the LCD-specific code for a HX8369 module.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#ifndef __SC7798_H__
#define __SC7798_H__

#include "display_drv.h"
#include "lcd.h"

//  LCD command definitions
#define PIXEL_FORMAT_RGB565	0x05   // for 16 bits
#define PIXEL_FORMAT_RGB666	0x06   // for 18 bits
#define PIXEL_FORMAT_RGB888	0x07   // for 24 bits

#define MAX_BRIGHTNESS		255
#define DEFAULT_BRIGHTNESS	160
#define DEFAULT_GAMMA_LEVEL	14/*180cd*/
#define MAX_GAMMA_VALUE 	24
#define MAX_GAMMA_LEVEL		25

#define DIM_BL 20
#define MIN_BL 30
#define MAX_BL 255

#define GAMMA_INDEX_NOT_SET	0xFFFF

typedef enum {
	SC7798_CMD_NOP					= 0x00,
	SC7798_CMD_SLEEP_IN				= 0x10,		
	SC7798_CMD_SLEEP_OUT				= 0x11,	
	SC7798_CMD_DISP_OFF				= 0x28,	
	SC7798_CMD_DISP_ON				= 0x29,
	SC7798_CMD_WRITE_B				= 0x51,
	SC7798_CMD_BL						= 0x53,	
	SC7798_CMD_SETPOWER				= 0xB1,
	SC7798_CMD_SETDISP				= 0xB2,	
	SC7798_CMD_SETRGBIF				= 0xB3,	
	SC7798_CMD_SETCYC					= 0xB4,
	SC7798_CMD_SETVCOM				= 0xB6,	
	SC7798_CMD_SETPOWER_EXT			= 0xB8,	
	SC7798_CMD_ENABLE_CMD			= 0xB9,
	SC7798_CMD_SETMIPI				= 0xBA,	
	SC7798_CMD_SETVDC				= 0xBC,			
	SC7798_CMD_SETTEST				= 0xC6,
	SC7798_CMD_SETIO					= 0xC7,	
	SC7798_CMD_SETCABC				= 0xC8,		
	SC7798_CMD_SETPANEL				= 0xCC,	
	SC7798_CMD_SETGAMMA				= 0xE0,	
	SC7798_CMD_SETEQ					= 0xE3,	
	SC7798_CMD_SETGIP1				= 0xE9,
	SC7798_CMD_SETGIP2				= 0xEA,	
} SC7798_CMD_T;

__initdata struct DSI_COUNTER sc7798_timing[] = {
	/* LP Data Symbol Rate Calc - MUST BE FIRST RECORD */
	{"ESC2LP_RATIO", DSI_C_TIME_ESC2LPDT, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0x0000003F, 1, 1},
	/* SPEC:  min =  100[us] + 0[UI] */
	/* SET:   min = 1000[us] + 0[UI]                             <= */
	{"HS_INIT", DSI_C_TIME_HS, 0,
		0, 100000, 0, 0, 0, 0, 0, 0, 0x00FFFFFF, 0, 0},
	/* SPEC:  min = 1[ms] + 0[UI] */
	/* SET:   min = 1[ms] + 0[UI] */
	{"HS_WAKEUP", DSI_C_TIME_HS, 0,
		0, 1000000, 0, 0, 0, 0, 0, 0, 0x00FFFFFF, 0, 0},
	/* SPEC:  min = 1[ms] + 0[UI] */
	/* SET:   min = 1[ms] + 0[UI] */
	{"LP_WAKEUP", DSI_C_TIME_ESC, 0,
		0, 1000000, 0, 0, 0, 0, 0, 0, 0x00FFFFFF, 1, 1},
	/* SPEC:  min = 0[ns] +  8[UI] */
	/* SET:   min = 0[ns] + 12[UI]                               <= */
	{"HS_CLK_PRE", DSI_C_TIME_HS, 0,
		0, 0, 12, 0, 0, 0, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min = 38[ns] + 0[UI]   max= 95[ns] + 0[UI] */
	/* SET:   min = 48[ns] + 0[UI]   max= 95[ns] + 0[UI]         <= */
	{"HS_CLK_PREPARE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 48, 0, 0, 0, 95, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min = 262[ns] + 0[UI] */
	/* SET:   min = 262[ns] + 0[UI]                              <= */
	{"HS_CLK_ZERO", DSI_C_TIME_HS, 0,
		0, 320, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min =  60[ns] + 52[UI] */
	/* SET:   min =  70[ns] + 52[UI]                             <= */
	{"HS_CLK_POST", DSI_C_TIME_HS, 0,
		0, 70, 52, 0, 0, 0, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min =  60[ns] + 0[UI] */
	/* SET:   min =  70[ns] + 0[UI]                              <= */
	{"HS_CLK_TRAIL", DSI_C_TIME_HS, 0,
		0, 70, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min =  50[ns] + 0[UI] */
	/* SET:   min =  60[ns] + 0[UI]                              <= */
	{"HS_LPX", DSI_C_TIME_HS, 0,
		0, 60, 0, 0, 0, 75, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min = 40[ns] + 4[UI]      max= 85[ns] + 6[UI] */
	/* SET:   min = 50[ns] + 4[UI]      max= 85[ns] + 6[UI]      <= */
	{"HS_PRE", DSI_C_TIME_HS, DSI_C_HAS_MAX,
		0, 50, 4, 0, 0, 85, 6, 0, 0x000001FF, 0, 0},
	/* SPEC:  min = 105[ns] + 6[UI] */
	/* SET:   min = 105[ns] + 6[UI]                              <= */
	{"HS_ZERO", DSI_C_TIME_HS, 0,
		0, 105, 6, 0, 0, 0, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	/* SET:   min = max(0[ns]+32[UI],60[ns]+16[UI])  n=4 */
	{"HS_TRAIL", DSI_C_TIME_HS, DSI_C_MIN_MAX_OF_2,
		0, 60, 4, 60, 16, 0, 0, 0, 0x000001FF, 0, 0},
	/* SPEC:  min = 100[ns] + 0[UI] */
	/* SET:   min = 110[ns] + 0[UI]                              <= */
	{"HS_EXIT", DSI_C_TIME_HS, 0,
		0, 110, 0, 0, 0, 0, 0, 0, 0x000001FF, 0, 0},
	/* min = 50[ns] + 0[UI] */
	/* LP esc counters are speced in LP LPX units.
	   LP_LPX is calculated by chal_dsi_set_timing
	   and equals LP data clock */
	{"LPX", DSI_C_TIME_ESC, 0,
		1, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1},
	/* min = 4*[Tlpx]  max = 4[Tlpx], set to 4 */
	{"LP-TA-GO", DSI_C_TIME_ESC, 0,
		4, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1},
	/* min = 1*[Tlpx]  max = 2[Tlpx], set to 2 */
	{"LP-TA-SURE", DSI_C_TIME_ESC, 0,
		2, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1},
	/* min = 5*[Tlpx]  max = 5[Tlpx], set to 5 */
	{"LP-TA-GET", DSI_C_TIME_ESC, 0,
		5, 0, 0, 0, 0, 0, 0, 0, 0x000000FF, 1, 1},
};

__initdata DISPCTRL_REC_T sc7798_scrn_on[] = {
	{DISPCTRL_WR_CMND, SC7798_CMD_DISP_ON},
	{DISPCTRL_SLEEP_MS, 100},	
	{DISPCTRL_WR_CMND, SC7798_CMD_BL},
	{DISPCTRL_WR_DATA, 0x2C},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T sc7798_scrn_off[] = {
	{DISPCTRL_WR_CMND, SC7798_CMD_BL},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_SLEEP_MS, 100},
	{DISPCTRL_WR_CMND, SC7798_CMD_DISP_OFF},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T sc7798_slp_in[] = {
	{DISPCTRL_WR_CMND, SC7798_CMD_DISP_OFF},
	{DISPCTRL_WR_CMND, SC7798_CMD_BL},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_SLEEP_MS, 100},
	{DISPCTRL_WR_CMND, SC7798_CMD_SLEEP_IN},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T sc7798_slp_out[] = {
	{DISPCTRL_WR_CMND, SC7798_CMD_SLEEP_OUT},
	{DISPCTRL_SLEEP_MS, 120},
	{DISPCTRL_WR_CMND, SC7798_CMD_DISP_ON},
	{DISPCTRL_SLEEP_MS, 100},	
	{DISPCTRL_WR_CMND, SC7798_CMD_BL},
	{DISPCTRL_WR_DATA, 0x24},
	{DISPCTRL_LIST_END, 0}
};

__initdata DISPCTRL_REC_T sc7798_init_panel_vid[] = {
  {DISPCTRL_WR_CMND, SC7798_CMD_ENABLE_CMD},
	{DISPCTRL_WR_DATA, 0xF1},		
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x00},

  {DISPCTRL_WR_CMND, SC7798_CMD_SETVDC},
	{DISPCTRL_WR_DATA, 0x67},
	
	{DISPCTRL_WR_CMND, SC7798_CMD_SETPOWER},
	{DISPCTRL_WR_DATA, 0x31},
	{DISPCTRL_WR_DATA, 0x1A},
	{DISPCTRL_WR_DATA, 0x1A},
	{DISPCTRL_WR_DATA, 0xA7},	
	{DISPCTRL_WR_DATA, 0x33},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0xB7},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETVCOM},
	{DISPCTRL_WR_DATA, 0x4E},
	{DISPCTRL_WR_DATA, 0x4E},	

	{DISPCTRL_WR_CMND, SC7798_CMD_SETPOWER_EXT},
	{DISPCTRL_WR_DATA, 0x06},
	{DISPCTRL_WR_DATA, 0x22},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETEQ},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x03},

	{DISPCTRL_SLEEP_MS, 10},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETMIPI},
	{DISPCTRL_WR_DATA, 0x31},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x44},
	{DISPCTRL_WR_DATA, 0x25},
	{DISPCTRL_WR_DATA, 0x91},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xC2},
	{DISPCTRL_WR_DATA, 0x34},
	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x04},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x1D},
	{DISPCTRL_WR_DATA, 0xB9},
	{DISPCTRL_WR_DATA, 0xEE},
	{DISPCTRL_WR_DATA, 0x40},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETGIP1},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x01},
	{DISPCTRL_WR_DATA, 0x00},	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x79},
	{DISPCTRL_WR_DATA, 0x78},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x30},
	{DISPCTRL_WR_DATA, 0x00},	
	
	{DISPCTRL_WR_DATA, 0x48},
	{DISPCTRL_WR_DATA, 0x03},
	{DISPCTRL_WR_DATA, 0x79},
	{DISPCTRL_WR_DATA, 0x78},	
	{DISPCTRL_WR_DATA, 0x47},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x60},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x28},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},	
	{DISPCTRL_WR_DATA, 0x86},
	{DISPCTRL_WR_DATA, 0x64},
	{DISPCTRL_WR_DATA, 0x42},
	{DISPCTRL_WR_DATA, 0x20},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x29},

	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x87},	
	{DISPCTRL_WR_DATA, 0x75},
	{DISPCTRL_WR_DATA, 0x53},
	{DISPCTRL_WR_DATA, 0x31},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x39},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},	

	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETGIP2},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x91},
	{DISPCTRL_WR_DATA, 0x13},	
	{DISPCTRL_WR_DATA, 0x35},
	{DISPCTRL_WR_DATA, 0x57},
	{DISPCTRL_WR_DATA, 0x73},
	{DISPCTRL_WR_DATA, 0x18},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},	
	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x90},
	{DISPCTRL_WR_DATA, 0x02},
	{DISPCTRL_WR_DATA, 0x24},	
	{DISPCTRL_WR_DATA, 0x46},
	{DISPCTRL_WR_DATA, 0x62},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x30},
	{DISPCTRL_WR_DATA, 0x00},
	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xFF},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x2A},	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},	
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETDISP},
	{DISPCTRL_WR_DATA, 0x23},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETRGBIF},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},	
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x0A},
	{DISPCTRL_WR_DATA, 0x25},
	{DISPCTRL_WR_DATA, 0x20},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETCYC},
	{DISPCTRL_WR_DATA, 0x00},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETTEST},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0xFD},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETPANEL},
	{DISPCTRL_WR_DATA, 0x0E},

	{DISPCTRL_WR_CMND, SC7798_CMD_SETGAMMA},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x10},
	{DISPCTRL_WR_DATA, 0x2D},	
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x1F},
	{DISPCTRL_WR_DATA, 0x3C},
	{DISPCTRL_WR_DATA, 0x09},
	{DISPCTRL_WR_DATA, 0x0C},	
	
	{DISPCTRL_WR_DATA, 0x0E},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x12},	
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x17},
	{DISPCTRL_WR_DATA, 0x00},
	{DISPCTRL_WR_DATA, 0x0B},
	{DISPCTRL_WR_DATA, 0x10},
	
	{DISPCTRL_WR_DATA, 0x2D},
	{DISPCTRL_WR_DATA, 0x34},
	{DISPCTRL_WR_DATA, 0x3F},
	{DISPCTRL_WR_DATA, 0x1F},	
	{DISPCTRL_WR_DATA, 0x3B},
	{DISPCTRL_WR_DATA, 0x08},
	{DISPCTRL_WR_DATA, 0x0D},
	{DISPCTRL_WR_DATA, 0x0E},
	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x14},

	{DISPCTRL_WR_DATA, 0x12},
	{DISPCTRL_WR_DATA, 0x14},
	{DISPCTRL_WR_DATA, 0x11},
	{DISPCTRL_WR_DATA, 0x17},	

  {DISPCTRL_WR_CMND, SC7798_CMD_SETCABC}, //Temporary BL control on lcd initialization
  {DISPCTRL_WR_DATA, 0x11},
  {DISPCTRL_WR_DATA, 0x00},

  {DISPCTRL_WR_CMND, SC7798_CMD_SETIO}, //PWM OUT EN
  {DISPCTRL_WR_DATA, 0xC0},

	{DISPCTRL_WR_CMND, SC7798_CMD_SLEEP_OUT},
	{DISPCTRL_SLEEP_MS, 200},

	{DISPCTRL_WR_CMND, SC7798_CMD_DISP_ON},
	{DISPCTRL_SLEEP_MS, 200},

	{DISPCTRL_WR_CMND, SC7798_CMD_BL},
	{DISPCTRL_WR_DATA, 0x2C},

	{DISPCTRL_WR_CMND, SC7798_CMD_WRITE_B}, //Temporary BL control on lcd initialization
	{DISPCTRL_WR_DATA, 0xA0},		

	{DISPCTRL_LIST_END, 0},
};

void sc7798_winset(char *msgData, DISPDRV_WIN_t *p_win)
{
	return;
}

__initdata struct lcd_config sc7798_cfg = {
	.name = "SC7798",
	.mode_supp = LCD_VID_ONLY,
	.phy_timing = &sc7798_timing[0],
	.max_lanes = 2,
	.max_hs_bps = 500000000,
	.max_lp_bps = 9500000,
	.phys_width = 55,
	.phys_height = 99,
	.init_cmd_seq = NULL,
	.init_vid_seq = &sc7798_init_panel_vid[0],
	.slp_in_seq = &sc7798_slp_in[0],
	.slp_out_seq = &sc7798_slp_out[0],
	.scrn_on_seq = &sc7798_scrn_on[0],
	.scrn_off_seq = &sc7798_scrn_off[0],
	.id_seq = NULL,
	.verify_id = false,	
	.updt_win_fn = sc7798_winset,
	.updt_win_seq_len = 0,
	.vid_cmnds = false,
	.vburst = true,
	.cont_clk = false,
	.hs = 17,
	.hbp = 49,
	.hfp = 94,
	.vs = 6,
	.vbp = 11,
	.vfp = 8,
};

#endif /* __HX8369_H__ */

