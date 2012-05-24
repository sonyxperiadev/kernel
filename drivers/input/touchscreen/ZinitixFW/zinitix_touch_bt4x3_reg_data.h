#ifndef _SAIN_BT4X3_REG_DATA_H
#define	_SAIN_BT4X3_REG_DATA_H

/*============================================================================*/
#define	MAX_REG_COUNT	0x1D0

typedef struct {
	s16 reg_val;
	u8 valid;
} _zinitix_reg_data;
const _zinitix_reg_data m_reg_data[MAX_REG_COUNT] = {
	/*{value, valid} */
	{0, 0},			/* 00, RESERVED */
	{0, 0},			/* 01, RESERVED */
	{0, 0},			/* 02, RESERVED */
	{0, 0},			/* 03, RESERVED */
	{0, 0},			/* 04, RESERVED */
	{0, 0},			/* 05, RESERVED */
	{0, 0},			/* 06, RESERVED */
	{0, 0},			/* 07, RESERVED */
	{0, 0},			/* 08, RESERVED */
	{0, 0},			/* 09, RESERVED */
	{0, 0},			/* 0A, RESERVED */
	{0, 0},			/* 0B, RESERVED */
	{0, 0},			/* 0C, RESERVED */
	{0, 0},			/* 0D, RESERVED */
	{0, 0},			/* 0E, RESERVED */
	{0, 0},			/* 0F, RESERVED */
	{0, 1},			/* 10, TOUCH MODE */
	{0x0E0E, 0},		/* 11, CHIP REVISION */
	{0x0038, 0},		/* 12, FIRMWARE VERSION */
	{0, 1},			/* 13, REGISTER DATA VERSION */
	{0, 1},			/* 14, TSP TYPE */
	{10, 1},		/* 15, SUPPORTED FINGER NUM */
	{0, 0},			/* 16, RESERVED */
	{0x002C, 1},		/* 17, INTERNAL FLAG */
	{0xFFFA, 0},		/* 18, EEPROM INFO */
	{0, 0},			/* 19, RESERVED */
	{0, 0},			/* 1A, RESERVED */
	{0, 0},			/* 1B, RESERVED */
	{0, 0},			/* 1C, RESERVED */
	{200, 0},		/* 1D, CURRENT SENSITIVITY TH */
	{0, 0},			/* 1E, CURRENT CHARGER LIMIT CNT */
	{4, 0},			/* 1F, CURRENT RAW VARIATION */
	{200, 1},		/* 20, SENSITIVITY TH. */
	{200, 1},		/* 21, Y0 SENSITIVITY TH */
	{200, 1},		/* 22, LAST Y SENSITIVITY TH */
	{200, 1},		/* 23, X0 SENSITIVITY TH */
	{200, 1},		/* 24, LAST X SENSITIVITY TH */
	{8, 1},			/* 25, ACTIVE SENSITIVITY COEF */
	{5, 1},			/* 26, AUTO SENSITIVITY TH STEP */
	{50, 1},		/* 27, AUTO SENSITIVITY TH VALUE PER STEP */
	{5, 1},			/* 28, 1st BASELINE VARIATION */
	{20, 1},		/* 29, 2nd BASELINE VARIATION */
	{40, 1},		/* 2A, 1st BASELINE PERIOD */
	{10, 1},		/* 2B, 2nd BASELINE PERIOD */
	{1000, 1},		/* 2C, BASELINE FORCE PERIOD */
	{20, 1},		/* 2D, 1st BASELINE VARIATION ON CHARGER */
	{40, 1},		/* 2E, 2nd BASELINE VARIATION ON CHARGER */
	{40, 1},		/* 2F, BASELINE UPDATE PERIOD ON CHARGER */
	{2, 1},			/* 30, FIR COEFFICIENT */
	{2, 1},			/* 31, HW_STYLUS MOVING FIR */
	{2, 1},			/* 32, HW_FINGER MOVING FIR */
	{2, 1},			/* 33, SW_FIR COEFFICIENT */
	{2, 1},			/* 34, SW WIDTH FIR */
	{15, 1},		/* 35, WIDTH(WEIGHT) COEF */
	{0x1E05, 1},		/* 36, MVAVG_1_VELOCITY */
	{258, 1},		/* 37, MVAVG_1_SW_INC */
	{0, 0},			/* 38, RESERVED */
	{0, 0},			/* 39, RESERVED */
	{0, 0},			/* 3A, RESERVED */
	{2, 1},			/* 3B, REACTION COUNT */
	{160, 1},		/* 3C, PALM REJECT TRESHHOLD */
	{160, 1},		/* 3D, NOISE REJECT TRESHHOLD */
	{0x0203, 1},		/* 3E, NOISE REJECT HILO RATIO */
	{80, 1},		/* 3F, NOISE PALM LEVEL */
	{10, 1},		/* 40, NOISE PALM UP SKIP COUNT */
	{5, 1},			/* 41, SKIP REJECT COUNT AFTER DETECT */
	{0x0103, 1},		/* 42, CUTOFF NOISE PDATA RATIO */
	{0x0103, 1},		/* 43, CUTOFF NOISE WIDTH RATIO */
	{128, 1},		/* 44, REACTION THRESHHOLD */
	{0x0104, 1},		/* 45, CHECK NOISE PATTERN P */
	{3, 1},			/* 46, CHECK NOISE PATTERN P CENTER CNT */
	{2, 1},			/* 47, CHECK NOISE PATTERN P EDGE CNT */
	{1, 1},			/* 48, CHECK NOISE PATTERN P CORNER CNT */
	{0x010A, 1},		/* 49, CHECK NOISE PATTERN N */
	{2, 1},			/* 4A, CHECK NOISE PATTERN N CNT */
	{80, 1},		/* 4B, CHECK NOISE STYLUS RAW LIMIT VALUE */
	{0x0203, 1},		/* 4C, CHECK NOISE STYLUS PATTERN P */
	{4, 1},			/* 4D, CHECK NOISE STYLUS PATTERN P CNT */
	{0x0203, 1},		/* 4E, CHECK NOISE STYLUS PATTERN N */
	{2, 1},			/* 4F, CHECK NOISE STYLUS PATTERN N CNT */
	{0, 1},			/* 50, AUTO CHARGING DETECT USE */
	{0, 1},			/* 51, CHARGING MODE */
	{15, 1},		/* 52, CHARGING STEP LIMIT */
	{1000, 1},		/* 53, CHARGING MODE SENSITIVITY TH */
	{20, 1},		/* 54, AUTO CHARGING OUT VARIATION */
	{200, 1},		/* 55, AUTO CHARGING IN VARIATION */
	{0x0103, 1},		/* 56, AUTO CHARING STRENGTH RATIO */
	{80, 1},		/* 57, AUTO CHARING LIMIT VALUE */
	{10, 1},		/* 58, AUTO CHARING LIMIT CNT */
	{10, 1},		/* 59, AUTO CHARGING SKIP CNT */
	{0, 1},			/* 5A, AUTO CHARGING REJECT HILO RATIO */
	{80, 1},		/* 5B, AUTO CHARGING REJECT PALM CNT */
	{10, 1},		/* 5C, AUTO CHARGING REACTION COUNT */
	{0, 0},			/* 5D, RESERVED */
	{0, 0},			/* 5E, RESERVED */
	{0, 0},			/* 5F, RESERVED */
	{20, 1},		/* 60, TOTAL NUM OF X */
	{16, 1},		/* 61, TOTAL NUM OF Y */
	{0x0B0A, 1},		/* 62, X00_01_DRIVE_NUM */
	{0x0D0C, 1},		/* 63, X02_03_DRIVE_NUM */
	{0x0F0E, 1},		/* 64, X04_05_DRIVE_NUM */
	{0x1110, 1},		/* 65, X06_07_DRIVE_NUM */
	{0x1312, 1},		/* 66, X08_09_DRIVE_NUM */
	{0x0100, 1},		/* 67, X10_11_DRIVE_NUM */
	{0x0302, 1},		/* 68, X12_13_DRIVE_NUM */
	{0x0504, 1},		/* 69, X14_15_DRIVE_NUM */
	{0x0706, 1},		/* 6A, X16_17_DRIVE_NUM */
	{0x0908, 1},		/* 6B, X18_19_DRIVE_NUM */
	{0x1514, 1},		/* 6C, X20_21_DRIVE_NUM */
	{0x1716, 1},		/* 6D, X22_23_DRIVE_NUM */
	{0x1918, 1},		/* 6E, X24_25_DRIVE_NUM */
	{0x1B1A, 1},		/* 6F, X26_27_DRIVE_NUM */
	{0x1D1C, 1},		/* 70, X28_29_DRIVE_NUM */
	{0x1F1E, 1},		/* 71, X30_31_DRIVE_NUM */
	{0x2120, 1},		/* 72, X32_33_DRIVE_NUM */
	{0x2322, 1},		/* 73, X34_35_DRIVE_NUM */
	{0x2524, 1},		/* 74, X36_37_DRIVE_NUM */
	{0x2726, 1},		/* 75, X38_39_DRIVE_NUM */
	{1700, 1},		/* 76, CALIBRATION REFERENCE */
	{1, 1},			/* 77, CALIBRATION C MODE */
	{15, 1},		/* 78, CALIBRATION DEFAULT N COUNT */
	{15, 1},		/* 79, CALIBRATION DEFAULT C */
	{32, 1},		/* 7A, CALIBRATION ACCURACY */
	{20, 1},		/* 7B, SOFT CALIBRATION INIT COUNT */
	{0, 0},			/* 7C, RESERVED */
	{0, 0},			/* 7D, RESERVED */
	{0, 0},			/* 7E, RESERVED */
	{0, 0},			/* 7F, RESERVED */
	{0, 0},			/* 80, RESERVED */
	{0, 0},			/* 81, RESERVED */
	{0, 0},			/* 82, RESERVED */
	{0, 0},			/* 83, RESERVED */
	{0, 0},			/* 84, RESERVED */
	{0, 0},			/* 85, RESERVED */
	{0, 0},			/* 86, RESERVED */
	{0, 0},			/* 87, RESERVED */
	{0, 0},			/* 88, RESERVED */
	{0, 0},			/* 89, RESERVED */
	{0, 0},			/* 8A, RESERVED */
	{0, 0},			/* 8B, RESERVED */
	{0, 0},			/* 8C, RESERVED */
	{0, 0},			/* 8D, RESERVED */
	{0, 0},			/* 8E, RESERVED */
	{0, 0},			/* 8F, RESERVED */
	{0, 0},			/* 90, RESERVED */
	{0, 0},			/* 91, RESERVED */
	{0, 0},			/* 92, RESERVED */
	{0, 0},			/* 93, RESERVED */
	{0, 0},			/* 94, RESERVED */
	{0, 0},			/* 95, RESERVED */
	{0, 0},			/* 96, RESERVED */
	{0, 0},			/* 97, RESERVED */
	{0, 0},			/* 98, RESERVED */
	{0, 0},			/* 99, RESERVED */
	{0, 0},			/* 9A, RESERVED */
	{0, 0},			/* 9B, RESERVED */
	{0, 0},			/* 9C, RESERVED */
	{0, 0},			/* 9D, RESERVED */
	{0, 0},			/* 9E, RESERVED */
	{0, 0},			/* 9F, RESERVED */
	{0, 0},			/* A0, RESERVED */
	{0, 0},			/* A1, RESERVED */
	{0, 0},			/* A2, RESERVED */
	{0, 0},			/* A3, RESERVED */
	{0, 0},			/* A4, RESERVED */
	{0, 0},			/* A5, RESERVED */
	{0, 0},			/* A6, RESERVED */
	{0, 0},			/* A7, RESERVED */
	{0, 0},			/* A8, RESERVED */
	{0, 0},			/* A9, RESERVED */
	{0, 0},			/* AA, RESERVED */
	{0, 0},			/* AB, RESERVED */
	{0, 0},			/* AC, RESERVED */
	{0, 0},			/* AD, RESERVED */
	{0, 0},			/* AE, RESERVED */
	{0, 0},			/* AF, RESERVED */
	{4, 1},			/* B0, SUPPORTED BUTTON NUM */
	{0, 1},			/* B1, BUTTON REACTION CNT */
	{200, 1},		/* B2, BUTTON SENSITIVITY TH */
	{0, 1},			/* B3, BUTTON LINE TYPE */
	{0, 1},			/* B4, BUTTON LINE NUM */
	{3, 1},			/* B5, BUTTON RANGE */
	{3, 1},			/* B6, BUTTON_0 START NODE */
	{7, 1},			/* B7, BUTTON_1 START NODE */
	{11, 1},		/* B8, BUTTON_2 START NODE */
	{15, 1},		/* B9, BUTTON_3 START NODE */
	{0, 1},			/* BA, BUTTON_4 START NODE */
	{0, 1},			/* BB, BUTTON_5 START NODE */
	{0, 1},			/* BC, BUTTON_6 START NODE */
	{0, 1},			/* BD, BUTTON_7 START NODE */
	{0, 0},			/* BE, RESERVED */
	{0, 0},			/* BF, RESERVED */
	{2560, 1},		/* C0, RESOLUTION OF X */
	{2048, 1},		/* C1, RESOLUTION OF Y */
	{0x0001, 1},		/* C2, COORD ORIENTATION */
	{8, 1},			/* C3, HOLD POINT THRESHOLD */
	{4, 1},			/* C4, HOLD WIDTH THRESHOLD */
	{1000, 1},		/* C5, STYLUS HW THRESHHOLD */
	{10000, 1},		/* C6, ASSUME UP THRESHHOLD */
	{64, 1},		/* C7, ASSUME UP SKIP THRESHHOLD */
	{0, 1},			/* C8, X POINT SHIFT */
	{0, 1},			/* C9, Y POINT SHIFT */
	{0, 1},			/* CA, VIEW XF OFFSET */
	{0, 1},			/* CB, VIEW XL OFFSET */
	{0, 1},			/* CC, VIEW YF OFFSET */
	{0, 1},			/* CD, VIEW YL OFFSET */
	{0, 0},			/* CE, RESERVED */
	{0, 0},			/* CF, RESERVED */
	{69, 1},		/* D0, FINGER COEF X GAIN */
	{1000, 1},		/* D1, FINGER ATTACH VALUE */
	{400, 1},		/* D2, STYLUS ATTACH VALUE */
	{0, 0},			/* D3, RESERVED */
	{0, 0},			/* D4, RESERVED */
	{0x0005, 1},		/* D5, PDATA COEF1 */
	{0x0003, 1},		/* D6, PDATA COEF2 */
	{0x0003, 1},		/* D7, PDATA COEF3 */
	{0, 0},			/* D8, RESERVED */
	{0, 0},			/* D9, RESERVED */
	{10, 1},		/* DA, EDGE COEFFICIENT */
	{100, 0},		/* DB, OPT Q RESOLUTION */
	{0x7777, 1},		/* DC, PDATA EDGE COEF1 */
	{0x4444, 1},		/* DD, PDATA EDGE COEF2 */
	{0x3333, 1},		/* DE, PDATA EDGE COEF3 */
	{160, 1},		/* DF, EDGE Q BIAS1_1 */
	{160, 1},		/* E0, EDGE Q BIAS2_1 */
	{165, 1},		/* E1, EDGE Q BIAS3_1 */
	{165, 1},		/* E2, EDGE Q BIAS4_1 */
	{0, 0},			/* E3, RESERVED */
	{0, 0},			/* E4, RESERVED */
	{0x8888, 1},		/* E5, PDATA CORNER COEF1 */
	{0x3333, 1},		/* E6, PDATA CORNER COEF2 */
	{0x3333, 1},		/* E7, PDATA CORNER COEF3 */
	{130, 1},		/* E8, CORNER Q BIAS1_1 */
	{130, 1},		/* E9, CORNER Q BIAS2_1 */
	{130, 1},		/* EA, CORNER Q BIAS3_1 */
	{130, 1},		/* EB, CORNER Q BIAS4_1 */
	{0, 0},			/* EC, RESERVED */
	{0, 0},			/* ED, RESERVED */
	{0, 0},			/* EE, RESERVED */
	{0, 0},			/* EF, RESERVED */
	{0x080F, 1},		/* F0, INT ENABLE FLAG */
	{0, 1},			/* F1, PERIODICAL INTERRUPT INTERVAL */
	{0, 0},			/* F2, RESERVED */
	{0, 0},			/* F3, RESERVED */
	{0, 0},			/* F4, RESERVED */
	{0, 0},			/* F5, RESERVED */
	{0, 0},			/* F6, RESERVED */
	{0, 0},			/* F7, RESERVED */
	{0, 0},			/* F8, RESERVED */
	{0, 0},			/* F9, RESERVED */
	{0, 0},			/* FA, RESERVED */
	{0, 0},			/* FB, RESERVED */
	{0, 0},			/* FC, RESERVED */
	{0, 0},			/* FD, RESERVED */
	{0, 0},			/* FE, RESERVED */
	{0, 0},			/* FF, RESERVED */
	{40, 1},		/* 100, AFE FREQUENCY */
	{0x2828, 1},		/* 101, FREQ X NUM 0_1 */
	{0x2828, 1},		/* 102, FREQ X NUM 2_3 */
	{0x2828, 1},		/* 103, FREQ X NUM 4_5 */
	{0x2828, 1},		/* 104, FREQ X NUM 6_7 */
	{0x2828, 1},		/* 105, FREQ X NUM 8_9 */
	{0x2828, 1},		/* 106, FREQ X NUM 10_11 */
	{0x2828, 1},		/* 107, FREQ X NUM 12_13 */
	{0x2828, 1},		/* 108, FREQ X NUM 14_15 */
	{0x2828, 1},		/* 109, FREQ X NUM 16_17 */
	{0x2828, 1},		/* 10A, FREQ X NUM 18_19 */
	{0x2828, 1},		/* 10B, FREQ X NUM 20_21 */
	{0x2828, 1},		/* 10C, FREQ X NUM 22_23 */
	{0x2828, 1},		/* 10D, FREQ X NUM 24_25 */
	{0x2828, 1},		/* 10E, FREQ X NUM 26_27 */
	{0x2828, 1},		/* 10F, FREQ X NUM 28_29 */
	{0x2828, 1},		/* 110, FREQ X NUM 30_31 */
	{0x2828, 1},		/* 111, FREQ X NUM 32_33 */
	{0x2828, 1},		/* 112, FREQ X NUM 34_35 */
	{0x2828, 1},		/* 113, FREQ X NUM 36_37 */
	{0x2828, 1},		/* 114, FREQ X NUM 38_39 */
	{0, 0},			/* 115, RESERVED */
	{0, 0},			/* 116, RESERVED */
	{0, 0},			/* 117, RESERVED */
	{0, 0},			/* 118, RESERVED */
	{0, 0},			/* 119, RESERVED */
	{0, 0},			/* 11A, RESERVED */
	{0, 0},			/* 11B, RESERVED */
	{0, 0},			/* 11C, RESERVED */
	{0, 0},			/* 11D, RESERVED */
	{0, 0},			/* 11E, RESERVED */
	{0, 0},			/* 11F, RESERVED */
	{0, 1},			/* 120, AFE MODE */
	{0, 1},			/* 121, AFE C MODE */
	{10, 1},		/* 122, AFE DEFAULT N COUNT */
	{63, 1},		/* 123, AFE DEFAULT C */
	{0x0000, 1},		/* 124, ONE NODE SCAN DELAY */
	{0x0000, 0},		/* 125, CUR ONE NODE SCAN DELAY */
	{0x0000, 1},		/* 126, ALL NODE SCAN DELAY LSB */
	{0x0000, 1},		/* 127, ALL NODE SCAN DELAY MSB */
	{0x0000, 0},		/* 128, CUR ALL NODE SCAN DELAY LSB */
	{0x0000, 0},		/* 129, CUR ALL NODE SCAN DELAYMSB */
	{0, 0},			/* 12A, AFE SCAN NOISE C */
	{2, 1},			/* 12B, AFE R SHIFT VALUE */
	{0, 1},			/* 12C, AFE SCAN MODE */
	{0, 0},			/* 12D, RESERVED */
	{0, 0},			/* 12E, RESERVED */
	{0, 0},			/* 12F, RESERVED */
	{0x3333, 1},		/* 130, REG_AFE_X_VAL */
	{0xFFFF, 1},		/* 131, REG_AFE_XA_EN */
	{0xFFFF, 1},		/* 132, REG_AFE_XB_EN */
	{0x3305, 1},		/* 133, REG_AFE_X_NOVL */
	{0x0011, 1},		/* 134, REG_AFE_Y_NOVL */
	{0x0133, 1},		/* 135, REG_AFE_Y_VAL */
	{0x0001, 1},		/* 136, REG_RBG_EN */
	{0x00FF, 1},		/* 137, REG_INTAMP_EN */
	{0x0011, 1},		/* 138, REG_INTAMP_VREF_EN */
	{0x2000, 1},		/* 139, REG_INTAMP_VREF_NSEL_N */
	{0x0002, 1},		/* 13A, REG_INTAMP_VREF_CTRL */
	{0x007F, 1},		/* 13B, REG_INTAMP_TIME0 */
	{0x00FF, 1},		/* 13C, REG_INTAMP_TIME1 */
	{0x3F0F, 1},		/* 13D, REG_SAR_SAMPLE_TIME */
	{0x0001, 1},		/* 13E, REG_SAR_CTRL */
	{0x0000, 1},		/* 13F, REG_SAR_BUF_EN */
	{0x0000, 1},		/* 140, REG_ATEST_CTRL */
	{0x0000, 1},		/* 141, REG_ATEST_SEL0 */
	{0x0000, 1},		/* 142, REG_ATEST_SEL1 */
	{0x0004, 1},		/* 143, REG_MULTI_FRAME */
	{0, 0},			/* 144, RESERVED */
	{0, 0},			/* 145,  */
	{0, 0},			/* 146,  */
	{0, 0},			/* 147,  */
	{0, 0},			/* 148,  */
	{0, 0},			/* 149,  */
	{0, 0},			/* 14A,  */
	{0, 0},			/* 14B,  */
	{0, 0},			/* 14C,  */
	{0, 0},			/* 14D,  */
	{0, 0},			/* 14E,  */
	{0, 0},			/* 14F,  */
	{0, 0},			/* 150,  */
	{0, 0},			/* 151,  */
	{0, 0},			/* 152,  */
	{0, 0},			/* 153,  */
	{0, 0},			/* 154,  */
	{0, 0},			/* 155,  */
	{0, 0},			/* 156,  */
	{0, 0},			/* 157,  */
	{0, 0},			/* 158,  */
	{0, 0},			/* 159,  */
	{0, 0},			/* 15A,  */
	{0, 0},			/* 15B,  */
	{0, 0},			/* 15C,  */
	{0, 0},			/* 15D,  */
	{0, 0},			/* 15E,  */
	{0, 0},			/* 15F,  */
	{0, 0},			/* 160,  */
	{0, 0},			/* 161,  */
	{0, 0},			/* 162,  */
	{0, 0},			/* 163,  */
	{0, 0},			/* 164,  */
	{0, 0},			/* 165,  */
	{0, 0},			/* 166,  */
	{0, 0},			/* 167,  */
	{0, 0},			/* 168,  */
	{0, 0},			/* 169,  */
	{0, 0},			/* 16A,  */
	{0, 0},			/* 16B,  */
	{0, 0},			/* 16C,  */
	{0, 0},			/* 16D,  */
	{0, 0},			/* 16E,  */
	{0, 0},			/* 16F,  */
	{0, 0},			/* 170,  */
	{0, 0},			/* 171,  */
	{0, 0},			/* 172,  */
	{0, 0},			/* 173,  */
	{0, 0},			/* 174,  */
	{0, 0},			/* 175,  */
	{0, 0},			/* 176,  */
	{0, 0},			/* 177,  */
	{0, 0},			/* 178,  */
	{0, 0},			/* 179,  */
	{0, 0},			/* 17A,  */
	{0, 0},			/* 17B,  */
	{0, 0},			/* 17C,  */
	{0, 0},			/* 17D,  */
	{0, 0},			/* 17E,  */
	{0, 0},			/* 17F,  */
	{0, 0},			/* 180,  */
	{0, 0},			/* 181,  */
	{0, 0},			/* 182,  */
	{0, 0},			/* 183,  */
	{0, 0},			/* 184,  */
	{0, 0},			/* 185,  */
	{0, 0},			/* 186,  */
	{0, 0},			/* 187,  */
	{0, 0},			/* 188,  */
	{0, 0},			/* 189,  */
	{0, 0},			/* 18A,  */
	{0, 0},			/* 18B,  */
	{0, 0},			/* 18C,  */
	{0, 0},			/* 18D,  */
	{0, 0},			/* 18E,  */
	{0, 0},			/* 18F,  */
	{0, 0},			/* 190,  */
	{0, 0},			/* 191,  */
	{0, 0},			/* 192,  */
	{0, 0},			/* 193,  */
	{0, 0},			/* 194,  */
	{0, 0},			/* 195,  */
	{0, 0},			/* 196,  */
	{0, 0},			/* 197,  */
	{0, 0},			/* 198,  */
	{0, 0},			/* 199,  */
	{0, 0},			/* 19A,  */
	{0, 0},			/* 19B,  */
	{0, 0},			/* 19C,  */
	{0, 0},			/* 19D,  */
	{0, 0},			/* 19E,  */
	{0, 0},			/* 19F,  */
	{0, 0},			/* 1A0,  */
	{0, 0},			/* 1A1,  */
	{0, 0},			/* 1A2,  */
	{0, 0},			/* 1A3,  */
	{0, 0},			/* 1A4,  */
	{0, 0},			/* 1A5,  */
	{0, 0},			/* 1A6,  */
	{0, 0},			/* 1A7,  */
	{0, 0},			/* 1A8,  */
	{0, 0},			/* 1A9,  */
	{0, 0},			/* 1AA,  */
	{0, 0},			/* 1AB,  */
	{0, 0},			/* 1AC,  */
	{0, 0},			/* 1AD,  */
	{0, 0},			/* 1AE,  */
	{0, 0},			/* 1AF,  */
	{0, 0},			/* 1B0,  */
	{0, 0},			/* 1B1,  */
	{0, 0},			/* 1B2,  */
	{0, 0},			/* 1B3,  */
	{0, 0},			/* 1B4,  */
	{0, 0},			/* 1B5,  */
	{0, 0},			/* 1B6,  */
	{0, 0},			/* 1B7,  */
	{0, 0},			/* 1B8,  */
	{0, 0},			/* 1B9,  */
	{0, 0},			/* 1BA,  */
	{0, 0},			/* 1BB,  */
	{0, 0},			/* 1BC,  */
	{0, 0},			/* 1BD,  */
	{0, 0},			/* 1BE,  */
	{0, 0},			/* 1BF,  */
	{0, 0},			/* 1C0,  */
	{0, 0},			/* 1C1,  */
	{0, 0},			/* 1C2,  */
	{0, 0},			/* 1C3,  */
	{0, 0},			/* 1C4,  */
	{0, 0},			/* 1C5,  */
	{0, 0},			/* 1C6,  */
	{0, 0},			/* 1C7,  */
	{0, 0},			/* 1C8,  */
	{0, 0},			/* 1C9,  */
	{0, 0},			/* 1CA,  */
	{0, 0},			/* 1CB,  */
	{0, 0},			/* 1CC,  */
	{0, 0},			/* 1CD,  */
	{0, 0},			/* 1CE,  */
	{0, 0},			/* 1CF,  */

};
#endif
