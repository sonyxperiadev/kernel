/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
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


/****************************************************************************/
/**
*  @file    ethHw.c
*
*  @brief   Low level ETH driver routines
*
*  @note
*
*   These routines provide basic ETH functionality only. Intended for use
*   with boot and other simple applications.
*/
/****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/module.h>
#include <mach/ethHw.h>
#include <mach/ethHw_reg.h>

/* ---- External Variable Declarations ----------------------------------- */
/* ---- External Function Prototypes ------------------------------------- */
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */

static uint16_t linkStatChg;

#define ONE_MHZ      (1 * 1000 * 1000)
#define CLOCK_FREQ   (100 * ONE_MHZ)

/* ---- Private Function Prototypes -------------------------------------- */
/* TODO: handle endianness */
/* mactou64(mac) and u64tomac(u64, mac) assumed little endian */

#define mactou64(mac)   (((uint64_t)((mac)[0]) << 40) + ((uint64_t)((mac)[1]) << 32) + \
                         ((uint64_t)((mac)[2]) << 24) + ((uint64_t)((mac)[3]) << 16) + \
                         ((uint64_t)((mac)[4]) << 8) + ((uint64_t)((mac)[5])))

#define u64tomac(u64, mac) {\
   mac[0] = ((uint8_t *)&u64)[5]; \
   mac[1] = ((uint8_t *)&u64)[4]; \
   mac[2] = ((uint8_t *)&u64)[3]; \
   mac[3] = ((uint8_t *)&u64)[2]; \
   mac[4] = ((uint8_t *)&u64)[1]; \
   mac[5] = ((uint8_t *)&u64)[0]; }

static int miiAccessDone(void);
static int arlAccessDone(void);

/* ==== Public Functions ================================================= */

/*****************************************************************************
* See ethHw.h for API documentation
*****************************************************************************/

int ethHw_Init(void)
{

   volatile uint32_t reg;

   /* Put switch registers in unsecure mode
   *  TODO:  This needs to be controlled some secure process
   */

   /* Configure internal port forwarding */
   ETHHW_REG_MASK_SET(ethHw_regCtrlSwitchMode,
   ETHHW_REG_CTRL_SW_FWDG_EN | ETHHW_REG_CTRL_SW_FWDG_MODE);

   reg = ETHHW_REG_GET_VAL(ethHw_regCtrlImpCtrl);
   reg |= ETHHW_REG_CTRL_IMPCTRL_RX_BCAST_EN;   /* Allow broadcast */
#if 0
   reg &= ~ETHHW_REG_CTRL_IMPCTRL_RX_MCAST_EN;  /* Drop unlearned multicast */
#else
   /* Enable multicast forwarding until multicast groups are supported */
   reg |= ETHHW_REG_CTRL_IMPCTRL_RX_MCAST_EN;   /* Allow unlearned multicast */
#endif
   reg &= ~ETHHW_REG_CTRL_IMPCTRL_RX_UCAST_EN;  /* Drop unlearned unicast */
   ETHHW_REG_SET(ethHw_regCtrlImpCtrl, reg);

   /* Note that the internal port is only configured at this point and not
   *  actually enabled yet.  The user needs to set-up the DMA first, and then
   *  enable the internal port by calling ethHw_impEnableSet()
   */

   return ETHHW_RC_NONE;
}


int ethHw_Exit(void)
{
   /* Application code will normally control shutdown of the driver */

   return ETHHW_RC_NONE;
}


int ethHw_stpStateSet(int port, ETHHW_STP_STATE state)
{
   volatile uint32_t reg;

   if (!ETHHW_VALID_PORT_EXT(port))
      return ETHHW_RC_PORT;
   if (!ETHHW_VALID_STP_STATE(state))
      return ETHHW_RC_PARAM;

   reg = ETHHW_REG_GET_VAL(ethHw_regCtrlPortCtrl(port));
   reg &= ~ETHHW_REG_CTRL_PORTCTRL_STP_STATE_MASK;
   reg |= (state << ETHHW_REG_CTRL_PORTCTRL_STP_STATE_SHIFT);
   ETHHW_REG_SET(ethHw_regCtrlPortCtrl(port), reg);

   return ETHHW_RC_NONE;
}


int ethHw_stpStateGet(int port, ETHHW_STP_STATE *statep)
{
   volatile uint32_t reg;

   *statep = ETHHW_VAR_UNINITIALIZED;
   if (!ETHHW_VALID_PORT_EXT(port))
      return ETHHW_RC_PORT;

   reg = ETHHW_REG_GET_VAL(ethHw_regCtrlPortCtrl(port));
   reg &= ETHHW_REG_CTRL_PORTCTRL_STP_STATE_SHIFT;

   *statep = reg >> ETHHW_REG_CTRL_PORTCTRL_STP_STATE_SHIFT;

   return ETHHW_RC_NONE;
}


int ethHw_miiSet(int port, ETHHW_MII_FLAGS flags, uint32_t addr, uint32_t data)
{
   uint32_t mdioAddr;
   uint32_t cmd;

   if (!ETHHW_VALID_PORT_EXT(port))
      return ETHHW_RC_PORT;

   addr <<= 1;    /* Convert MII register address to physical address */

   if (flags & ETHHW_MII_FLAGS_EXT)
      mdioAddr = ETHHW_REG_MII_EXT(port, addr);
   else
      mdioAddr = ETHHW_REG_MII(port, addr);

   cmd = ETHHW_REG_MDIO_DONE_MASK | ETHHW_REG_MDIO_EXTRA_MASK | ETHHW_REG_MDIO_WRITE_MASK;
   if (flags & ETHHW_MII_FLAGS_SERDES)
      cmd |= ETHHW_REG_MDIO_SERDES_MASK;

   ETHHW_REG_SET(ethHw_regMdioAddr, mdioAddr);
   ETHHW_REG_SET(ethHw_regMdioWriteData, data & 0x0000ffff);
   ETHHW_REG_SET(ethHw_regMdioStatus, cmd);

   if (!miiAccessDone())
      return ETHHW_RC_TIMEOUT;

   return ETHHW_RC_NONE;
}


int ethHw_miiGet(int port, ETHHW_MII_FLAGS flags, uint32_t addr, uint32_t *datap)
{
   uint32_t mdioAddr;
   uint32_t cmd;

   *datap = ETHHW_VAR_UNINITIALIZED;
   if (!ETHHW_VALID_PORT_EXT(port))
      return ETHHW_RC_PORT;

   addr <<= 1;    /* Convert MII register address to physical address */

   if (flags & ETHHW_MII_FLAGS_EXT)
      mdioAddr = ETHHW_REG_MII_EXT(port, addr);
   else
      mdioAddr = ETHHW_REG_MII(port, addr);

   cmd = (ETHHW_REG_MDIO_DONE_MASK | ETHHW_REG_MDIO_EXTRA_MASK | ETHHW_REG_MDIO_READ_MASK);
   if (flags & ETHHW_MII_FLAGS_SERDES)
      cmd |= ETHHW_REG_MDIO_SERDES_MASK;

   ETHHW_REG_SET(ethHw_regMdioAddr, mdioAddr);
   ETHHW_REG_SET(ethHw_regMdioStatus, cmd);

   if (!miiAccessDone())
      return ETHHW_RC_TIMEOUT;

   *datap = ETHHW_REG_GET_VAL(ethHw_regMdioReadData) & 0x0000ffff;

   return ETHHW_RC_NONE;
}


int ethHw_impEnableSet(int enable)
{
   volatile uint32_t reg;

   reg = ETHHW_REG_GET_VAL(ethHw_regMgmtGblMgmtCtrl);
   reg &= ~ETHHW_REG_MGMT_GBLMGMTCTRL_IMP_MODE_MASK;
   if (enable)
      reg |= ETHHW_REG_MGMT_GBLMGMTCTRL_IMP_MODE_EN;

   ETHHW_REG_SET(ethHw_regMgmtGblMgmtCtrl, reg);

   return ETHHW_RC_NONE;
}


int ethHw_impEnableGet(int *enablep)
{
   volatile uint32_t reg;

   reg = ETHHW_REG_GET_VAL(ethHw_regMgmtGblMgmtCtrl);
   *enablep = (((reg & ETHHW_REG_MGMT_GBLMGMTCTRL_IMP_MODE_EN) ==
               ETHHW_REG_MGMT_GBLMGMTCTRL_IMP_MODE_EN) ? 1 : 0);

   return ETHHW_RC_NONE;
}


int ethHw_impSpeedSet( int mbpsRx, int mbpsTx )
{
   volatile uint32_t reg;

   reg = ETHHW_REG_GET_VAL(ethHw_regImpSpeed);
   reg &= ~ETHHW_REG_IMP_SPEED(ETHHW_REG_IMP_SPEED_MASK);

   if (mbpsRx == 1000) {
      reg |= ETHHW_REG_IMP_SPEED_RX( ETHHW_REG_IMP_SPEED_MASK_1000MBPS );
   } else if (mbpsRx == 100) {
      reg |= ETHHW_REG_IMP_SPEED_RX( ETHHW_REG_IMP_SPEED_MASK_100MBPS );
   } else if (mbpsRx == 10) {
      reg |= ETHHW_REG_IMP_SPEED_RX( ETHHW_REG_IMP_SPEED_MASK_10MBPS );
   }

   if (mbpsTx == 1000) {
      reg |= ETHHW_REG_IMP_SPEED_TX( ETHHW_REG_IMP_SPEED_MASK_1000MBPS );
   } else if (mbpsTx == 100) {
      reg |= ETHHW_REG_IMP_SPEED_TX( ETHHW_REG_IMP_SPEED_MASK_100MBPS );
   } else if (mbpsTx == 10) {
      reg |= ETHHW_REG_IMP_SPEED_TX( ETHHW_REG_IMP_SPEED_MASK_10MBPS );
   }

   ETHHW_REG_SET(ethHw_regImpSpeed, reg);

   return ETHHW_RC_NONE;
}


int ethHw_impSpeedGet( int *mbpsRxp, int *mbpsTxp )
{
   volatile uint32_t reg;
   uint32_t mask;

   reg = ETHHW_REG_GET_VAL(ethHw_regImpSpeed);

   mask = reg & ETHHW_REG_IMP_SPEED_RX( ETHHW_REG_IMP_SPEED_MASK );
   if (mask == ETHHW_REG_IMP_SPEED_RX( ETHHW_REG_IMP_SPEED_MASK_1000MBPS )) {
      *mbpsRxp = 1000;
   } else if (mask == ETHHW_REG_IMP_SPEED_RX( ETHHW_REG_IMP_SPEED_MASK_100MBPS )) {
      *mbpsRxp = 100;
   } else if (mask == ETHHW_REG_IMP_SPEED_RX( ETHHW_REG_IMP_SPEED_MASK_10MBPS )) {
      *mbpsRxp = 10;
   } else {
      *mbpsRxp = -1;
   }

   mask = reg & ETHHW_REG_IMP_SPEED_TX( ETHHW_REG_IMP_SPEED_MASK );
   if (mask == ETHHW_REG_IMP_SPEED_TX( ETHHW_REG_IMP_SPEED_MASK_1000MBPS )) {
      *mbpsTxp = 1000;
   } else if (mask == ETHHW_REG_IMP_SPEED_TX( ETHHW_REG_IMP_SPEED_MASK_100MBPS )) {
      *mbpsTxp = 100;
   } else if (mask == ETHHW_REG_IMP_SPEED_TX( ETHHW_REG_IMP_SPEED_MASK_10MBPS )) {
      *mbpsTxp = 10;
   } else {
      *mbpsTxp = -1;
   }

   return ETHHW_RC_NONE;
}


int ethHw_arlEntrySet(char *macp, int vid, int port, int cos,
                      int entryValid, int entryStatic)
{
   int i;
   uint64_t mac, mac2;
   volatile uint32_t reg;

   /*
    * mac is endian-dependent.  Cannot use *((uint64_t *)(macp))
    * mac = *((uint64_t *)(macp));
    */
   mac = mactou64(macp);
   ETHHW_REG_SET64(ETHHW_REG_ARL_MAC_INDEX, mac);
   ETHHW_REG_SET(ethHw_regArlVlanIndex, vid);
   ETHHW_REG_SET(ethHw_regArlRwCtrl, (ETHHW_REG_ARL_RWCTRL_START_DONE | ETHHW_REG_ARL_RWCTRL_READ));
   if (!arlAccessDone())
      return ETHHW_RC_TIMEOUT;

   mac |= (((uint64_t)vid) << 48);

   for (i = ETHHW_REG_ARL_ENTRY_MIN; i <= ETHHW_REG_ARL_ENTRY_MAX; i++) {
      /* current entry is not valid */
      if ((ETHHW_REG_GET_VAL(ethHw_regArlFwdEntry(i)) & ETHHW_REG_ARL_FWDENTRY_VALID)) {
         /* check if MAC and VID are the same as existing entry */
         /* if yes, then update entry instead */
         ETHHW_REG_GET64(ETHHW_REG_ARL_MAC_VID_ENTRY(i), &mac2);
         if (mac2 == mac)
            break;
      } else {
         break;
      }
   }

   /* All entries are used */
   if (i > ETHHW_REG_ARL_ENTRY_MAX)
      return ETHHW_RC_FULL;

   /* Validate entry */
   reg = ETHHW_REG_ARL_FWDENTRY_AGE | (cos << ETHHW_REG_ARL_FWDENTRY_TC_SHIFT) | port;

   /* Validate entry */
   if (entryValid)
      reg |= ETHHW_REG_ARL_FWDENTRY_VALID;

   /* Static entry */
   if (entryStatic)
      reg |= ETHHW_REG_ARL_FWDENTRY_STATIC;

   ETHHW_REG_SET (ethHw_regArlFwdEntry(i), reg);

   ETHHW_REG_SET64(ETHHW_REG_ARL_MAC_VID_ENTRY(i), mac);

   mac = mactou64(macp);
   ETHHW_REG_SET64(ETHHW_REG_ARL_MAC_INDEX, mac);
   ETHHW_REG_SET(ethHw_regArlVlanIndex, vid);
   ETHHW_REG_SET(ethHw_regArlRwCtrl, (ETHHW_REG_ARL_RWCTRL_START_DONE | ETHHW_REG_ARL_RWCTRL_WRITE));
   if (!arlAccessDone())
      return ETHHW_RC_TIMEOUT;

   return ETHHW_RC_NONE;
}


int ethHw_portLinkStatus(int port)
{
   int up;

   up = ETHHW_VAR_UNINITIALIZED;
   if (!ETHHW_VALID_PORT(port))
      return ETHHW_RC_PORT;

   up = ((ETHHW_REG_GET_VAL(ethHw_regStatusLinkStatus) & ETHHW_PORT_MASK(port)) ? 1 : 0);

   return up;
}


int ethHw_portLinkStatusChange(int port)
{
   int change;

   change = ETHHW_VAR_UNINITIALIZED;
   if (!ETHHW_VALID_PORT(port))
      return ETHHW_RC_PORT;

   linkStatChg |= ETHHW_REG_GET_VAL(ethHw_regStatusLinkStatusChange);
   change = ((linkStatChg & ETHHW_PORT_MASK(port)) ? 1 : 0);
   linkStatChg &= ~ETHHW_PORT_MASK(port);

   return change;
}


int ethHw_portSpeed(int port)
{
   int speed;
   int speed_bmp;

   speed = ETHHW_VAR_UNINITIALIZED;
   if (!ETHHW_VALID_PORT(port))
      return ETHHW_RC_PORT;

   speed_bmp = (ETHHW_REG_GET_VAL(ethHw_regStatusPortSpeed) >> ETHHW_REG_STATUS_PORT_SPEED_SHIFT(port)) &
               ETHHW_REG_STATUS_PORT_SPEED_MASK;

   if (speed_bmp & ETHHW_REG_STATUS_PORT_SPEED_1000MB) {
      speed = 1000;
   } else if (speed_bmp & ETHHW_REG_STATUS_PORT_SPEED_100MB) {
      speed = 100;
   } else {
      speed = 10;
   }

   return speed;
}


int ethHw_macEnableSet(int port, int tx, int rx)
{
   volatile uint32_t reg;

   if (!(ETHHW_VALID_PORT_EXT(port) || ETHHW_VALID_PORT_INT(port)))
      return ETHHW_RC_PORT;

   reg = ETHHW_REG_GET_VAL(ethHw_regCtrlPortCtrl(port));

   if (tx)
      reg &= ~ETHHW_REG_CTRL_PORTCTRL_MAC_TX_DISABLE;
   else
      reg |= ETHHW_REG_CTRL_PORTCTRL_MAC_TX_DISABLE;

   if (rx)
      reg &= ~ETHHW_REG_CTRL_PORTCTRL_MAC_RX_DISABLE;
   else
      reg |= ETHHW_REG_CTRL_PORTCTRL_MAC_RX_DISABLE;

   ETHHW_REG_SET(ethHw_regCtrlPortCtrl(port), reg);

   return ETHHW_RC_NONE;
}


int ethHw_macEnableGet(int port, int *txp, int *rxp)
{
   volatile uint32_t reg;

   *txp = ETHHW_VAR_UNINITIALIZED;
   *rxp = ETHHW_VAR_UNINITIALIZED;
   if (!ETHHW_VALID_PORT_EXT(port))
      return ETHHW_RC_PORT;

   reg = ETHHW_REG_GET_VAL(ethHw_regCtrlPortCtrl(port));

   *txp = ((reg & ETHHW_REG_CTRL_PORTCTRL_MAC_TX_DISABLE) ? 0 : 1);
   *rxp = ((reg & ETHHW_REG_CTRL_PORTCTRL_MAC_RX_DISABLE) ? 0 : 1);

   return ETHHW_RC_NONE;
}


int ethHw_phyPowerdownSet(int port, int enable)
{
   volatile uint32_t reg;

   ETHHW_REG_GET(ethHw_regImpPhyMisc, reg);

   if (enable)
      reg |= ETHHW_REG_IMP_PHY_MISC_POWERDOWN(port);
   else
      reg &= ~ETHHW_REG_IMP_PHY_MISC_POWERDOWN(port);

   ETHHW_REG_SET(ethHw_regImpPhyMisc, reg);

   return ETHHW_RC_NONE;
}


int ethHw_phyPowerdownGet(int port, int *enablep)
{
   volatile uint32_t reg;

   ETHHW_REG_GET(ethHw_regImpPhyMisc, reg);

   *enablep = ((reg & ETHHW_REG_IMP_PHY_MISC_POWERDOWN(port)) ==
               ETHHW_REG_IMP_PHY_MISC_POWERDOWN(port));

   return ETHHW_RC_NONE;
}


int ethHw_Loopback( int port, int enable )
{
   volatile uint32_t reg;
   uint32_t reg32;

   if( enable )
   {
      ethHw_stpStateSet( port, ETHHW_STP_STATE_FORWARDING );

      /* Enable forwarding of unlearned packets
       */
      reg = ETHHW_REG_GET_VAL(ethHw_regCtrlImpCtrl);
      reg |= ETHHW_REG_CTRL_IMPCTRL_RX_MCAST_EN;  /* Forward unlearned multicast */
      reg |= ETHHW_REG_CTRL_IMPCTRL_RX_UCAST_EN;  /* Forward unlearned unicast */
      ETHHW_REG_SET(ethHw_regCtrlImpCtrl, reg);

      /* Disable bridging
       */
      reg = ETHHW_REG_GET_VAL(ethHw_regCtrlProtectPortSel);
      reg |= 1<<port;
      ETHHW_REG_SET(ethHw_regCtrlProtectPortSel, reg);

      /* Configure port for loopback
       */
      ethHw_miiGet( port, 0, 0, &reg32 );
      reg32 |= 0x4000;
      ethHw_miiSet( port, 0, 0, reg32 );
   }
   else
   {
      ethHw_stpStateSet( port, ETHHW_STP_STATE_DISABLED );

      /* Disable forwarding of unlearned packets
       */
      reg = ETHHW_REG_GET_VAL(ethHw_regCtrlImpCtrl);
      reg &= ~ETHHW_REG_CTRL_IMPCTRL_RX_MCAST_EN;  /* Forward unlearned multicast */
      reg &= ~ETHHW_REG_CTRL_IMPCTRL_RX_UCAST_EN;  /* Forward unlearned unicast */
      ETHHW_REG_SET(ethHw_regCtrlImpCtrl, reg);

      /* Enable bridging
       */
      reg = ETHHW_REG_GET_VAL(ethHw_regCtrlProtectPortSel);
      reg &= ~(1<<port);
      ETHHW_REG_SET(ethHw_regCtrlProtectPortSel, reg);

      /* Configure port for loopback
       */
      ethHw_miiGet( port, 0, 0, &reg32 );
      reg32 &= ~0x4000;
      ethHw_miiSet( port, 0, 0, reg32 );
   }

   return ETHHW_RC_NONE;
}

/* MIB functions */
#define MIB_FUNC_32(type) \
uint32_t ethHw_mib ## type (int port) \
{\
   if (!ETHHW_VALID_PORT_MIB(port)) \
      return 0; \
   \
   return ethHw_regMib ## type (port); \
} \
\
EXPORT_SYMBOL(ethHw_mib ## type);

#define MIB_FUNC_64(type) \
uint64_t ethHw_mib ## type (int port) \
{\
   uint64_t mib; \
   \
   if (!ETHHW_VALID_PORT_MIB(port)) \
      return 0; \
   \
   ethHw_get64(ETHHW_REG_MIB_ ## type (port), &mib); \
   \
   return mib; \
} \
\
EXPORT_SYMBOL(ethHw_mib ## type);

MIB_FUNC_64(TxOctets);
MIB_FUNC_32(TxDropPkts);
MIB_FUNC_32(TxQ0Pkt);
MIB_FUNC_32(TxBroadcastPkts);
MIB_FUNC_32(TxMulticastPkts);
MIB_FUNC_32(TxUnicastPkts);
MIB_FUNC_32(TxCollisions);
MIB_FUNC_32(TxSingleCollision);
MIB_FUNC_32(TxMultipleCollision);
MIB_FUNC_32(TxDeferredTransmit);
MIB_FUNC_32(TxLateCollision);
MIB_FUNC_32(TxExcessiveCollision);
MIB_FUNC_32(TxFrameInDisc);
MIB_FUNC_32(TxPausePkts);
MIB_FUNC_32(TxQ1Pkt);
MIB_FUNC_32(TxQ2Pkt);
MIB_FUNC_32(TxQ3Pkt);
MIB_FUNC_32(TxQ4Pkt);
MIB_FUNC_32(TxQ5Pkt);
MIB_FUNC_64(RxOctets);
MIB_FUNC_32(RxUndersizePkts);
MIB_FUNC_32(RxPausePkts);
MIB_FUNC_32(Pkts64Octets);
MIB_FUNC_32(Pkts65To127Octets);
MIB_FUNC_32(Pkts128To255Octets);
MIB_FUNC_32(Pkts256To511Octets);
MIB_FUNC_32(Pkts512To1023Octets);
MIB_FUNC_32(Pkts1024ToMaxPktOctets);
MIB_FUNC_32(RxOversizePkts);
MIB_FUNC_32(RxJabbers);
MIB_FUNC_32(RxAlignmentErrors);
MIB_FUNC_32(RxFcsErrors);
MIB_FUNC_64(RxGoodOctets);
MIB_FUNC_32(RxDropPkts);
MIB_FUNC_32(RxUnicastPkts);
MIB_FUNC_32(RxMulticastPkts);
MIB_FUNC_32(RxBroadcastPkts);
MIB_FUNC_32(RxSaChanges);
MIB_FUNC_32(RxFragments);
MIB_FUNC_32(JumboPktCount);
MIB_FUNC_32(RxSymbolError);
MIB_FUNC_32(InRangeErrorCount);
MIB_FUNC_32(OutRangeErrorCount);
MIB_FUNC_32(RxDiscard);
MIB_FUNC_32(DosAssertCount);
MIB_FUNC_32(DosDropCount);
MIB_FUNC_32(IngressRateAssertCount);
MIB_FUNC_32(IngressRateDropCount);
MIB_FUNC_32(EgressRateAssertCount);


/* ==== Exported Public Functions ======================================== */

EXPORT_SYMBOL(ethHw_Init);
EXPORT_SYMBOL(ethHw_Exit);

EXPORT_SYMBOL(ethHw_stpStateSet);
EXPORT_SYMBOL(ethHw_stpStateGet);

EXPORT_SYMBOL(ethHw_miiSet);
EXPORT_SYMBOL(ethHw_miiGet);

EXPORT_SYMBOL(ethHw_impEnableSet);
EXPORT_SYMBOL(ethHw_impEnableGet);
EXPORT_SYMBOL(ethHw_impSpeedSet);
EXPORT_SYMBOL(ethHw_impSpeedGet);

EXPORT_SYMBOL(ethHw_arlEntrySet);

EXPORT_SYMBOL(ethHw_portLinkStatus);
EXPORT_SYMBOL(ethHw_portLinkStatusChange);
EXPORT_SYMBOL(ethHw_portSpeed);

EXPORT_SYMBOL(ethHw_macEnableSet);
EXPORT_SYMBOL(ethHw_macEnableGet);

EXPORT_SYMBOL(ethHw_phyPowerdownSet);
EXPORT_SYMBOL(ethHw_phyPowerdownGet);

EXPORT_SYMBOL(ethHw_get64);
EXPORT_SYMBOL(ethHw_set64_byVal);
EXPORT_SYMBOL(ethHw_set64_byRef);

/* MIB public functions are exported in the function macros */

/* ==== Private Functions ================================================ */

static int miiAccessDone(void)
{
   int timeout;

   timeout = 0;
   while (ETHHW_REG_GET_VAL(ethHw_regMdioStatus) & ETHHW_REG_MDIO_DONE_MASK)
      if (timeout++ >= ETHHW_REG_MII_TIMEOUT)
         return 0;

   return 1;
}


static int arlAccessDone(void)
{
   int timeout;

   timeout = 0;
   while (ETHHW_REG_GET_VAL(ethHw_regArlRwCtrl) & ETHHW_REG_ARL_RWCTRL_START_DONE)
      if (timeout++ >= ETHHW_REG_ARL_TIMEOUT)
         return 0;

   return 1;
}
