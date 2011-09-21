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
 *    FILE:          chal_nand_uc.h
 *
 *    DESCRIPTION:   NAND MICROCODE (included in chal_nand.c)
 *
 *    WRITTEN BY:    agf@broadcom.com, Aug 10 2007.
 *                   lingwan@broadcom.com, Apr. 24 2008
 *                   cdoban@broadcom.com, Jan 26 2010
**/

#ifndef __CHAL_NAND_UC_H__
#define __CHAL_NAND_UC_H__

#include "chal_nand_cmd.h"

/* microcode instructions */
#define instr_WC	(0x00)	/* put command 		*/
#define instr_WA	(0x04)	/* put address 		*/
#define instr_WD	(0x08)	/* put data 		*/
#define instr_RD	(0x0C)	/* sample/read data 	*/
#define instr_RS	(0x10)	/* read stsus , works?? */
#define instr_WT	(0x20)	/* wait event/timeout	*/

#define instr_ADD	(0x40)
#define instr_SUB  (0x60)
#define instr_AND  (0x80)
#define instr_OR   (0xA0)
#define instr_SH   (0xC0)
#define instr_MV   (0xD0)
#define instr_BR   (0xE0)

#define instr_END   (0x1C)


/* instruction with 10 bits for operands */
#define mk_instr_10(cmd,op)	(0x10000 | ((cmd & 0xFC) << 8)|((op) & 0x3FF))
/* instruction with 13 bits for operands */
#define mk_instr_13(cmd,op)	(0x10000 | ((cmd & 0xE0) << 8)|((op) & 0x1FFF))

/* uASSEMBLY */
#define uR0	(0x0)
#define uR1	(0x1)
#define uR2	(0x2)
#define uR3	(0x3)
#define uR4	(0x4)
#define uR5	(0x5)
#define uR6	(0x6)
#define uR7	(0x7)
#define uR8	(0x8)
#define uR9	(0x9)
#define uRA	(0xA)
#define uRB	(0xB)
#define uRC	(0xC)
#define uRD	(0xD)
#define uRE	(0xE)
#define uRF	(0xF)

#define uEVT_RB                  0
#define uEVT_DMA_START           1
#define uEVT_WR_DONE             2
#define uEVT_RD_DONE             3
#define uEVT_DMA_DONE            4
#define uEVT_TOUT                7


#define uA_END			            mk_instr_10(instr_END,0x3FF)
#define uA_WC(opcode)		      mk_instr_10(instr_WC,opcode)
#define uA_WA(cycles)		      mk_instr_10(instr_WA,cycles)
#define uA_WD(bytes)		         mk_instr_10(instr_WD,bytes)
#define uA_RD(bytes)		         mk_instr_10(instr_RD,bytes)
#define uA_RS			            mk_instr_10(instr_RS,0x1)

#define uA_ADDI(reg,val)         mk_instr_13(instr_ADD, ((reg & 0xF) << 9) | (val & 0xFF))
#define uA_SUBI(reg,val)         mk_instr_13(instr_SUB, ((reg & 0xF) << 9) | (val & 0xFF))
#define uA_ANDI(reg,val)         mk_instr_13(instr_AND, ((reg & 0xF) << 9) | (val & 0xFF))
#define uA_ORI(reg,val)          mk_instr_13(instr_OR, ((reg & 0xF) << 9) | (val & 0xFF))

#define uA_ADD(reg1,reg2)        mk_instr_13(instr_ADD, ((reg1 & 0xF) << 9) | 0x100 | ((reg2 & 0xF) << 4))
#define uA_SUB(reg1,reg2)        mk_instr_13(instr_SUB, ((reg1 & 0xF) << 9) | 0x100 | ((reg2 & 0xF) << 4))
#define uA_AND(reg1,reg2)        mk_instr_13(instr_AND, ((reg1 & 0xF) << 9) | 0x100 | ((reg2 & 0xF) << 4))
#define uA_OR(reg1,reg2)         mk_instr_13(instr_OR, ((reg1 & 0xF) << 9) | 0x100 | ((reg2 & 0xF) << 4))

#define uA_LSH(reg,offset)       mk_instr_10(instr_SH, ((reg & 0xF) << 5) | (offset & 0xF))
#define uA_RSH(reg,offset)       mk_instr_10(instr_SH, ((reg & 0xF) << 5) | 0x10 | (offset & 0xF))
#define uA_MV(reg1,reg2)         mk_instr_10(instr_MV, ((reg1 & 0xF) << 5) | (reg2 & 0xF))

#define uA_BRA(offset)           mk_instr_13(instr_BR, ((offset<0) ? (0x100 | (-(offset) & 0xFF)) : (offset & 0xFF))) 
#define uA_BRZ(offset)           mk_instr_13(instr_BR, ((0x1 << 10) | ((offset<0) ? (0x100 | (-(offset) & 0xFF)) : (offset & 0xFF)))) 
#define uA_BRC(offset)           mk_instr_13(instr_BR, ((0x2 << 10) | ((offset<0) ? (0x100 | (-(offset) & 0xFF)) : (offset & 0xFF))))

#define uA_WTS(evt,ticks)        mk_instr_13(instr_WT, ((evt & 0x7) << 10) | (ticks & 0x1FF))
#define uA_WTL(evt,ticks)        mk_instr_13(instr_WT, ((evt & 0x7) << 10) | 0x200 | (ticks & 0x1FF))


static const uint32_t uc_RESET[]  = {
  uA_WC(NAND_CMD_RESET)		/* WC 0xFF	  |               */
};
  

static const uint32_t uc_ID_GET[]  = {
  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00 |						*/
  uA_MV(uR2,uRB), 	/* MV  #R2, #RB   | 1 byte of address ( 0x0 or 0x20 )  	*/
  uA_WC(NAND_CMD_ID),
  uA_WA(0x1) ,
  uA_RD(0x8),
  uA_BRA(-6)  		  	/* BRA -6 to WT_DMA						*/
};

/* parameter page read preamble */
static const uint32_t uc_PARAM_READ_PRE[]  = {
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | zero the adderess */
  uA_WC(NAND_CMD_READ_PARAM) , 	/* WC 0xEC        | 	 				*/
  uA_WA(0x1), 			/* WA 0x01	  | one address cycle	*/
};

static const uint32_t uc_PARAM_READ[]  = {
  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00 */

  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_BRA(-4)  		   /*BRA -4 to WT_DMA */
};

/* page read preamble */
static const uint32_t uc_PAGE_READ_PRE[]  = {
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_MV(uR2,uRC), 	/* MV  #R2, #RC   | first 2 bytes of address ( lower )  	*/
  uA_MV(uR3,uRD), 	/* MV  #R3, #RD   | 3rd byte of address			*/
  uA_WC(NAND_CMD_READ_1ST) , 	/* WC 0x00        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard read sequence		*/
  uA_WC(NAND_CMD_READ_2ND) 	/* WC 0x30	  |					*/
};


static const uint32_t uc_PAGE_READ[]  = {
  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00 */

  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_BRA(-10)  		/*BRA -10 to WT_DMA */
};


static const uint32_t uc_PAGE_READ_ECC[]  = {
  uA_ANDI(uR2,0x0),	/* AND #R2 ,0x0	|									*/
  uA_ADDI(uR2,0x80),	/* ADD #R2, 0x80 	| r2=0x80							*/
  uA_LSH(uR2,0x4),	/* LSH #R2, 0x4	| r2 = 0x800						*/
  uA_ADDI(uR2,0x26),	/* ADD #R2, 0x26.	| r2 = 0x826, ecc offset			*/

  uA_WTS(uEVT_DMA_START,0),	 /* WT_DMA0 S,0x00	*/

  uA_RD (0x100), 	   /* read the 2k page */
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),

  uA_MV(uR2,uRB),		/* MV #R2,#RB		| ECC offset ready					*/
  uA_WC(NAND_CMD_READ_RAND_1ST),		/* start random output 				*/
  uA_WA(0x2),		   /* 2 address cycles 					*/
  uA_WC(NAND_CMD_READ_RAND_2ND),
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/
  uA_RD (0x18),	   /* read ECC data (24 bytes)				*/

  uA_BRA(-16)		   /* BRA -16 to WT_DMA */
};


static const uint32_t uc_PAGE_READ_4K[]  = {
  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00 */

  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_BRA(-18)     /*BRA -18 to WT_DMA */
};


static const uint32_t uc_PAGE_READ_ECC_4K[]  = {
  uA_ANDI(uR2,0x0),  /* AND #R2 ,0x0   |                 */
  uA_ADDI(uR2,0x80), /* ADD #R2, 0x80  | r2=0x80               */
  uA_LSH(uR2,0x4),   /* LSH #R2, 0x4   | r2 = 0x800               */
  uA_MV(uR2,uR3),    /* MV  #R2, #R3   | r3 = 0x800, main data offset         */ 
  uA_LSH(uR2,0x1),   /* LSH #R2, 0x1   | r2 = 0x1000              */
  uA_ADDI(uR2,0x4C), /* ADD #R2, 0x4C. | r2 = 0x104C, ecc offset        */

  uA_WTS(uEVT_DMA_START,0),      /* WT_DMA0 S,0x00   */

  uA_RD (0x100),     /* read page 0x0 */
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),

  uA_MV(uR2,uRB),    /* MV #R2,#RB     | ECC offset ready      */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output              */
  uA_WA(0x2),        /* 2 address cycles              */
  uA_WC(NAND_CMD_READ_RAND_2ND),
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x18),      /* read ECC data (24 bytes)            */

  uA_MV(uR3,uRB),    /* MV  #R3, #RB               */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output              */
  uA_WA(0x2),     /* 2 address cycles              */
  uA_WC(NAND_CMD_READ_RAND_2ND),
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x100),  /* read page 0x1               */
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 

  uA_ADDI(uR2,0x18), /* r2 += 24, move ecc pointer             */
  uA_MV(uR2,uRB),    /* MV #R2,#RB     | ECC offset ready */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output */
  uA_WA(0x2),        /* 2 address cycles */
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x18),      /* read ECC data*/

  uA_BRA(-36)        /* BRA -36 to WT_DMA */
};


static const uint32_t uc_PAGE_READ_8K[]  = {
  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00 */

  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100),
  uA_BRA(-34)     /*BRA -34 to WT_DMA */
};


static const uint32_t uc_PAGE_READ_ECC_8K[]  = {
  uA_ANDI(uR2,0x0),  /* AND #R2 ,0x0   |                 */
  uA_ADDI(uR2,0x80), /* ADD #R2, 0x80  | r2 = 0x80                */
  uA_LSH(uR2,0x4),   /* LSH #R2, 0x4   | r2 = 0x800               */
  uA_MV(uR2,uR3),    /* MV  #R2, #R3   | r3 = 0x800, main data offset         */
  uA_MV(uR2,uRE),	   /* MV  #R2, #RE   | rE = 0x800, main data increment size	*/  
  uA_LSH(uR2,0x2),   /* LSH #R2, 0x2   | r2 = 0x2000              */
  uA_ADDI(uR2,0x98), /* ADD #R2, 0x98. | r2 = 0x2098, ecc offset        */

  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00   */

  uA_RD (0x100),  /* read page 0x0 */
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),
  uA_RD (0x100),

  uA_MV(uR2,uRB),    /* MV #R2,#RB     | ECC offset ready      */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output              */
  uA_WA(0x2),        /* 2 address cycles              */
  uA_WC(NAND_CMD_READ_RAND_2ND),
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x18),      /* read ECC data (24 bytes)            */

  uA_MV(uR3,uRB),    /* MV  #R3, #RB               */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output              */
  uA_WA(0x2),        /* 2 address cycles              */
  uA_WC(NAND_CMD_READ_RAND_2ND),
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x100),     /* read page 0x1               */
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 

  uA_ADDI(uR2,0x18), /* r2 += 24, move ecc pointer             */
  uA_MV(uR2,uRB),    /* MV #R2,#RB     | ECC offset ready */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output */
  uA_WA(0x2),        /* 2 address cycles */
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x18),      /* read ECC data*/

  uA_ADD(uR3,uRE),	/* ADD #R3, #RE */  
  uA_MV(uR3,uRB),    /* MV  #R3, #RB               */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output              */
  uA_WA(0x2),        /* 2 address cycles              */
  uA_WC(NAND_CMD_READ_RAND_2ND),
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x100),     /* read page 0x2               */
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 

  uA_ADDI(uR2,0x18), /* r2 += 24, move ecc pointer             */
  uA_MV(uR2,uRB),    /* MV #R2,#RB     | ECC offset ready */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output */
  uA_WA(0x2),        /* 2 address cycles */
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x18),      /* read ECC data*/
 
  uA_ADD(uR3,uRE),   /* ADD #R3, #RE */  
  uA_MV(uR3,uRB),    /* MV  #R3, #RB               */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output              */
  uA_WA(0x2),     /* 2 address cycles              */
  uA_WC(NAND_CMD_READ_RAND_2ND),
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x100),  /* read page 0x3               */
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 
  uA_RD (0x100), 

  uA_ADDI(uR2,0x18), /* r2 += 24, move ecc pointer             */
  uA_MV(uR2,uRB),    /* MV #R2,#RB     | ECC offset ready */
  uA_WC(NAND_CMD_READ_RAND_1ST),    /* start random output */
  uA_WA(0x2),     /* 2 address cycles */
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x18),      /* read ECC data*/


  uA_BRA(-78)           /* BRA -78 to WT_DMA */
};


static const uint32_t uc_OOB_READ[]  = {
  uA_ANDI(uRB,0x0),	/* AND #RB ,0x0	| rB = 0x0						*/
  uA_ADDI(uRB,0x80), /* ADD #RB, 0x80  | rB = 0x80               	*/
  uA_LSH(uRB,0x4),   /* LSH #RB, 0x4   | rB = 0x800		*/
  
  uA_WTS(uEVT_DMA_START,0),	/* WT_DMA0 S,0x00	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output */
  uA_WA(0x2),			/* 2 address cycles */
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x40),		   /* read 0x40 bytes	*/

  uA_BRA(-7)		   /* BRA -7 to WT_DMA	*/
};


static const uint32_t uc_OOB_READ_4K[]  = {
  uA_ANDI(uRB,0x0),	/* AND #RB ,0x0	| rB = 0x0						*/
  uA_ADDI(uRB,0x80), /* ADD #RB, 0x80  | rB = 0x80               	*/
  uA_LSH(uRB,0x5),   /* LSH #RB, 0x5   | rB = 0x1000		*/
  
  uA_WTS(uEVT_DMA_START,0),	/* WT_DMA0 S,0x00	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output */
  uA_WA(0x2),				/* 2 address cycles */
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x80),		   /* read 0x80 bytes	*/

  uA_BRA(-7)		   /* BRA -7 to WT_DMA	*/
};


static const uint32_t uc_OOB_READ_8K[]  = {
  uA_ANDI(uRB,0x0),	/* AND #RB ,0x0	| rB = 0x0						*/
  uA_ADDI(uRB,0x80), /* ADD #RB, 0x80  | rB = 0x80               	*/
  uA_LSH(uRB,0x6),   /* LSH #RB, 0x6   | rB = 0x2000		*/
  
  uA_WTS(uEVT_DMA_START,0),	/* WT_DMA0 S,0x00	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output */
  uA_WA(0x2),				/* 2 address cycles */
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x100),	   /* read 0x100 bytes	*/

  uA_BRA(-7)		   /* BRA -7 to WT_DMA	*/
};


static const uint32_t uc_OOB_READ_ECC[]  = {
  uA_ANDI(uRB,0x0),	/* AND #RB ,0x0	| rB = 0x0						*/
  uA_ADDI(uRB,0x80), /* ADD #RB, 0x80  | rB = 0x80               	*/
  uA_LSH(uRB,0x4),   /* LSH #RB, 0x4   | rB = 0x800		*/
  
  uA_WTS(uEVT_DMA_START,0),      /* WT_DMA0 S,0x00	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x26),			/* read OOB data (38 bytes)	*/

  uA_ADDI(uRB,0x3E),	/* ADD #RB, 0x3E.	| rB = 0x83E, HM ECC offset ready	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),				/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x2),			/* read HM ECC data (2 bytes)	*/

  uA_BRA(-13)			/* BRA -13 to WT_DMA		*/
};


static const uint32_t uc_OOB_READ_ECC_4K[]  = {
  uA_ANDI(uRB,0x0),	/* AND #RB ,0x0	| rB = 0x0			*/
  uA_ADDI(uRB,0x80), /* ADD #RB, 0x80  | rB = 0x80      	*/
  uA_LSH(uRB,0x5),   /* LSH #RB, 0x5   | rB = 0x1000		*/
  
  uA_WTS(uEVT_DMA_START,0),	/* WT_DMA0 S,0x00	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x26),			/* read OOB (1st) data (38 bytes)	*/

  uA_ADDI(uRB,0x7C),	/* ADD #RB, 0x7C.	| rB = 0x107C, HM ECC (1st) offset ready*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),				/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x2),			/* read HM ECC (1st) data (2 bytes)	*/

  uA_SUBI(uRB,0x56),	/* SUB #RB, 0x56.	| rB = 0x1026, OOB (2nd) offset ready	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x26),			/* read OOB (2nd) data (38 bytes)	*/

  uA_ADDI(uRB,0x58),	/* ADD #RB, 0x58.	| rB = 0x107E, HM ECC (2nd) offset ready*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x2),			/* read HM ECC (2nd) data (2 bytes)	*/

  uA_BRA(-25)		   /* BRA -25 to WT_DMA	*/
};


static const uint32_t uc_OOB_READ_ECC_8K[]  = {
  uA_ANDI(uRB,0x0),	/* AND #RB ,0x0	| rB = 0x0			*/
  uA_ADDI(uRB,0x80), /* ADD #RB, 0x80  | rB = 0x80      	*/
  uA_LSH(uRB,0x6),   /* LSH #RB, 0x6   | rB = 0x2000		*/

  uA_WTS(uEVT_DMA_START,0),	/* WT_DMA0 S,0x00	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x26),			/* read OOB (1st) data (38 bytes)	*/

  uA_ADDI(uRB,0xF8),	/* ADD #RB, 0xF8.	| rB = 0x20F8, HM ECC (1st) offset ready*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),				/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x2),			/* read HM ECC (1st) data (2 bytes)	*/

  uA_SUBI(uRB,0xD2),	/* SUB #RB, 0xD2.	| rB = 0x2026, OOB (2nd) offset ready	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x26),			/* read OOB (2nd) data (38 bytes)	*/

  uA_ADDI(uRB,0xD4),	/* ADD #RB, 0xD4.	| rB = 0x20FA, HM ECC (2nd) offset ready*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x2),			/* read HM ECC (2nd) data (2 bytes)	*/

  uA_SUBI(uRB,0xAE),	/* SUB #RB, 0xAE.	| rB = 0x204C, OOB (3nd) offset ready	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x26),			/* read OOB (2nd) data (38 bytes)	*/

  uA_ADDI(uRB,0xB0),	/* ADD #RB, 0xB0.	| rB = 0x20FC, HM ECC (3nd) offset ready*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x2),			/* read HM ECC (2nd) data (2 bytes)	*/

  uA_SUBI(uRB,0x8A),	/* SUB #RB, 0x8A.	| rB = 0x2072, OOB (4nd) offset ready	*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD(0x26),			/* read OOB (2nd) data (38 bytes)	*/

  uA_ADDI(uRB,0x8C),	/* ADD #RB, 0x8C.	| rB = 0x20FE, HM ECC (4nd) offset ready*/

  uA_WC(NAND_CMD_READ_RAND_1ST),	/* start random output	*/
  uA_WA(0x2),			/* 2 address cycles	*/
  uA_WC(NAND_CMD_READ_RAND_2ND),

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_RD (0x2),			/* read HM ECC (2nd) data (2 bytes)	*/

  uA_BRA(-49)		   /* BRA -49 to WT_DMA	*/
};


#ifndef __BROM_CODE__

static const uint32_t uc_STATUS_GET[]  = {
  uA_WC(NAND_CMD_STATUS),
  uA_WTS(uEVT_TOUT,1), 	/* WT_TOUT S, 0x01   */
  uA_RS, 	            /* RS                */
  uA_MV(uRA,uRF)  	   /* MV  #RA, #RF      */
};


static const uint32_t uc_BLOCK_ERASE[]  = {
  uA_MV(uR2,uRB), 		/* MV  #R2, #RB    | first 2 bytes of address ( lower )  	*/
  uA_MV(uR3,uRC), 		/* MV  #R3, #RC    | 3rd byte of address			*/
  uA_WC (NAND_CMD_BERASE_1ST),	/* WC 0x60	   | block erase 				*/
  uA_WA (0x3), 			/* WA 0x3	   | 3 address cycles				*/
  uA_WC (NAND_CMD_BERASE_2ND) 	/* WC 0xD0	   | confirm the erase operation		*/ 
};


static const uint32_t uc_PAGE_WRITE_ECC[]  = {
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_MV(uR2,uRC), 	/* MV  #R2, #RC   | first 2 bytes of address ( lower )  	*/
  uA_MV(uR3,uRD), 	/* MV  #R3, #RD   | 3rd byte of address			*/
  uA_WC(NAND_CMD_PROG_1ST) , 	/* WC 0x80			| 	 				*/
  uA_WA(0x5), 			/* WA 0x05		| standard write sequence		*/
  
  uA_ANDI(uR2,0x0),	/* AND #R2 ,0x0	|						*/
  uA_ADDI(uR2,0x80),	/* ADD #R2, 0x80 	| r2=0x80					*/
  uA_LSH(uR2,0x4),	/* LSH #R2, 0x4	| r2 = 0x800					*/
  uA_ADDI(uR2,0x26),	/* ADD #R2, 0x26.	| r2 = 0x826, ecc offset			*/
  uA_WTS(uEVT_TOUT,1),	      /* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/

  uA_WD (0x100), 		/* write sector 0x0 							*/
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),

  uA_MV(uR2,uRB),			/* MV #R2,#RB		| ECC offset ready				*/
  
  uA_WC(NAND_CMD_PROG_RAND),	/* start random input		 					*/
  
  uA_WA(0x2),			   /* 2 address cycles 							*/
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),			/* write  ECC data (24 bytes)						*/

  uA_WC(NAND_CMD_PROG_2ND), 	/* WC 0x10	  |					*/
  uA_BRA(-16) 		    	/* BRA -16		| to WT_DMA					*/
};

static const uint32_t uc_OOB_WRITE_ECC[]  = {
  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/

  uA_ANDI(uRB,0x0),	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_ADDI(uRB,0x80), /* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x4),	/* LSH #RB, 0x4   | rB = 0x800				*/
  uA_MV(uR2,uRC),		/* MV  #R2, #RC   | first 2 bytes of address ( lower )  */
  uA_MV(uR3,uRD),		/* MV  #R3, #RD   | 3rd byte of address			*/

  uA_WC(NAND_CMD_PROG_1ST), 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard write sequence		*/

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB data (38 bytes)					*/

  uA_ADDI(uRB,0x3E),	/* ADD #RB, 0x3E.	| rB = 0x83E, HM ECC offset ready	*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/
  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/
  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-17)		   /* BRA -17 to WT_DMA	*/
};

static const uint32_t uc_PAGE_WRITE_ECC_4K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0    | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC    | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD    | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80  */
  uA_WA(0x5),        /* WA 0x05         | standard write sequence      */
  
  uA_ANDI(uR2,0x0),  /* AND #R2 ,0x0    |                 */
  uA_ADDI(uR2,0x80), /* ADD #R2, 0x80   | r2=0x80               */
  uA_LSH(uR2,0x4),   /* LSH #R2, 0x4    | r2 = 0x800               */
  uA_MV(uR2,uR3),    /* MV  #R2, #R3    | r3 = 0x800, main data offset         */ 
  uA_LSH(uR2,0x1),   /* LSH #R2, 0x1    | r2 = 0x1000              */
  uA_ADDI(uR2,0x4C), /* ADD #R2, 0x4C.  | r2 = 0x104C, ecc offset         */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00                     */

  uA_WD (0x100),     /* write sector 0x0                    */
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),

  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  
  uA_WC(NAND_CMD_PROG_RAND),  /* start random input                     */
  
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data (24 bytes)                */

  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x1                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),         /* write  ECC data*/

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-34)        /* BRA -34    | to WT_DMA             */
};

static const uint32_t uc_OOB_WRITE_ECC_4K[]  = {
  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/

  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x5),	/* LSH #RB, 0x5   | rB = 0x1000				*/
  uA_MV(uR2,uRC),		/* MV  #R2, #RC   | first 2 bytes of address ( lower )  */
  uA_MV(uR3,uRD),		/* MV  #R3, #RD   | 3rd byte of address			*/


  uA_WC(NAND_CMD_PROG_1ST), 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard write sequence		*/

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (1st) data (38 bytes)				*/

  uA_ADDI(uRB,0x7C),	/* ADD #RB, 0x7C.	| rB = 0x107C, HM ECC (1st) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/

  uA_SUBI(uRB,0x56),	/* SUB #RB, 0x56.	| rB = 0x1026, OOB (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0x58),	/* ADD #RB, 0x58.	| rB = 0x107E, HM ECC (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-27)		   /* BRA -27 to WT_DMA	*/
};

static const uint32_t uc_PAGE_WRITE_ECC_8K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0    | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC    | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD    | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80  */
  uA_WA(0x5),        /* WA 0x05         | standard write sequence      */
  
  uA_ANDI(uR2,0x0),  /* AND #R2 ,0x0    |                 */
  uA_ADDI(uR2,0x80), /* ADD #R2, 0x80   | r2=0x80               */
  uA_LSH(uR2,0x4),   /* LSH #R2, 0x4    | r2 = 0x800               */
  uA_MV(uR2,uR3),    /* MV  #R2, #R3    | r3 = 0x800, main data offset         */ 
  uA_MV(uR2,uRE),    /* MV  #R2, #RE	 | rE = 0x800, main data increment size */
  uA_LSH(uR2,0x2),   /* LSH #R2, 0x2    | r2 = 0x2000              */
  uA_ADDI(uR2,0x98), /* ADD #R2, 0x98.  | r2 = 0x2098, ecc offset         */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00                     */

  uA_WD (0x100),     /* write sector 0x0                    */
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),

  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  
  uA_WC(NAND_CMD_PROG_RAND),  /* start random input                     */
  
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data (24 bytes)                */

  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x1                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),         /* write  ECC data*/
  
  uA_ADD(uR3,uRE),	/* ADD #R3, #RE	| move main data pointer */
  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x2                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data*/

  uA_ADD(uR3,uRE),	/* ADD #R3, #RE	| move main data pointer */
  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x3                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data*/

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-72)        /* BRA -72    | to WT_DMA             */
};

static const uint32_t uc_OOB_WRITE_ECC_8K[]  = {
  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/

  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x6),	/* LSH #RB, 0x6   | rB = 0x2000				*/
  uA_MV(uR2,uRC),		/* MV  #R2, #RC   | first 2 bytes of address ( lower )  */
  uA_MV(uR3,uRD),		/* MV  #R3, #RD   | 3rd byte of address			*/


  uA_WC(NAND_CMD_PROG_1ST), 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard write sequence		*/

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (1st) data (38 bytes)				*/

  uA_ADDI(uRB,0xF8),	/* ADD #RB, 0xF8.	| rB = 0x20F8, HM ECC (1st) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/

  uA_SUBI(uRB,0xD2),	/* SUB #RB, 0xD2.	| rB = 0x2026, OOB (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0xD4),	/* ADD #RB, 0xD4.	| rB = 0x20FA, HM ECC (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/


  uA_SUBI(uRB,0xAE),	/* SUB #RB, 0xAE.	| rB = 0x204C, OOB (3rd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0xB0),	/* ADD #RB, 0xB0.	| rB = 0x20FC, HM ECC (3rd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/


  uA_SUBI(uRB,0x8A),	/* SUB #RB, 0x8A.	| rB = 0x2072, OOB (4th) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0x8C),	/* ADD #RB, 0x8C.	| rB = 0x20FE, HM ECC (3th) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/


  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-47)	      /* BRA -47 to WT_DMA	*/

};


static const uint32_t uc_PAGE_WRITE[]  = {
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_MV(uR2,uRC), 	/* MV  #R2, #RC   | first 2 bytes of address ( lower )  	*/
  uA_MV(uR3,uRD), 	/* MV  #R3, #RD   | 3rd byte of address			*/
  uA_WC(NAND_CMD_PROG_1ST) , 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard read sequence		*/
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),	/*  WT_DMA0 S,0x00					*/


  uA_WD (0x100), 		/* write sector 0x0 */
  uA_WD (0x100),

  uA_WD (0x100), 		/* write sector 0x1 					*/
  uA_WD (0x100), 	

  uA_WD (0x100), 		/* write sector 0x2					*/
  uA_WD (0x100),

  uA_WD (0x100), 		/* write sector 0x3 					*/
  uA_WD (0x100), 

  uA_WC(NAND_CMD_PROG_2ND), 	/* WC 0x10	  |					*/

  uA_BRA(-11) 			/*  BRA -11 to WT_DMA					*/
};


static const uint32_t uc_OOB_WRITE[]  = {
  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/

  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x4),	/* LSH #RB, 0x4   | rB = 0x800				*/
  uA_MV(uR2,uRC),		/* MV  #R2, #RC   | first 2 bytes of address ( lower )  */
  uA_MV(uR3,uRD),		/* MV  #R3, #RD   | 3rd byte of address			*/



  uA_WC(NAND_CMD_PROG_1ST), 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard write sequence		*/

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/


  uA_WD (0x40),		/* write 64 bytes							*/ 

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-12)		/* BRA -12 to WT_DMA	*/
};

static const uint32_t uc_PAGE_WRITE_4K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0   | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC   | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD   | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80        |              */
  uA_WA(0x5),        /* WA 0x05    | standard read sequence    */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),   /*  WT_DMA0 S,0x00               */


  uA_WD (0x100),     /* write sector 0x0 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x1              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x2              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x3              */
  uA_WD (0x100), 

  uA_WD (0x100),     /* write sector 0x4 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x5              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x6              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x7              */
  uA_WD (0x100), 

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-19)        /*  BRA -19 to WT_DMA             */
};

static const uint32_t uc_OOB_WRITE_4K[]  = {
  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x5),	/* LSH #RB, 0x5   | rB = 0x1000				*/
  uA_MV(uR2,uRC),		/* MV  #R2, #RC   | first 2 bytes of address ( lower )  */
  uA_MV(uR3,uRD),		/* MV  #R3, #RD   | 3rd byte of address			*/


  uA_WC(NAND_CMD_PROG_1ST), 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard write sequence		*/

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/


  uA_WD (0x80),		/* write 128 bytes							*/ 

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-12)	      /* BRA -12 to WT_DMA	*/

};

static const uint32_t uc_PAGE_WRITE_8K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0   | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC   | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD   | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80        |              */
  uA_WA(0x5),        /* WA 0x05    | standard read sequence    */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),        /*  WT_DMA0 S,0x00               */


  uA_WD (0x100),     /* write sector 0x0 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x1              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x2              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x3              */
  uA_WD (0x100), 

  uA_WD (0x100),     /* write sector 0x4 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x5              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x6              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x7              */
  uA_WD (0x100), 
  
  uA_WD (0x100),     /* write sector 0x8 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x9              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x10              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x11              */
  uA_WD (0x100), 

  uA_WD (0x100),     /* write sector 0x12 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x13              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x14              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x15              */
  uA_WD (0x100), 


  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-35)        /*  BRA -35 to WT_DMA             */
};


static const uint32_t uc_OOB_WRITE_8K[]  = {
  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x6),	/* LSH #RB, 0x6   | rB = 0x2000				*/
  uA_MV(uR2,uRC),		/* MV  #R2, #RC   | first 2 bytes of address ( lower )  */
  uA_MV(uR3,uRD),		/* MV  #R3, #RD   | 3rd byte of address			*/


  uA_WC(NAND_CMD_PROG_1ST), 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard write sequence		*/

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/


  uA_WD (0x100),		/* write 256 bytes							*/ 

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-12)	   /* BRA -12 to WT_DMA	*/

};


static const uint32_t uc_PAGE_OOB_WRITE_ECC[]  = {
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_MV(uR2,uRC), 	/* MV  #R2, #RC   | first 2 bytes of address ( lower )  	*/
  uA_MV(uR3,uRD), 	/* MV  #R3, #RD   | 3rd byte of address			*/
  uA_WC(NAND_CMD_PROG_1ST) , 	/* WC 0x80			| 	 				*/
  uA_WA(0x5), 			/* WA 0x05		| standard write sequence		*/
  
  uA_ANDI(uR2,0x0),	/* AND #R2 ,0x0	|						*/
  uA_ADDI(uR2,0x80),	/* ADD #R2, 0x80 	| r2=0x80					*/
  uA_LSH(uR2,0x4),	/* LSH #R2, 0x4	| r2 = 0x800					*/
  uA_ADDI(uR2,0x26),	/* ADD #R2, 0x26.	| r2 = 0x826, ecc offset			*/
  uA_WTS(uEVT_TOUT,1),	      /* WT_TOUT S, 0x01	| tADL = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WTS(uEVT_DMA_START,0), 	/* WT_DMA0 S,0x00							*/

  uA_WD (0x100), 		/* write sector 0x0 							*/
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),

  uA_MV(uR2,uRB),			/* MV #R2,#RB		| ECC offset ready				*/
  
  uA_WC(NAND_CMD_PROG_RAND),	/* start random input		 					*/
  uA_WA(0x2),			   /* 2 address cycles 							*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/
  uA_WD (0x18),			/* write  ECC data (24 bytes)						*/

  uA_SUBI(uRB,0x26),	   /* SUB #RB, 0x26.	| rB = 0x800, OOB offset ready	*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/
  uA_WD (0x26),		/* write OOB data (38 bytes)					*/

  uA_ADDI(uRB,0x3E),	/* ADD #RB, 0x3E.	| rB = 0x83E, HM ECC offset ready	*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/
  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/
  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-26)		   /* BRA -26 to WT_DMA	*/
};

static const uint32_t uc_PAGE_OOB_WRITE_ECC_4K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0    | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC    | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD    | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80  */
  uA_WA(0x5),        /* WA 0x05         | standard write sequence      */
  
  uA_ANDI(uR2,0x0),  /* AND #R2 ,0x0    |                 */
  uA_ADDI(uR2,0x80), /* ADD #R2, 0x80   | r2=0x80               */
  uA_LSH(uR2,0x4),   /* LSH #R2, 0x4    | r2 = 0x800               */
  uA_MV(uR2,uR3),    /* MV  #R2, #R3    | r3 = 0x800, main data offset         */ 
  uA_LSH(uR2,0x1),   /* LSH #R2, 0x1    | r2 = 0x1000              */
  uA_ADDI(uR2,0x4C), /* ADD #R2, 0x4C.  | r2 = 0x104C, ecc offset         */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00                     */

  uA_WD (0x100),     /* write sector 0x0                    */
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),

  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  
  uA_WC(NAND_CMD_PROG_RAND),  /* start random input                     */
  
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data (24 bytes)                */

  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x1                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),         /* write  ECC data*/

  uA_SUBI(uRB,0x64),	/* SUB #RB, 0x64.	| rB = 0x1000, OOB (1st) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (1st) data (38 bytes)				*/

  uA_ADDI(uRB,0x7C),	/* ADD #RB, 0x7C.	| rB = 0x107C, HM ECC (1st) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/

  uA_SUBI(uRB,0x56),	/* SUB #RB, 0x56.	| rB = 0x1026, OOB (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0x58),	/* ADD #RB, 0x58.	| rB = 0x107E, HM ECC (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-54)		   /* BRA -54 to WT_DMA	*/
};

static const uint32_t uc_PAGE_OOB_WRITE_ECC_8K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0    | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC    | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD    | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80  */
  uA_WA(0x5),        /* WA 0x05         | standard write sequence      */
  
  uA_ANDI(uR2,0x0),  /* AND #R2 ,0x0    |                 */
  uA_ADDI(uR2,0x80), /* ADD #R2, 0x80   | r2=0x80               */
  uA_LSH(uR2,0x4),   /* LSH #R2, 0x4    | r2 = 0x800               */
  uA_MV(uR2,uR3),    /* MV  #R2, #R3    | r3 = 0x800, main data offset         */ 
  uA_MV(uR2,uRE),    /* MV  #R2, #RE	 | rE = 0x800, main data increment size */
  uA_LSH(uR2,0x2),   /* LSH #R2, 0x2    | r2 = 0x2000              */
  uA_ADDI(uR2,0x98), /* ADD #R2, 0x98.  | r2 = 0x2098, ecc offset         */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),   /* WT_DMA0 S,0x00                     */

  uA_WD (0x100),     /* write sector 0x0                    */
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),
  uA_WD (0x100),

  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  
  uA_WC(NAND_CMD_PROG_RAND),  /* start random input                     */
  
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data (24 bytes)                */

  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x1                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),         /* write  ECC data*/
  
  uA_ADD(uR3,uRE),	/* ADD #R3, #RE	| move main data pointer */
  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x2                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data*/

  uA_ADD(uR3,uRE),	/* ADD #R3, #RE	| move main data pointer */
  uA_MV(uR3,uRB),    /* rb <- r3, next sector address ready                */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */

  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),     /* write sector 0x3                    */
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  
  uA_WD (0x100),  

  uA_ADDI(uR2,0x18), /* ADD #R2, 0x18   | r2 += 24, move ecc pointer        */
  uA_MV(uR2,uRB),    /* MV #R2,#RB      | ECC offset ready            */
  uA_WC(NAND_CMD_PROG_RAND),  /* start random output                    */
  uA_WA(0x2),        /* 2 address cycles                    */
  
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x18),      /* write  ECC data*/

  uA_SUBI(uRB,0xE0),	/* SUB #RB, 0xE0.	| rB = 0x2000, OOB (1st) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (1st) data (38 bytes)				*/

  uA_ADDI(uRB,0xF8),	/* ADD #RB, 0xF8.	| rB = 0x20F8, HM ECC (1st) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/

  uA_SUBI(uRB,0xD2),	/* SUB #RB, 0xD2.	| rB = 0x2026, OOB (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0xD4),	/* ADD #RB, 0xD4.	| rB = 0x20FA, HM ECC (2nd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/


  uA_SUBI(uRB,0xAE),	/* SUB #RB, 0xAE.	| rB = 0x204C, OOB (3rd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0xB0),	/* ADD #RB, 0xB0.	| rB = 0x20FC, HM ECC (3rd) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/


  uA_SUBI(uRB,0x8A),	/* SUB #RB, 0x8A.	| rB = 0x2072, OOB (4th) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x26),		/* write OOB (2nd) data (38 bytes)				*/

  uA_ADDI(uRB,0x8C),	/* ADD #RB, 0x8C.	| rB = 0x20FE, HM ECC (3th) offset ready*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD(0x2),			/* write HM ECC data (2 bytes)					*/


  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-112)	      /* BRA -112 to WT_DMA	*/

};


static const uint32_t uc_PAGE_OOB_WRITE[]  = {
  uA_ANDI(uRB,0x0), 	/* AND #RB, 0x0   | 2 bytes of column 			*/
  uA_MV(uR2,uRC), 	/* MV  #R2, #RC   | first 2 bytes of address ( lower )  	*/
  uA_MV(uR3,uRD), 	/* MV  #R3, #RD   | 3rd byte of address			*/
  uA_WC(NAND_CMD_PROG_1ST) , 	/* WC 0x80        | 	 				*/
  uA_WA(0x5), 			/* WA 0x05	  | standard read sequence		*/
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),	/*  WT_DMA0 S,0x00					*/


  uA_WD (0x100), 		/* write sector 0x0 */
  uA_WD (0x100),

  uA_WD (0x100), 		/* write sector 0x1 					*/
  uA_WD (0x100), 	

  uA_WD (0x100), 		/* write sector 0x2					*/
  uA_WD (0x100),

  uA_WD (0x100), 		/* write sector 0x3 					*/
  uA_WD (0x100), 

  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x4),	/* LSH #RB, 0x4   | rB = 0x800				*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x40),		/* write 64 bytes							*/ 

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-17)		/* BRA -17 to WT_DMA	*/
};


static const uint32_t uc_PAGE_OOB_WRITE_4K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0   | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC   | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD   | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80        |              */
  uA_WA(0x5),        /* WA 0x05    | standard read sequence    */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),   /*  WT_DMA0 S,0x00               */

  uA_WD (0x100),     /* write sector 0x0 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x1              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x2              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x3              */
  uA_WD (0x100), 

  uA_WD (0x100),     /* write sector 0x4 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x5              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x6              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x7              */
  uA_WD (0x100), 

  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x5),	/* LSH #RB, 0x5   | rB = 0x1000				*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x80),		/* write 128 bytes							*/ 

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-25)	      /* BRA -12 to WT_DMA	*/
};

static const uint32_t uc_PAGE_OOB_WRITE_8K[]  = {
  uA_ANDI(uRB,0x0),  /* AND #RB, 0x0   | 2 bytes of column        */
  uA_MV(uR2,uRC),    /* MV  #R2, #RC   | first 2 bytes of address ( lower )   */
  uA_MV(uR3,uRD),    /* MV  #R3, #RD   | 3rd byte of address         */
  uA_WC(NAND_CMD_PROG_1ST) ,  /* WC 0x80        |              */
  uA_WA(0x5),        /* WA 0x05    | standard read sequence    */
  uA_WTS(uEVT_TOUT,1),  /* WT_TOUT S, 0x01 | tADL = 70 nsec min, this tout ~320 ns   @100MHz  */

  uA_WTS(uEVT_DMA_START,0),        /*  WT_DMA0 S,0x00               */


  uA_WD (0x100),     /* write sector 0x0 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x1              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x2              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x3              */
  uA_WD (0x100), 

  uA_WD (0x100),     /* write sector 0x4 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x5              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x6              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x7              */
  uA_WD (0x100), 
  
  uA_WD (0x100),     /* write sector 0x8 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x9              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x10              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x11              */
  uA_WD (0x100), 

  uA_WD (0x100),     /* write sector 0x12 */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x13              */
  uA_WD (0x100),  

  uA_WD (0x100),     /* write sector 0x14              */
  uA_WD (0x100),

  uA_WD (0x100),     /* write sector 0x15              */
  uA_WD (0x100), 

  uA_ADDI(uRB,0x80),	/* ADD #RB, 0x80  | rB = 0x80         		      	*/
  uA_LSH(uRB,0x6),	/* LSH #RB, 0x6   | rB = 0x2000				*/

  uA_WC(NAND_CMD_PROG_RAND),	/* WC 0x85		| start random input			*/
  uA_WA(0x2),			/* WA 0x02		| standard write sequence		*/
  uA_WTS(uEVT_TOUT,1),	/* WT_TOUT S, 0x01	| tCCS = 70 nsec min, this tout ~320 ns	@100MHz	*/

  uA_WD (0x100),		/* write 256 bytes							*/ 

  uA_WC(NAND_CMD_PROG_2ND),   /* WC 0x10    |               */

  uA_BRA(-41)	   /* BRA -41 to WT_DMA	*/
};

#endif

#endif

