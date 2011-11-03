/*****************************************************************************
*  Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/*****************************************************************************
 *    PURPOSE:       NAND controller driver.
 *
 *    FILE:          chal_nand.c
 *
 *    DESCRIPTION:   NAND API and low-level routines
 *
 *    WRITTEN BY:    agf@broadcom.com, Jul 20 2007.
 *                   dwh, Nov 18 2008.
 *                   cdoban@broadcom.com, Jan 26 2010.
**/

#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <chal/chal_nand.h>
#include <chal/chal_nand_reg.h>
#include <chal/chal_nand_cmd.h>
#include <chal/chal_nand_uc.h>     /* micro-code */

#define CHAL_NAND_ECC_HM  0

/* Add line number in the upper 3 bytes of return code */
#define CHAL_NAND_DEBUG   0

/* timeout values are in micro second */
/* reset timeout (> tRST) */
#define CHAL_NAND_TIMEOUT_RESET              2000
/* read timeout (> tR) */
#define CHAL_NAND_TIMEOUT_READ               5000
/* program timeout (> tPROG) */
#define CHAL_NAND_TIMEOUT_WRITE              10000
/* block erase timeout (> tBERS) */
#define CHAL_NAND_TIMEOUT_ERASE              50000
/* DMA complete timeout */
#define CHAL_NAND_TIMEOUT_DMA                10000
/* UC complete timeout */
#define CHAL_NAND_TIMEOUT_UC_COMPLETE        10000
/* MM CMD complete timeout */
#define CHAL_NAND_TIMEOUT_MM_CMD_COMPLETE    5000



/* bad block marker */
#define NAND_BADBLOCK_MARKER_POS    (0x0)
#define NAND_BADBLOCK_MARKER_GOOD   (0xFF)
#define NAND_BADBLOCK_MARKER_BAD    (0x0)


#ifndef __BROM_CODE__
#define	TIMER_TICK_ADJUSTED_DELAY(usec)     (usec)
#else
/*
 * Boot ROM is using 32KHz GPT timer, so minimal resolution is 31.25us
 * Delays need to be rounded up to 31usec multiple
 */
#define  TIMER_RESOLUTION           31  /* actually 31.25us*/
#define	TIMER_TICK_ADJUSTED_DELAY(usec)	   ((((usec)+TIMER_RESOLUTION-1)/TIMER_RESOLUTION)*TIMER_RESOLUTION)
#endif


/*============================================================================*/

/* PRIVATE TYPES		*/

typedef struct tag_chal_nand_ops_t {
#ifndef __BROM_CODE__
  uint8_t	(*status_get)  (chal_nand_info_t *ni, uint8_t bank);
  uint32_t	(*block_erase) (chal_nand_info_t *ni, uint8_t bank, uint32_t block);
  uint32_t 	(*page_write)  (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
  uint32_t 	(*oob_write)  (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
  uint32_t 	(*page_oob_write)  (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
#endif
  uint32_t	(*id_get ) 	(chal_nand_info_t *ni, uint8_t bank, uint8_t addr, uint8_t *buf);
  uint32_t 	(*page_read)	(chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t *buf);
  uint32_t 	(*oob_read)	(chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t *buf);
  uint32_t 	(*param_read)	(chal_nand_info_t *ni, uint8_t bank, uint8_t *buf);
} chal_nand_ops_t;


/*============================================================================*/

/* PRIVATE prototypes */

static void	    nand_pshare_set	   (void);
static uint32_t uc_load_function    (chal_nand_info_t *ni, chal_nand_uc_cmd_t cmd, const uint32_t *ucode, uint16_t nb_instr);
static uint32_t nand_do_cmd		   (chal_nand_info_t *ni, chal_nand_uc_cmd_t cmd, uint8_t bank, uint32_t page, void* buf);
static void	    nand_cfg_geometry	(chal_nand_info_t *ni);
static void	    nand_cfg_onfi	      (chal_nand_info_t *ni, uint8_t *param_page);
static uint32_t shift_of		      (uint32_t i);
static uint32_t crc16_valid         (uint8_t *p, uint32_t len);
#ifndef __BROM_CODE__
static uint8_t  nand_status_get     (chal_nand_info_t *ni, uint8_t bank);
#endif

#ifndef __BROM_CODE__
/*============================================================================*/
/* pio/test methods */
static uint32_t page_write_pio 	   (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
static uint32_t page_oob_write_pio  (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
static uint32_t page_read_pio		   (chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t *buf);
static uint8_t	 status_get_pio		(chal_nand_info_t *ni, uint8_t bank);
static uint32_t id_get_pio		      (chal_nand_info_t *ni, uint8_t bank, uint8_t addr, uint8_t *buf);
static uint32_t block_erase_pio	   (chal_nand_info_t *ni, uint8_t bank, uint32_t block);
static uint32_t oob_read_pio        (chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t	*buf);
static uint32_t oob_write_pio       (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
static uint32_t param_read_pio		(chal_nand_info_t *ni, uint8_t bank, uint8_t *buf);
#endif

/* "normal" methods */
#ifndef __BROM_CODE__
static uint32_t page_write 		   (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
static uint32_t page_oob_write 	   (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
static uint32_t oob_write           (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf);
static uint32_t block_erase		   (chal_nand_info_t *ni, uint8_t bank, uint32_t block);
static uint8_t  status_get          (chal_nand_info_t *ni, uint8_t bank);
#endif
static uint32_t page_read           (chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t *buf);
static uint32_t id_get              (chal_nand_info_t *ni, uint8_t bank, uint8_t addr, uint8_t *buf);
static uint32_t oob_read            (chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t	*buf);
static uint32_t param_read          (chal_nand_info_t *ni, uint8_t bank, uint8_t *buf);


/*============================================================================*/

/* PRIVATE DATA		*/

#ifndef __BROM_CODE__
static const chal_nand_ops_t ops_pio = {
  status_get_pio,
  block_erase_pio,
  page_write_pio,
  oob_write_pio,
  page_oob_write_pio,
  id_get_pio,
  page_read_pio,
  oob_read_pio,
  param_read_pio
};
#endif

static const chal_nand_ops_t ops = {
#ifndef __BROM_CODE__
  status_get,
  block_erase,
  page_write,
  oob_write,
  page_oob_write,
#endif
  id_get,
  page_read,
  oob_read,
  param_read
};

/* convenience macros */
#if ( CHAL_NAND_DEBUG == 1 )
#define nand_rc(rc)	   (( rc == CHAL_NAND_RC_SUCCESS ) ? CHAL_NAND_RC_SUCCESS : ((__LINE__ << 8) | rc ))
#else
#define nand_rc(rc)	   (rc)
#endif

#define N_ELEMENTS(a)	(sizeof(a)/sizeof(a[0]))

void nand_ctrl_reset(void)
{
    CHAL_REG_WRITE32 (NAND_CTRL_CTRL, NAND_CTRL_PIO_INIT | NAND_CTRL_RST);
    /* disable and clear interrupts */
    CHAL_REG_WRITE32 (NAND_CTRL_IRQ_CTRL, 0);
    CHAL_REG_WRITE32 (NAND_CTRL_IRQ_STATUS, ~0x0);
    CHAL_REG_WRITE32 (NAND_CTRL_CTRL, NAND_CTRL_PIO_INIT);
}

static inline uint32_t mm_cmd_complete(uint32_t reg)
{
  int32_t i = CHAL_NAND_TIMEOUT_MM_CMD_COMPLETE;
  int32_t d = TIMER_TICK_ADJUSTED_DELAY(1);
  while((CHAL_REG_READ32(reg) & NAND_CTRL_CMD_VALID) && (i > 0)) {
    i-=d;
    udelay(d);
  }
  return (i > 0) ?  nand_rc(CHAL_NAND_RC_SUCCESS) : nand_rc(CHAL_NAND_RC_CMD_TOUT);
}

static inline uint32_t do_cmd(uint32_t cmd)
{
    CHAL_REG_WRITE32 (NAND_CTRL_MINSTR, cmd);
    return mm_cmd_complete(NAND_CTRL_MINSTR);
}

#define do_WC(opcode)		do_cmd(mk_instr_10(instr_WC,opcode))
#define do_WA(cycles)		do_cmd(mk_instr_10(instr_WA,cycles))
#define do_WD(bytes)		   do_cmd(mk_instr_10(instr_WD,bytes))
#define do_RD(bytes)		   do_cmd(mk_instr_10(instr_RD,bytes))
#define do_RS( )		      do_cmd(mk_instr_10(instr_RS,0x1))



/* all the "waits" in following macros are intended	*/
/* to alleviate some apparent HW shortcomings which 	*/
/* do cause timing problems in register writes: one	*/
/* can never be sure that by the time next instruction	*/
/* is executed the register was indeed loaded. 		*/
static inline void dma_start(void)
{
  CHAL_REG_WRITE32 (NAND_CTRL_CTRL, CHAL_REG_READ32 (NAND_CTRL_CTRL) | NAND_CTRL_DMA_START);
}

static inline void dma_stop(void)
{
  CHAL_REG_WRITE32 (NAND_CTRL_CTRL, CHAL_REG_READ32 ( NAND_CTRL_CTRL) & ~NAND_CTRL_DMA_START);
}

#define msleep(a) udelay(a * 1000)
/* wait usec for rb line. returns 1 for timeout, 0 for success */
static inline int rb_wait(int bank, int usec)
{
  int d = TIMER_TICK_ADJUSTED_DELAY(10);
  while (1)
  {
    if (CHAL_REG_READ32(NAND_CTRL_IRQ_STATUS) & (((0x1 << bank) << NAND_IRQ_RB_CHANGE_SHIFT) & NAND_IRQ_RB_CHANGE_MASK))
    {
       return 0;     /* ready */
    }
    if (usec <= 0)
    {
       return 1;     /* timeout */
    }
    if (usec > 10000)
    {
       int i;
       usec -= 10000;
       //msleep(10);
       for (i=0; i<10; i++)
          udelay(1000);
    }
    else
    {
       usec-=d;
       udelay(d);
    }
  }
}

static inline void rb_clear(void)
{
  CHAL_REG_WRITE32(NAND_CTRL_IRQ_STATUS, (CHAL_REG_READ32(NAND_CTRL_IRQ_STATUS) & NAND_IRQ_RB_CHANGE_MASK));
}

/* if it turns out ARM CPP supports 'variadic'  macros 	*/
/* I'll code it nicely, otherwise will stick to that	*/
#define NAND_OPS_CALL(ni,op,params)    (* CHAL_NAND_OPS(ni)->op ) params

#if ( CHAL_NAND_DEBUG == 1 )
#define ASSERT(a)       if (!(a)) while(1)
#else
#define ASSERT(a)
#endif

/*******************************************************************************
* PUBLIC INTERFACE IMPLEMENTATION
*******************************************************************************/

/*******************************************************************************
* chal_nand_timing(ni, tRP, tREH, tWP, tWH, tS, tH, tOE, diff_tCEA_tREA, tRHZ)
*     Sets the nand timing parameters in the ni structure
* @ni:               [in/out] nand info structure
* @tRP:              [in] tRP    (ns)
* @tREH:             [in] tREH   (ns)
* @tWP:              [in] tWP    (ns)
* @tWH:              [in] tWH    (ns)
* @tS:               [in] tS     (ns)
* @tH:               [in] tH     (ns)
* @tOE:              [in] tOE    (ns)
* @diff_tCEA_tREA:   [in] tCEA - tREA  (ns)
* @tRHZ:             [in] tRHZ   (ns)
*******************************************************************************/
void chal_nand_timing (
   chal_nand_info_t *ni,
   uint8_t	tRP,
   uint8_t	tREH,
   uint8_t	tWP,
   uint8_t	tWH,
   uint8_t	tS,
   uint8_t	tH,
   uint8_t	tOE,
   uint8_t	diff_tCEA_tREA,
   uint8_t	tRHZ)
{
   CHAL_NAND_TIMING_CONF1(ni) =
     (((tRP / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF1_TRP_SHIFT) & NAND_CTRL_CONF1_TRP_MASK) |
     (((tREH / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF1_TREH_SHIFT) & NAND_CTRL_CONF1_TREH_MASK) |
     (((tWP / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF1_TWP_SHIFT) & NAND_CTRL_CONF1_TWP_MASK) |
     (((tWH / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF1_TWH_SHIFT) & NAND_CTRL_CONF1_TWH_MASK) |
     (((tS / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF1_TS_SHIFT) & NAND_CTRL_CONF1_TS_MASK) |
     (((tH / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF1_TH_SHIFT) & NAND_CTRL_CONF1_TH_MASK);

   CHAL_NAND_TIMING_CONF2(ni) =
     (((tOE / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF2_TOE_SHIFT) & NAND_CTRL_CONF2_TOE_MASK) |
     (((diff_tCEA_tREA / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF2_TCEA_TREA_SHIFT) & NAND_CTRL_CONF2_TCEA_TREA_MASK) |
     (((tRHZ / CHAL_NAND_TIMING_STEP) << NAND_CTRL_CONF2_TRHZ_SHIFT) & NAND_CTRL_CONF2_TRHZ_MASK);
}

/*******************************************************************************
* chal_nand_init(ni, flags, dma_setup_cb) - Initialize nand interface
* @ni:            [in/out] nand info structure
* @flags:         [in] options
*******************************************************************************/
uint32_t chal_nand_init (chal_nand_info_t *ni,
                         uint8_t flags)

{
  static uint8_t init = 0;
  uint8_t id[CHAL_NAND_ID_SIZE];
  uint8_t param_page[0x200];  /* ONFI parameter page buffer */ 
  uint32_t bank;
  uint32_t rc = nand_rc(CHAL_NAND_RC_FAILURE);
  uint32_t lrc = 0;
  uint32_t i;
  uint8_t onfi;

  if (ni == NULL) {
    return rc;
  }

  if ((flags & CHAL_NAND_FLAG_DMA) && (CHAL_NAND_DMA_SETUP_CB(ni) == NULL)) {
      return rc;
  }

  if ( !init || (CHAL_NAND_FLAG_FORCE & flags)){

    nand_pshare_set ();
/*
 * 	clocks_enable (CMGR_NANDCTL_AHB_CLK_EN);
 * 	clocks_enable (CMGR_NANDCTL_CLK_EN);
 */

    /* Reset NAND module */
/*
 * 	CHAL_REG_WRITE32(RSTMGR_LOCK, RSTMGR_LOCK_UNLOCK_TAG);
 * 	CHAL_REG_WRITE32(RSTMGR_SFTRSTNCLRA, 0x100);
 * 	CHAL_REG_WRITE32(RSTMGR_SFTRSTNSETA, 0x100);
 *     CHAL_REG_WRITE32(RSTMGR_LOCK, RSTMGR_LOCK_LOCK_TAG);
 * 
 */

    CHAL_NAND_FLAGS(ni) = flags;

    nand_ctrl_reset();

    /* AUX 8 bytes, setup busw after id stage */
    CHAL_REG_WRITE32 (NAND_CTRL_CONF0, NAND_CTRL_CONF0_GOLD);

    if ((CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_TIMING) == 0) {
       CHAL_NAND_TIMING_CONF1(ni) = NAND_CTRL_CONF1_GOLD;
       CHAL_NAND_TIMING_CONF2(ni) = NAND_CTRL_CONF2_GOLD;
    }
    CHAL_REG_WRITE32 (NAND_CTRL_CONF1, CHAL_NAND_TIMING_CONF1(ni));
    CHAL_REG_WRITE32 (NAND_CTRL_CONF2, CHAL_NAND_TIMING_CONF2(ni));

#ifndef __BROM_CODE__
    if ((CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC) || ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_DMA) ) {
#endif

      /* force DMA mode of just ECC was requested */
      CHAL_NAND_FLAGS(ni) |= CHAL_NAND_FLAG_DMA   ;

      CHAL_NAND_UC_P(ni) = (uint32_t *)NAND_CTRL_MCODE_START;

      lrc |= uc_load_function (ni, CHAL_NAND_UC_RESET, uc_RESET, N_ELEMENTS(uc_RESET));
      lrc |= uc_load_function (ni, CHAL_NAND_UC_ID_GET, uc_ID_GET, N_ELEMENTS(uc_ID_GET));
      lrc |= uc_load_function (ni, CHAL_NAND_UC_PARAM_READ_PRE, uc_PARAM_READ_PRE, N_ELEMENTS(uc_PARAM_READ_PRE));
      lrc |= uc_load_function (ni, CHAL_NAND_UC_PARAM_READ, uc_PARAM_READ, N_ELEMENTS(uc_PARAM_READ));

      if (CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC)
#if ( CHAL_NAND_ECC_HM == 1 )
        CHAL_REG_WRITE32 (NAND_CTRL_CTRL, NAND_CTRL_NORMAL_INIT);
#else
        CHAL_REG_WRITE32 (NAND_CTRL_CTRL, (NAND_CTRL_NORMAL_INIT | NAND_CTRL_ECC_HM_DISABLE));
#endif
      else
        CHAL_REG_WRITE32 (NAND_CTRL_CTRL, (NAND_CTRL_NORMAL_INIT | NAND_CTRL_ECC_RS_DISABLE | NAND_CTRL_ECC_HM_DISABLE));


      CHAL_NAND_OPS(ni) = &ops;

#ifndef __BROM_CODE__
    } else {
      CHAL_REG_WRITE32 (NAND_CTRL_CTRL, NAND_CTRL_PIO_INIT);
      CHAL_NAND_OPS(ni) = &ops_pio;
    }
#endif


    /* discover bank and configuration */
    for (bank = 0; bank< CHAL_NAND_BANKS_MAX; bank++) {
      if ((rc = chal_nand_id_get (ni, bank, 0, id)) != nand_rc(CHAL_NAND_RC_SUCCESS)) {
        break;
      }
      /* save ID of the first bank --- they all MUST be the same */
      if (bank == 0) {
        for (i = 0; i < CHAL_NAND_ID_SIZE; i++) {
          (CHAL_NAND_ID(ni))[i] = id[i];
        }
      }
      else
      {
        /* all NAND chips must be the same */
        for (i = 0; i < CHAL_NAND_ID_BYTES; i++) {
          if ((CHAL_NAND_ID(ni))[i] != id[i]) {
             return nand_rc (CHAL_NAND_RC_BANK_CFG_ERR);
          }
        }

      }
    }

    if(bank == 0) {
      return rc;
    }

    CHAL_NAND_BANKS(ni) = bank;

    /* check if supports ONFI */
    onfi = 0;
    if ((rc = chal_nand_id_get (ni, 0, 0x20, id)) == nand_rc(CHAL_NAND_RC_SUCCESS)) {
       if ((id[0]=='O')&&(id[1]=='N')&&(id[2]=='F')&&(id[3]=='I'))
       {
          if ((rc = chal_nand_param_read (ni, 0, param_page)) == nand_rc(CHAL_NAND_RC_SUCCESS)) {
              /* validated CRC */ 
              if (crc16_valid(param_page, 254))
              {
                  onfi = 1;
              }
              else if (crc16_valid(param_page + 256, 254))
              {
                  onfi = 1;
              }
          }
       }
    }

    if ((CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_GEOMETRY) == 0) {
      if (onfi) {
         nand_cfg_onfi(ni, param_page);
      }
      else {
         /* parse the id of first bank to determine configuration */
         nand_cfg_geometry(ni);
      }
    }

    if (CHAL_NAND_BUS_WIDTH(ni) == 16)
      CHAL_REG_WRITE32(NAND_CTRL_CONF0, NAND_CTRL_CONF0_GOLD | NAND_CTRL_CONF0_BWIDTH_16);

/*
    printf("\nNAND %d bit: %s %s\n"
           "ID %02X%02X%02X%02X%02X\n"
           "banks %d\n"
           "bank size %d MB\n"
           "page size %d KB\n"
           "block size %d KB\n"
           "oob size %d bytes\n"
           "aux data size %d bytes\n",
           CHAL_NAND_BUS_WIDTH(ni),
           (CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_DMA) ? "DMA" : "",
#if ( CHAL_NAND_ECC_HM == 1 )
           (CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC) ? "ECC-RS ECC-HM" : "",
#else
           (CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC) ? "ECC-RS" : "",
#endif
           (CHAL_NAND_ID(ni))[0],
           (CHAL_NAND_ID(ni))[1],
           (CHAL_NAND_ID(ni))[2],
           (CHAL_NAND_ID(ni))[3],
           (CHAL_NAND_ID(ni))[4]
           CHAL_NAND_BANKS(ni),
           0x1 << (CHAL_NAND_BANK_SHIFT(ni) - 20),
           CHAL_NAND_PAGE_SIZE(ni) >> 10,
           CHAL_NAND_BLOCK_SIZE(ni) >> 10,
           CHAL_NAND_OOB_SIZE(ni),
           CHAL_NAND_AUX_DATA_SIZE(ni)); 
*/

    rc = nand_rc(CHAL_NAND_RC_SUCCESS);

    if ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_DMA) {

      lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_READ_PRE, uc_PAGE_READ_PRE, N_ELEMENTS(uc_PAGE_READ_PRE));
#ifndef __BROM_CODE__
      lrc |= uc_load_function (ni, CHAL_NAND_UC_STATUS_GET, uc_STATUS_GET, N_ELEMENTS(uc_STATUS_GET));
      lrc |= uc_load_function (ni, CHAL_NAND_UC_BLOCK_ERASE, uc_BLOCK_ERASE, N_ELEMENTS(uc_BLOCK_ERASE));
#endif

      switch  (CHAL_NAND_PAGE_SIZE(ni))  {
      case 0x800:
        if ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC) {
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_READ_ECC, uc_PAGE_READ_ECC, N_ELEMENTS(uc_PAGE_READ_ECC));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_READ_ECC, uc_OOB_READ_ECC, N_ELEMENTS(uc_OOB_READ_ECC));
#ifndef __BROM_CODE__
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_WRITE_ECC, uc_PAGE_WRITE_ECC, N_ELEMENTS(uc_PAGE_WRITE_ECC));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_WRITE_ECC, uc_OOB_WRITE_ECC, N_ELEMENTS(uc_OOB_WRITE_ECC));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_OOB_WRITE_ECC, uc_PAGE_OOB_WRITE_ECC, N_ELEMENTS(uc_PAGE_OOB_WRITE_ECC));
#endif
        }
        else {
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_READ, uc_PAGE_READ, N_ELEMENTS(uc_PAGE_READ));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_READ, uc_OOB_READ, N_ELEMENTS(uc_OOB_READ));
#ifndef __BROM_CODE__
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_WRITE, uc_PAGE_WRITE, N_ELEMENTS(uc_PAGE_WRITE));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_WRITE, uc_OOB_WRITE, N_ELEMENTS(uc_OOB_WRITE));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_OOB_WRITE, uc_PAGE_OOB_WRITE, N_ELEMENTS(uc_PAGE_OOB_WRITE));
#endif
        }
        break;
      case 0x1000:
        if ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC) {
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_READ_ECC_4K, uc_PAGE_READ_ECC_4K, N_ELEMENTS(uc_PAGE_READ_ECC_4K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_READ_ECC_4K, uc_OOB_READ_ECC_4K, N_ELEMENTS(uc_OOB_READ_ECC_4K));
#ifndef __BROM_CODE__
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_WRITE_ECC_4K, uc_PAGE_WRITE_ECC_4K, N_ELEMENTS(uc_PAGE_WRITE_ECC_4K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_WRITE_ECC_4K, uc_OOB_WRITE_ECC_4K, N_ELEMENTS(uc_OOB_WRITE_ECC_4K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_OOB_WRITE_ECC_4K, uc_PAGE_OOB_WRITE_ECC_4K, N_ELEMENTS(uc_PAGE_OOB_WRITE_ECC_4K));
#endif
        }
        else {
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_READ_4K, uc_PAGE_READ_4K, N_ELEMENTS(uc_PAGE_READ_4K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_READ_4K, uc_OOB_READ_4K, N_ELEMENTS(uc_OOB_READ_4K));
#ifndef __BROM_CODE__
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_WRITE_4K, uc_PAGE_WRITE_4K, N_ELEMENTS(uc_PAGE_WRITE_4K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_WRITE_4K, uc_OOB_WRITE_4K, N_ELEMENTS(uc_OOB_WRITE_4K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_OOB_WRITE_4K, uc_PAGE_OOB_WRITE_4K, N_ELEMENTS(uc_PAGE_OOB_WRITE_4K));
#endif
        }
        break;
      case 0x2000:
        if ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC) {
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_READ_ECC_8K, uc_PAGE_READ_ECC_8K, N_ELEMENTS(uc_PAGE_READ_ECC_8K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_READ_ECC_8K, uc_OOB_READ_ECC_8K, N_ELEMENTS(uc_OOB_READ_ECC_8K));
#ifndef __BROM_CODE__
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_WRITE_ECC_8K, uc_PAGE_WRITE_ECC_8K, N_ELEMENTS(uc_PAGE_WRITE_ECC_8K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_WRITE_ECC_8K, uc_OOB_WRITE_ECC_8K, N_ELEMENTS(uc_OOB_WRITE_ECC_8K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_OOB_WRITE_ECC_8K, uc_PAGE_OOB_WRITE_ECC_8K, N_ELEMENTS(uc_PAGE_OOB_WRITE_ECC_8K));
#endif
        }
        else {
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_READ_8K, uc_PAGE_READ_8K, N_ELEMENTS(uc_PAGE_READ_8K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_READ_8K, uc_OOB_READ_8K, N_ELEMENTS(uc_OOB_READ_8K));
#ifndef __BROM_CODE__
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_WRITE_8K, uc_PAGE_WRITE_8K, N_ELEMENTS(uc_PAGE_WRITE_8K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_OOB_WRITE_8K, uc_OOB_WRITE_8K, N_ELEMENTS(uc_OOB_WRITE_8K));
          lrc |= uc_load_function (ni, CHAL_NAND_UC_PAGE_OOB_WRITE_8K, uc_PAGE_OOB_WRITE_8K, N_ELEMENTS(uc_PAGE_OOB_WRITE_8K));
#endif
        }
        break;
      default:
        /* printf ("BRCM NAND: unsupported page size %u\n",CHAL_NAND_PAGE_SIZE(ni)); */
        rc = nand_rc (CHAL_NAND_RC_PAGE_SIZE_ERR);
        break;

      } /* switch */

      /* check if any micro-code function failed to load */
      if (lrc)
      {
         rc = nand_rc (CHAL_NAND_RC_NOMEM);
      }

    } /* if DMA */

    if (rc == nand_rc(CHAL_NAND_RC_SUCCESS)) {
       init = 1;
    }
  }

  return rc ;
}

/*******************************************************************************
* chal_nand_id_get(ni, bank, buf) - Get nand id
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @addr:	   [in] address (0x0 or 0x20)
* @buf:	   [out] buffer to get the id
*******************************************************************************/
uint32_t chal_nand_id_get (chal_nand_info_t *ni,
                           uint8_t bank,
                           uint8_t addr,
                           uint8_t *buf)
{
   return (NAND_OPS_CALL(ni,id_get,(ni,bank,addr,buf)));
}

/*******************************************************************************
* chal_nand_page_read(ni, bank, page, buf) - Reads one page from nand into buf
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [out] target buffer
*******************************************************************************/
uint32_t chal_nand_page_read (chal_nand_info_t *ni,
                              uint8_t bank,
                              uint32_t page,
                              uint8_t *buf)
{
    return NAND_OPS_CALL(ni,page_read,(ni,bank,page,buf));
}

/*******************************************************************************
* chal_nand_oob_read(ni, bank, page, buf) - Reads the auxiliary data from oob
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [out] target buffer
*******************************************************************************/
uint32_t chal_nand_oob_read (chal_nand_info_t *ni,
                             uint8_t bank,
                             uint32_t page,
                             uint8_t *buf)
{
   return (NAND_OPS_CALL(ni,oob_read,(ni,bank,page,buf)));
}

/*******************************************************************************
* chal_nand_block_isbad(ni, bank, block, is_bad) - Checks if block is marked bad
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
* @is_bad:  [out] 1 if block is marked bad, 0 if good
*******************************************************************************/
uint32_t chal_nand_block_isbad (chal_nand_info_t *ni,
                                uint8_t bank,
                                uint32_t block,
                                uint8_t *is_bad)
{
   uint8_t oob[CHAL_NAND_OOB_MAX_SIZE];
   uint32_t rc, i, page;
   uint32_t off[3];
   uint32_t flags;

   /* check first second and last page of the block */
   off[0] = 0;
   off[1] = 1;
   off[2] = (1<<(CHAL_NAND_BLOCK_SHIFT(ni)-CHAL_NAND_PAGE_SHIFT(ni))) - 1;

	*is_bad = 0;

   page = block<<(CHAL_NAND_BLOCK_SHIFT(ni)-CHAL_NAND_PAGE_SHIFT(ni));

   /* temporary bypass ECC */
   flags = CHAL_NAND_FLAGS(ni);
   CHAL_NAND_FLAGS(ni) &= ~CHAL_NAND_FLAG_ECC;

   for (i = 0; i < 3; i++) {

      rc = chal_nand_oob_read(ni, bank, page+off[i], oob);

      if (rc != CHAL_NAND_RC_SUCCESS) {
	      *is_bad = 1;
  	      break;
      }

      if (oob[NAND_BADBLOCK_MARKER_POS] != NAND_BADBLOCK_MARKER_GOOD) {
	      *is_bad = 1;
  	      break;
      }
   }

   /* Restore original flags */
   CHAL_NAND_FLAGS(ni) = flags;

   return rc;
}

/*******************************************************************************
* chal_nand_param_read(ni, bank, buf) - Reads ONFI parameter page into buf
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @buf:     [out] target buffer
*******************************************************************************/
uint32_t chal_nand_param_read (chal_nand_info_t *ni,
                               uint8_t bank,
                               uint8_t *buf)
{
   return  (NAND_OPS_CALL(ni,param_read,(ni,bank,buf)));
}

#ifndef __BROM_CODE__

/*******************************************************************************
* chal_nand_block_force_erase(ni, bank, block) - Erase block ignoring bad block
*                                                marks
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
*******************************************************************************/
uint32_t  chal_nand_block_force_erase (chal_nand_info_t *ni,
                                       uint8_t bank,
                                       uint32_t block)
{
   return (NAND_OPS_CALL(ni,block_erase,(ni,bank,block)));
}

/*******************************************************************************
* chal_nand_page_write(ni, bank, page, buf) - Writes page from buf to the nand
*                                             device
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [in] source buffer
*******************************************************************************/
uint32_t chal_nand_page_write (chal_nand_info_t *ni,
                               uint8_t bank,
                               uint32_t page,
                               uint8_t *buf)
{
   return (NAND_OPS_CALL(ni,page_write,(ni,bank,page,buf)));
}

/******************************************************************************
* chal_nand_oob_write (ni, bank, page, buf) - Writes the auxiliary data to oob
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [in] source buffer
*******************************************************************************/
uint32_t chal_nand_oob_write (chal_nand_info_t *ni,
                              uint8_t bank,
                              uint32_t page,
                              uint8_t *buf)
{
	return (NAND_OPS_CALL(ni,oob_write,(ni,bank,page,buf)));
}

/*******************************************************************************
* chal_nand_page_oob_write(ni, bank, page, buf) - Write page and oob from buf to
*                                                 nand device
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [in] source buffer
*******************************************************************************/
uint32_t chal_nand_page_oob_write (chal_nand_info_t *ni,
                                   uint8_t bank,
                                   uint32_t page,
                                   uint8_t *buf)
{
   return (NAND_OPS_CALL(ni,page_oob_write,(ni,bank,page,buf)));
}

/*******************************************************************************
* chal_nand_block_erase(ni, bank, block)  - Erase a block
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
*******************************************************************************/
uint32_t chal_nand_block_erase (chal_nand_info_t *ni,
                                uint8_t bank,
                                uint32_t block)
{
   uint32_t rc;
   uint8_t isbad;

   rc = chal_nand_block_isbad(ni, bank, block, &isbad);
   if (rc != CHAL_NAND_RC_SUCCESS)
	   return rc;
   if (isbad) {
  	   return CHAL_NAND_RC_BB_NOERASE;
   }
 
   return chal_nand_block_force_erase(ni, bank, block);
}

/*******************************************************************************
* chal_nand_block_markbad(ni, bank, block) - Erase block and write the bad block
*                                            marker in oob
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
*******************************************************************************/
uint32_t chal_nand_block_markbad (chal_nand_info_t *ni,
                                  uint8_t bank,
                                  uint32_t block)
{
   uint8_t  oob[CHAL_NAND_OOB_MAX_SIZE];
   uint8_t  isbad;
	uint32_t	i, rc, page;
   uint32_t off[3];
	uint32_t	flags;

   /* erase block first */
   chal_nand_block_force_erase(ni, bank, block);

   /* mark first second and last page of the block */
   off[0] = 0;
   off[1] = 1;
   off[2] = (1<<(CHAL_NAND_BLOCK_SHIFT(ni)-CHAL_NAND_PAGE_SHIFT(ni))) - 1;

	for (i = 0; i < CHAL_NAND_AUX_DATA_SIZE(ni) ; i++) {
		oob[i] = 0xFF;
	}
	oob[NAND_BADBLOCK_MARKER_POS] = NAND_BADBLOCK_MARKER_BAD;

   page = block<<(CHAL_NAND_BLOCK_SHIFT(ni)-CHAL_NAND_PAGE_SHIFT(ni));

   /* temporary bypass ECC */
   flags = CHAL_NAND_FLAGS(ni);
   CHAL_NAND_FLAGS(ni) &= ~CHAL_NAND_FLAG_ECC;

   for (i = 0; i < 3; i++) {
	   chal_nand_oob_write(ni, bank, page+off[i], oob);
   }

   /* Restore original flags */
   CHAL_NAND_FLAGS(ni) = flags;

   rc = chal_nand_block_isbad(ni, bank, block, &isbad);
   if ((rc != CHAL_NAND_RC_SUCCESS) || !isbad) {
  	   return CHAL_NAND_RC_FAILURE;
   }

   return CHAL_NAND_RC_SUCCESS;
}

#endif

/*******************************************************************************
* PPRIVATE INTERFACE IMPLEMENTATION
*******************************************************************************/

uint32_t id_get_pio(chal_nand_info_t *ni, uint8_t bank, uint8_t addr, uint8_t *buf)
{
  uint16_t *id;

  (void) ni;

  rb_clear();

  CHAL_REG_CLRBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 (NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 (NAND_CTRL_MADDR0, 0x0);

  do_WC (NAND_CMD_RESET); /* reset the bank */

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_RESET)) {
    return nand_rc(CHAL_NAND_RC_RB_TOUT);
  }

  CHAL_REG_WRITE32 (NAND_CTRL_MADDR0, addr);

  /* get chip ID 0x90, 0x00 -- read 2 words , 5 bytes actual */
  do_WC(NAND_CMD_ID);
  do_WA(0x1);	/* number of cycles, data from MADDR0 or at least I hope so :-) */
  do_RD(0x8);	/* 6 cycles to read ID data, 8 total to figure where it wraps around */

  /* this forced cast should be ok as the buffer is word aligned */
  id = (uint16_t *)buf;

  *id++ = CHAL_REG_READ16 ( NAND_CTRL_RD_FIFO );
  *id++ = CHAL_REG_READ16 ( NAND_CTRL_RD_FIFO );
  *id++ = CHAL_REG_READ16 ( NAND_CTRL_RD_FIFO );
  *id++ = CHAL_REG_READ16 ( NAND_CTRL_RD_FIFO );

  return nand_rc(CHAL_NAND_RC_SUCCESS);
}

#ifndef __BROM_CODE__

/********************************************************************************/
/* "PIO" implementation 							*/
/********************************************************************************/

static uint8_t status_get_pio (chal_nand_info_t *ni, uint8_t bank)
{
  uint8_t res;

  (void) ni;

  CHAL_REG_CLRBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 (NAND_CTRL_BANK, bank & 0x7);

  do_WC(NAND_CMD_STATUS);

  do_RD(0x2);	/* 2 cycle to read status data, 2 total as controller seems to be 16'b */

  res = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);

  return ( res );

}

static uint32_t page_read_pio(chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t *buf)
{
  uint32_t len	= CHAL_NAND_PAGE_SIZE(ni);
  uint16_t *p;

  rb_clear();

  CHAL_REG_CLRBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 ( NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR0, (page << 16) );
  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR1, (page >> 16) );

  do_WC ( NAND_CMD_READ_1ST);	/* read command 	*/
  do_WA ( 0x5);			/* 5 address cycles 	*/
  do_WC ( NAND_CMD_READ_2ND);	/* read command 	*/

  /* need a delay here */
  if(rb_wait(bank, CHAL_NAND_TIMEOUT_READ)) {
    return nand_rc(CHAL_NAND_RC_RB_TOUT);
  }

  /* this forced cast should be ok as the buffer is word aligned */
  p = (uint16_t *)buf;
  while ( len ) {

    len -= 0x8;

    do_RD (0x8);	/* FIFO depth ??? */

    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);

  }

  return ( !len ? nand_rc(CHAL_NAND_RC_SUCCESS): nand_rc(CHAL_NAND_RC_FAILURE));
}


static uint32_t oob_read_pio(chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t *buf)
{
  uint32_t len	= CHAL_NAND_AUX_DATA_SIZE(ni);
  uint16_t *p;

  rb_clear();

  CHAL_REG_CLRBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 ( NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR0, (page << 16) );
  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR1, (page >> 16) );

  do_WC ( NAND_CMD_READ_1ST);	/* read command 	*/
  do_WA ( 0x5);			/* 5 address cycles 	*/
  do_WC ( NAND_CMD_READ_2ND);	/* read command 	*/

  /* need a delay here */
  if(rb_wait(bank, CHAL_NAND_TIMEOUT_READ)) {
    return nand_rc(CHAL_NAND_RC_RB_TOUT);
  }

  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR0, CHAL_NAND_PAGE_SIZE(ni) ); /* OOB offset */

  do_WC ( NAND_CMD_READ_RAND_1ST);	/* read random command 	*/
  do_WA ( 0x2);			/* 2 address cycles 	*/
  do_WC ( NAND_CMD_READ_RAND_2ND);	/* read random command 	*/

  /* this forced cast should be ok as the buffer is word aligned */
  p = (uint16_t *)buf;
  while ( len ) {

    len -= 0x8;

    do_RD (0x8);	/* FIFO depth ??? */

    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);

  }

  return ( !len ? nand_rc(CHAL_NAND_RC_SUCCESS): nand_rc(CHAL_NAND_RC_FAILURE));
}


uint32_t block_erase_pio (chal_nand_info_t *ni, uint8_t bank, uint32_t block)
{
  uint8_t status;

  CHAL_REG_CLRBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 ( NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR0, block << (CHAL_NAND_BLOCK_SHIFT(ni) - CHAL_NAND_PAGE_SHIFT(ni)) );

  do_WC ( NAND_CMD_BERASE_1ST);
  do_WA ( 0x3 );
  do_WC ( NAND_CMD_BERASE_2ND);

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_ERASE)) {
    return nand_rc(CHAL_NAND_RC_RB_TOUT);
  }

  do {
    status = nand_status_get (ni, bank);
  } while ( ! (status & NAND_STATUS_READY));

  return (status & NAND_STATUS_FAIL ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS));
}

static uint32_t page_write_pio (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf)
{
  uint32_t len	= CHAL_NAND_PAGE_SIZE(ni);
  uint8_t status;
  uint16_t *p;

  CHAL_REG_SETBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 ( NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR0, (page << 16) );
  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR1, (page >> 16) );

  do_WC (NAND_CMD_PROG_1ST);
  do_WA (0x5);

  /* this forced cast should be ok as the buffer is word aligned */
  p = (uint16_t *)buf;
  while ( len ) {

    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);

    do_WD (0x8);	/* FIFO depth ??? */

    len -= 8;

  } /* while */

  do_WC (NAND_CMD_PROG_2ND);

  do {
    status = nand_status_get (ni, bank);
  } while ( ! (status & NAND_STATUS_READY));


  return (status & NAND_STATUS_FAIL  ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS) ) ;

}


static uint32_t oob_write_pio (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf)
{
  uint32_t len	= CHAL_NAND_AUX_DATA_SIZE(ni);
  uint8_t status;
  uint16_t *p;

  CHAL_REG_SETBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 ( NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR0, ((page << 16) | CHAL_NAND_PAGE_SIZE(ni)) ); /* oob offset */
  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR1, (page >> 16) );

  do_WC (NAND_CMD_PROG_1ST);
  do_WA (0x5);

  /* this forced cast should be ok as the buffer is word aligned */
  p = (uint16_t *)buf;
  while ( len ) {

    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);

    do_WD (0x8);	/* FIFO depth ??? */

    len -= 8;

  } /* while */

  do_WC (NAND_CMD_PROG_2ND);

  do {
    status = nand_status_get (ni, bank);
  } while ( ! (status & NAND_STATUS_READY));

  return (status & NAND_STATUS_FAIL  ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS) ) ;
}


static uint32_t page_oob_write_pio (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf)
{
  uint32_t len	= CHAL_NAND_PAGE_SIZE(ni) + CHAL_NAND_AUX_DATA_SIZE(ni);
  uint8_t status;
  uint16_t *p;

  CHAL_REG_SETBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 ( NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR0, (page << 16) );
  CHAL_REG_WRITE32 ( NAND_CTRL_MADDR1, (page >> 16) );

  do_WC (NAND_CMD_PROG_1ST);
  do_WA (0x5);

  /* this forced cast should be ok as the buffer is word aligned */
  p = (uint16_t *)buf;
  while ( len ) {

    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);
    CHAL_REG_WRITE16 (NAND_CTRL_WR_FIFO,*p++);

    do_WD (0x8);	/* FIFO depth ??? */

    len -= 8;

  } /* while */

  do_WC (NAND_CMD_PROG_2ND);

  do {
    status = nand_status_get (ni, bank);
  } while ( ! (status & NAND_STATUS_READY));

  return (status & NAND_STATUS_FAIL  ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS) ) ;
}

static uint32_t param_read_pio(chal_nand_info_t *ni, uint8_t bank, uint8_t *buf)
{
  uint32_t len	= 0x200;
  uint16_t *p;

  (void) ni;

  rb_clear();

  CHAL_REG_CLRBIT32 (NAND_CTRL_CTRL, NAND_CTRL_2NAND);

  CHAL_REG_WRITE32 (NAND_CTRL_BANK, bank & 0x7);

  CHAL_REG_WRITE32 (NAND_CTRL_MADDR0, 0);
  CHAL_REG_WRITE32 (NAND_CTRL_MADDR1, 0);

  do_WC (NAND_CMD_READ_PARAM);	/* read parameter page command 0xEC	*/
  do_WA (0x1);			/* 1 address cycle 	*/

  /* need a delay here */
  if(rb_wait(bank, CHAL_NAND_TIMEOUT_READ)) {
    return nand_rc(CHAL_NAND_RC_RB_TOUT);
  }

  /* this forced cast should be ok as the buffer is word aligned */
  p = (uint16_t *)buf;
  while ( len ) {

    len -= 0x8;

    do_RD (0x8);	/* FIFO depth ??? */

    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);
    *p++ = CHAL_REG_READ16 (NAND_CTRL_RD_FIFO);

  }

  return ( !len ? nand_rc(CHAL_NAND_RC_SUCCESS): nand_rc(CHAL_NAND_RC_FAILURE));
}

#endif

/********************************************************************************/
/* "NORMAL"  implementation 							*/
/********************************************************************************/

/**
 * id_get - gets the nand id information
 * @ni:     [in] nand info structure
 * @bank:	[in] bank to get id info
 * @addr:   [in] address (0x0 or 0x20)
 * @buf: 	[out] buffer to get the id
 */
uint32_t id_get (chal_nand_info_t *ni, uint8_t bank, uint8_t addr, uint8_t *buf)
{
  uint32_t rc;

  ASSERT((((uint32_t)buf) % CHAL_NAND_DMA_ALIGN) == 0);

  rb_clear();
  rc = nand_do_cmd (ni, CHAL_NAND_UC_RESET,bank,0,NULL);

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_RESET)) {
    return ((rc != nand_rc (CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
  }

  if (rc != nand_rc (CHAL_NAND_RC_SUCCESS)) {
    return rc;
  }

  return nand_do_cmd (ni, CHAL_NAND_UC_ID_GET,bank,addr,(void*)buf);
}


/**
 * page_read - read one page
 * @ni:     [in] nand info structure
 * @bank: 	[in] select the bank
 * @page: 	[in] page to read
 * @buf:	[out] target buffer
 */
static uint32_t page_read(chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t *buf)
{
  uint32_t rc;

  ASSERT((((uint32_t)buf) % CHAL_NAND_DMA_ALIGN) == 0);

  rb_clear();
  rc = nand_do_cmd (ni, CHAL_NAND_UC_PAGE_READ_PRE, bank,page,NULL);

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_READ)) {
    return ((rc != nand_rc (CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
  }

  if (rc != nand_rc (CHAL_NAND_RC_SUCCESS)) {
    return rc;
  }

  rc = nand_do_cmd (
                       ni,
                       CHAL_NAND_UC_PAGE_READ,
                       bank,
                       page,
                       (void*)buf
                   );

  (CHAL_NAND_ECC_STATS(ni))[bank] = CHAL_REG_READ32 (NAND_CTRL_ECC_STATUS + (bank << 2));

  return ( rc ) ;
}


/**
 * oob_read - read the auxiliary data from oob
 * @ni:     [in] nand info structure
 * @bank:	[in] selected bank
 * @page:	[in] selected page
 * @buf:	[out] target buffer
 */
static uint32_t oob_read(chal_nand_info_t *ni, uint8_t bank, uint32_t page, uint8_t	*buf )
{
  uint32_t rc;

  ASSERT((((uint32_t)buf) % CHAL_NAND_DMA_ALIGN) == 0);

  rb_clear();	
  rc = nand_do_cmd (ni, CHAL_NAND_UC_PAGE_READ_PRE, bank,page,NULL);

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_READ)) {
    return ((rc != nand_rc (CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
  }

  if ( rc != nand_rc (CHAL_NAND_RC_SUCCESS)) {
    return rc;
  }

  return nand_do_cmd (
                         ni,
                         CHAL_NAND_UC_OOB_READ,
                         bank,
                         page,
                         (void*)buf
                     );
}

/**
 * param_read - read ONFI parameter page
 * @ni:     [in] nand info structure
 * @bank: 	[in] select the bank
 * @buf:	   [out] target buffer
 */
static uint32_t param_read(chal_nand_info_t *ni, uint8_t bank, uint8_t *buf)
{
  uint32_t rc;

  ASSERT((((uint32_t)buf) % CHAL_NAND_DMA_ALIGN) == 0);

  rb_clear();
  rc = nand_do_cmd (ni, CHAL_NAND_UC_PARAM_READ_PRE, bank,0,NULL);

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_READ)) {
    return ((rc != nand_rc (CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
  }

  if (rc != nand_rc (CHAL_NAND_RC_SUCCESS)) {
    return rc;
  }

  rc = nand_do_cmd (
                       ni,
                       CHAL_NAND_UC_PARAM_READ,
                       bank,
                       0,
                       buf
                   );
 
  return ( rc ) ;
}

#ifndef __BROM_CODE__

/** 
 * status_get - gets the status for an individual bank
 * @ni:     [in] nand info structure
 * @bank:	[in] bank to get status
 */
uint8_t status_get (chal_nand_info_t *ni, uint8_t bank)
{
  uint32_t rc;
  uint8_t buf[8]; /* this buffer does not need to be aligned because reading status is not using DMA */

  rc =  nand_do_cmd (ni,CHAL_NAND_UC_STATUS_GET,bank,0,(void*)buf);
  return ( uint8_t ) (rc == nand_rc(CHAL_NAND_RC_SUCCESS)? buf[0] : ~0);
}


/**
 * block_erase - erase one block for selected bank
 * @ni:     [in] nand info structure
 * @bank: 	[in] selected bank
 * @block:	[in] block to erase
 */
uint32_t block_erase(chal_nand_info_t *ni, uint8_t bank, uint32_t block)
{
  uint32_t rc;
  uint8_t status;

  rb_clear();
  /* uses 'page' to pass block address with proper shift to A[18:30] or A[19:30] depending on block size */
  rc = nand_do_cmd (ni, CHAL_NAND_UC_BLOCK_ERASE,bank, (block<< ( CHAL_NAND_BLOCK_SHIFT(ni) - CHAL_NAND_PAGE_SHIFT(ni))), NULL);

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_ERASE)) {
    return ((rc !=  nand_rc ( CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
  }

  if (rc != nand_rc ( CHAL_NAND_RC_SUCCESS) ) {
    return rc;
  }

  do {
    status = nand_status_get (ni, bank);
  } while ( ! (status & NAND_STATUS_READY));

  return ((status & NAND_STATUS_FAIL ) ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS));
}


/**
 * oob_write - writes the auxiliary data to the OOB
 * @ni:     [in] nand info structure
 * @bank:	[in] selected bank
 * @page:	[in] selected page
 * @buf:	[in] source buffer
 */
static uint32_t oob_write(chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf)
{
	uint32_t rc;
	uint8_t status;

   ASSERT((((uint32_t)buf) % CHAL_NAND_DMA_ALIGN) == 0);

	rb_clear();

	rc = nand_do_cmd (
            ni,
			  	CHAL_NAND_UC_OOB_WRITE,
				bank,
				page,
				(void*)buf
		  );

	if (rb_wait(bank, CHAL_NAND_TIMEOUT_WRITE)) {
      return ((rc !=  nand_rc ( CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
	}

   if (rc != nand_rc ( CHAL_NAND_RC_SUCCESS) ) {
      return rc;
   }

   do {
      status = nand_status_get (ni, bank);
   } while ( ! (status & NAND_STATUS_READY));

   return ((status & NAND_STATUS_FAIL ) ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS));
}


/**
 * page_write - write one page
 * @ni:     [in] nand info structure
 * @bank:	[in] selected bank
 * @page:	[in] selected page
 * @buf:	[in] source buffer
 */
static uint32_t page_write (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf)
{

  uint32_t rc;
  uint8_t status;

  ASSERT((((uint32_t)buf) % CHAL_NAND_DMA_ALIGN) == 0);

  rb_clear();
  rc = nand_do_cmd (
                         ni,
                         CHAL_NAND_UC_PAGE_WRITE,
                         bank,
                         page,
                         (void*)buf
                   );

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_WRITE)) {
    return ((rc !=  nand_rc ( CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
  }

  if (rc != nand_rc ( CHAL_NAND_RC_SUCCESS) ) {
    return rc;
  }

  do {
    status = nand_status_get (ni, bank);
  } while ( ! (status & NAND_STATUS_READY));

  return ((status & NAND_STATUS_FAIL ) ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS));
}


/**
 * page_oob_write - write one page including oob
 * @ni:     [in] nand info structure
 * @bank:	[in] selected bank
 * @page:	[in] selected page
 * @buf:	[in] source buffer
 */
static uint32_t page_oob_write (chal_nand_info_t *ni, uint8_t bank, uint32_t page, const uint8_t *buf)
{

  uint32_t rc;
  uint8_t status;

  ASSERT((((uint32_t)buf) % CHAL_NAND_DMA_ALIGN) == 0);

  rb_clear();
  rc = nand_do_cmd (
                         ni,
                         CHAL_NAND_UC_PAGE_OOB_WRITE,
                         bank,
                         page,
                         (void*)buf
                   );

  if(rb_wait(bank, CHAL_NAND_TIMEOUT_WRITE)) {
    return ((rc !=  nand_rc ( CHAL_NAND_RC_SUCCESS)) ? rc : nand_rc(CHAL_NAND_RC_RB_TOUT));
  }

  if (rc != nand_rc ( CHAL_NAND_RC_SUCCESS) ) {
    return rc;
  }

  do {
    status = nand_status_get (ni, bank);
  } while ( ! (status & NAND_STATUS_READY));

  return ((status & NAND_STATUS_FAIL ) ? nand_rc(CHAL_NAND_RC_FAILURE): nand_rc(CHAL_NAND_RC_SUCCESS));
}

#endif

/* auxiliary routines */

#ifndef __BROM_CODE__
/******************************************************************************
*
* nand_status_get(bank)
*
*
**/
static uint8_t nand_status_get (chal_nand_info_t *ni, uint8_t bank)
{
  return (NAND_OPS_CALL (ni,status_get,(ni,bank)));
}
#endif

/**
 * nand_pshare_set - sets up nand pinsharing
 */
static void nand_pshare_set (void)
{
/*
 * 	nand_boot_pin_share_settings();
 *   int32_t i;
 *  Kauai Pin shares
 *   for (i=27; i<=53; i++ ) {
 *     gpio_pshare_fsel ( i, GPIO_ALT1); // NAND
 *   }
 *   gpio_pshare_fsel ( 20, GPIO_ALT0); // NAND
 *   gpio_pshare_fsel ( 21, GPIO_ALT0); // NAND
 */

/*
 *  Maui pin shares
 *   gpio_pshare_fsel(6, GPIO_ALT1);	// NAND_WP
 *   gpio_pshare_fsel(7, GPIO_ALT1);	// NAND_CEN_0
 *   gpio_pshare_fsel(8, GPIO_ALT1);	// NAND_CEN_1
 *   for (i=22, i<=45; i++) {
 *     gpio_pshare_fsel(i, GPIO_ALT1);	// NAND_
 *   }
 *   gpio_pshare_fsel(74, GPIO_ALT2);	// NAND_RDY_3
 *   gpio_pshare_fsel(75, GPIO_ALT2); 	// NAND_RDY_2
 */

  return;
}

/**
 * uc_load_function - loads the uCode function into the uCode storage
 * @ni:	      [in] nand info structure
 * @cmd:	      [in] uc function
 * @ucode: 	   [in] pointer to the curret ucode offset location
 * @nb_instr:	[in] number of instrucions of uCode function
 */
static uint32_t uc_load_function (chal_nand_info_t *ni, chal_nand_uc_cmd_t cmd, const uint32_t *ucode, uint16_t nb_instr)
{
  uint32_t rc = 1;

  if (CHAL_NAND_UC_P(ni) + nb_instr <= ( uint32_t *) NAND_CTRL_MCODE_END ) {

    (CHAL_NAND_UC_LKUP(ni))[cmd] = CHAL_NAND_UC_P(ni) - (uint32_t *)NAND_CTRL_MCODE_START;

    do{
      *CHAL_NAND_UC_P(ni)++ = *ucode++;
    } while (--nb_instr);

    *CHAL_NAND_UC_P(ni)++ = uA_END;

    rc = 0; /* success */
  }
  return ( rc );
}

/**
 * nand_do_cmd - processes a nand cmd
 * @ni:	   [in] nand info structure
 * @cmd:	   [in] uc command
 * @bank:	[in] selected bank
 * @page:	[in] selected page
 * @buf:	   [in,out] buffer
 */
static uint32_t nand_do_cmd (chal_nand_info_t *ni, chal_nand_uc_cmd_t cmd, uint8_t bank, uint32_t page, void* buf )
{
  uint32_t rc = nand_rc ( CHAL_NAND_RC_FAILURE);
  uint32_t dma_desc;
  uint32_t ctrl = CHAL_REG_READ32 ( NAND_CTRL_CTRL);
  uint32_t size = 0;


  if ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC ) {
    ctrl &= ~NAND_CTRL_ECC_BYPASS;
  } else {
    ctrl |=  NAND_CTRL_ECC_BYPASS;
  }


  switch(cmd) {

  case CHAL_NAND_UC_ID_GET:
    /* Aux data (8 bytes) */
    size = 8;
    dma_desc = (NAND_CTRL_DMAIL_ENABLE | (bank << 10) | (size >> 3));
    ctrl &= ~NAND_CTRL_2NAND;
    /* need to bypass ECC on ID retrieval */
    ctrl |=  NAND_CTRL_ECC_BYPASS;
    break;

  case CHAL_NAND_UC_PARAM_READ:
    /* main data (512 bytes) */
    size = 0x200;
    /* number of 512-bytes shift by 5 into descriptor field */
    /* =>>  -9 +5 = -4								*/
    dma_desc = ((size >> 4) | NAND_CTRL_DMAIL_ENABLE | (bank << 10));
    ctrl &= ~NAND_CTRL_2NAND;
    /* need to bypass ECC on parameter page retrieval */
    ctrl |=  NAND_CTRL_ECC_BYPASS;
    break;

#ifndef __BROM_CODE__
  case CHAL_NAND_UC_STATUS_GET:
  case CHAL_NAND_UC_BLOCK_ERASE:
#endif
  case CHAL_NAND_UC_RESET:
  case CHAL_NAND_UC_PAGE_READ_PRE:
  case CHAL_NAND_UC_PARAM_READ_PRE:
    dma_desc = 0;
    break;

  case CHAL_NAND_UC_PAGE_READ:
    /* number of 512-bytes  sectors PAGE_SIZE/512 and shift by 5 into descriptor field */
    /* =>>  -9 +5 = -4								*/
    size = CHAL_NAND_PAGE_SIZE(ni);
    dma_desc = ((size >> 4) | NAND_CTRL_DMAIL_ENABLE | (bank << 10));
    /* set direction to FROM device */
    ctrl &= ~NAND_CTRL_2NAND;
    break;

  case CHAL_NAND_UC_OOB_READ:
    size = CHAL_NAND_AUX_DATA_SIZE(ni);
    dma_desc = (NAND_CTRL_DMAIL_ENABLE | (bank << 10) | (size >> 3));
    /* set direction to FROM device */
    ctrl &= ~NAND_CTRL_2NAND;
    break;

#ifndef __BROM_CODE__
  case CHAL_NAND_UC_PAGE_WRITE:
    /* number of 512-bytes  sectors PAGE_SIZE/512 and shift by 5 into descriptor field */
    /* =>>  -9 +5 = -4								*/
    size = CHAL_NAND_PAGE_SIZE(ni);
    dma_desc = ((size >> 4) | NAND_CTRL_DMAIL_ENABLE | (bank << 10));
    /* set direction to TO device */
    ctrl |= NAND_CTRL_2NAND;
    break;

  case CHAL_NAND_UC_OOB_WRITE:
    size = CHAL_NAND_AUX_DATA_SIZE(ni);
    dma_desc = (NAND_CTRL_DMAIL_ENABLE | (bank << 10) | (size >> 3));
    /* set direction to TO device */
    ctrl |= NAND_CTRL_2NAND;
    break;

  case CHAL_NAND_UC_PAGE_OOB_WRITE:
    /* number of 512-bytes  sectors PAGE_SIZE/512 and shift by 5 into descriptor field */
    /* =>>  -9 +5 = -4								*/
    size = CHAL_NAND_PAGE_SIZE(ni) + CHAL_NAND_AUX_DATA_SIZE(ni);
    dma_desc = ((CHAL_NAND_PAGE_SIZE(ni) >> 4) | NAND_CTRL_DMAIL_ENABLE | (bank << 10) | (CHAL_NAND_AUX_DATA_SIZE(ni) >> 3));
    /* set direction to TO device */
    ctrl |= NAND_CTRL_2NAND;
    break;
#endif

  default:
    rc = nand_rc ( CHAL_NAND_RC_NOCMD );
    goto nand_do_cmd_exit;
  }

  CHAL_REG_WRITE32 (NAND_CTRL_CTRL, ctrl | NAND_CTRL_ECC_RST);   /* reset ECCi blocks */
  CHAL_REG_WRITE32 (NAND_CTRL_CTRL, ctrl);			/* un-reset ECC and DMA blocks */
  CHAL_REG_WRITE32 (NAND_CTRL_ADDR, page);
  CHAL_REG_WRITE32 (NAND_CTRL_BANK, bank);
  /* clear error log for the block */
  CHAL_REG_WRITE32 ((uint32_t*)(NAND_CTRL_ERRLOG_BANK0)+bank,0x0);
  /* clear all IRQ in the status (W1C) */
  CHAL_REG_WRITE32 ( NAND_CTRL_IRQ_STATUS,~0x0);

  if ( dma_desc ) {
    /* 
     * call dma setup calback and put the physical address of the PRD table
     * in the PRD register
     */
    CHAL_REG_WRITE32 (NAND_CTRL_PRD,
                      (uint32_t) (CHAL_NAND_DMA_SETUP_CB(ni))(((ctrl & NAND_CTRL_2NAND) ? CHAL_NAND_DMA_TO_DEVICE : CHAL_NAND_DMA_FROM_DEVICE),
                                                               bank,
                                                               buf,
                                                               size));

    CHAL_REG_WRITE32 (NAND_CTRL_DMAIL_BANK10, dma_desc);
    CHAL_REG_WRITE32 (NAND_CTRL_DMAIL_BANK32, 0x0);
    CHAL_REG_WRITE32 (NAND_CTRL_DMAIL_BANK54, 0x0);
    CHAL_REG_WRITE32 (NAND_CTRL_DMAIL_BANK76, 0x0);

    dma_start();
  }

  udelay(TIMER_TICK_ADJUSTED_DELAY(1));
  /* jump to ucode */
  CHAL_REG_WRITE32(NAND_CTRL_CMD, (CHAL_NAND_UC_LKUP(ni))[cmd] | NAND_CTRL_CMD_VALID);

  if ( dma_desc ) {
    /* cmd parameter is hack as this on &%*^ of hardware	*/
    /* doesn't behave idetically in terms of status reporting	*/
    /* see DMA_WAIT code for details 				*/
    /* status = DMA_WAIT(bank,cmd);*/
    uint32_t res;
    uint32_t tmp;
    int32_t timer;
    int32_t d = TIMER_TICK_ADJUSTED_DELAY(1);

    if ((cmd != CHAL_NAND_UC_PAGE_READ) && (cmd != CHAL_NAND_UC_PARAM_READ)) {
    tmp = (((0x1 << bank) << NAND_IRQ_BANK_COMPLETE_SHIFT) | NAND_IRQ_DMA_COMPLETE);
    } else {
      tmp = NAND_IRQ_DMA_COMPLETE;
    }

    timer = CHAL_NAND_TIMEOUT_DMA;
    while (CHAL_REG_READ32(NAND_CTRL_STATUS) & NAND_CTRL_STATUS_DMA_ACTIVE) {
      if(timer <= 0) {
        break;
      }
      timer-=d;
      udelay(d);
    }

    if(timer <= 0) {
      rc = nand_rc( CHAL_NAND_RC_DMA_ACTIVE_TOUT );
    }
    else {
      timer = CHAL_NAND_TIMEOUT_UC_COMPLETE;
      /* check even futher */
      while (((res = CHAL_REG_READ32 ( NAND_CTRL_IRQ_STATUS )) & tmp)!= tmp) {
        if(timer <= 0) {
          break;
         }
         timer-=d;
         udelay(d);
      }
      if(timer <= 0) {
        if (res & NAND_IRQ_HM_ERROR) {
          rc = nand_rc ( CHAL_NAND_RC_ECC_HM_ERROR );
        }
        else {
          rc = !(res & NAND_IRQ_DMA_COMPLETE ) ? nand_rc ( CHAL_NAND_RC_DMA_CMPL_TOUT ):
                                                 nand_rc ( CHAL_NAND_RC_BANK_CMPL_TOUT );
        }
      }
      else if (res & NAND_IRQ_RS_ERROR) {
        rc = nand_rc ( CHAL_NAND_RC_ECC_RS_ERROR );
      }
      else if (res & NAND_IRQ_HM_ERROR) {
        rc = nand_rc ( CHAL_NAND_RC_ECC_HM_ERROR );
      }
      else {
        rc = nand_rc ( CHAL_NAND_RC_SUCCESS );
      }
    }

    dma_stop();

  } else {
    rc = mm_cmd_complete(NAND_CTRL_CMD);
  }

#ifndef __BROM_CODE__
  if ( cmd == CHAL_NAND_UC_STATUS_GET ) {
    *((uint8_t*)buf) = (uint8_t)CHAL_REG_READ16(NAND_CTRL_MRESP);
  }
#endif

nand_do_cmd_exit:

  return ( rc );
}

/**
 * nand_cfg_geometry - configures the geometry based on chip id info
 */
static void nand_cfg_geometry (chal_nand_info_t *ni)
{
  uint8_t sizes;
  uint8_t *id = CHAL_NAND_ID(ni);

  /* figure out device specific sizes -- page, plane , block , bank */
  sizes = id[4];

  /* 64 M_bit min (1 min plane)  => 2^26 / 8 (bit/byte) = 2^23 */
  CHAL_NAND_BANK_SHIFT(ni) = 23 + ((sizes & CHAL_NAND_ID5_PLANE_SIZE) >> CHAL_NAND_ID5_PLANE_SIZE_SHIFT) +
                         ((sizes & CHAL_NAND_ID5_NUM_PLANE) >> CHAL_NAND_ID5_NUM_PLANE_SHIFT);

  sizes = id[3];

  /* 1K min : 10 */
  CHAL_NAND_PAGE_SHIFT(ni) = 10 + ((sizes & CHAL_NAND_ID4_PAGE_SIZE) >> CHAL_NAND_ID4_PAGE_SIZE_SHIFT);

  /* bl_size = 16 + (sizes >> 4) & 0x3 [min 64K]*/
  CHAL_NAND_BLOCK_SHIFT(ni)	= 16 + ((sizes & CHAL_NAND_ID4_BLOCK_SIZE) >> CHAL_NAND_ID4_BLOCK_SIZE_SHIFT);

  /* 			  sectors in a page(512 bytes)	   spare size 8 or 16 bytes per sector */
  CHAL_NAND_OOB_SIZE(ni)	= (CHAL_NAND_PAGE_SIZE(ni) >> 9) * (8 << ((sizes & CHAL_NAND_ID4_SPARE_SIZE) >> CHAL_NAND_ID4_SPARE_SIZE_SHIFT));

  if ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC )
    CHAL_NAND_AUX_DATA_SIZE(ni) = 0x28 << (CHAL_NAND_PAGE_SHIFT(ni) - 11);   /* 40 bytes for every 2KB of main data */
  else
    CHAL_NAND_AUX_DATA_SIZE(ni) = 0x40 << (CHAL_NAND_PAGE_SHIFT(ni) - 11);   /* 64 bytes for every 2KB of main data */

  CHAL_NAND_BUS_WIDTH(ni) = (8 << ((sizes & CHAL_NAND_ID4_BUS_WIDTH) >> CHAL_NAND_ID4_BUS_WIDTH_SHIFT));
}

/**
 * nand_cfg_onfi - configures the geometry based on the ONFI parameter page
 */
static void nand_cfg_onfi (chal_nand_info_t *ni, uint8_t *param_page)
{
  uint32_t page_size;
  uint32_t spare_size;
  uint32_t pages_per_block;
  uint32_t blocks_per_lun;
  uint32_t lun_per_chip_enable;

  page_size = param_page[80] + (param_page[81] << 8) + (param_page[82] << 16) + (param_page[83] << 24);   
  spare_size = param_page[84] + (param_page[85] << 8);
  pages_per_block = param_page[92] + (param_page[93] << 8) + (param_page[94] << 16) + (param_page[95] << 24);   
  blocks_per_lun = param_page[96] + (param_page[97] << 8) + (param_page[98] << 16) + (param_page[99] << 24);   
  lun_per_chip_enable = param_page[100];

  CHAL_NAND_BUS_WIDTH(ni) = 8;

  CHAL_NAND_PAGE_SHIFT(ni) =  shift_of(page_size);

  CHAL_NAND_BLOCK_SHIFT(ni) = shift_of(pages_per_block) + CHAL_NAND_PAGE_SHIFT(ni);

  CHAL_NAND_BANK_SHIFT(ni) = shift_of(lun_per_chip_enable) + shift_of(blocks_per_lun) + CHAL_NAND_BLOCK_SHIFT(ni); 

  CHAL_NAND_OOB_SIZE(ni) = spare_size;

  if ( CHAL_NAND_FLAGS(ni) & CHAL_NAND_FLAG_ECC )
    CHAL_NAND_AUX_DATA_SIZE(ni) = 0x28 << (CHAL_NAND_PAGE_SHIFT(ni) - 11);   /* 40 bytes for every 2KB of main data */
  else
    CHAL_NAND_AUX_DATA_SIZE(ni) = 0x40 << (CHAL_NAND_PAGE_SHIFT(ni) - 11);   /* 64 bytes for every 2KB of main data */
}

static uint32_t shift_of(uint32_t i)
{
   uint32_t s = 0;

   i >>= 1;
   while (i)
   {
      s++;
      i >>= 1;
   }
   return s;
}


/* 
 * CRC16 verification
 *
 * Argunents:
 *    p     pointer to byte stream
 *    len   byte stream length
 * Return:
 *    1     crc correct
 *    0     crc incorrect
 */
static uint32_t crc16_valid(uint8_t *p, uint32_t len)
{
   /* Bit by bit algorithm without augmented zero bytes */
   const uint32_t crcinit = 0x4F4E; /* Initial CRC value in the shift register */
   const int32_t order = 16; /* Order of the CRC-16 */
   const uint32_t polynom = 0x8005; /* Polynomial */

   uint32_t i, j, c, bit;
   uint32_t crc;
   uint32_t crcmask, crchighbit;

   crc = crcinit; /* Initialize the shift register with 0x4F4E */
   crcmask = ((((uint32_t)1<<(order-1))-1)<<1)|1;
   crchighbit = (uint32_t)1<<(order-1);

   /* process byte stream, one byte at a time, bits processed from MSB to LSB */
   for(i=0; i<len; i++)
   {
      c = (uint32_t)(*p);
      p++;
      for (j=0x80; j; j>>=1)
      {
         bit = crc & crchighbit;
         crc<<= 1;
         if (c & j) bit^= crchighbit;
         if (bit) crc^= polynom;
      }
      crc&= crcmask;
   }

   c = ((uint32_t)(*(p+1)) << 8 ) | (uint32_t)(*p);

   return (c == crc);
}

