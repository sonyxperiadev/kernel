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
#ifndef _ETHHW_REG_H
#define _ETHHW_REG_H

/* ---- Include Files ---------------------------------------------------- */
#include <mach/memory.h>
#include <mach/io_map.h>
#include <mach/reg_utils.h>

/* ---- Public Constants and Types --------------------------------------- */
#define ETHHW_PORT_0         0
#define ETHHW_PORT_1         1
#define ETHHW_PORT_INT       8

#define ETHHW_REG_BASE              KONA_ESW_VA

#define ETHHW_REG8(x)               (*((volatile uint8_t *)(x)))
#define ETHHW_REG16(x)              (*((volatile uint16_t *)(x)))
#define ETHHW_REG32(x)              (*((volatile uint32_t *)(x)))
#define ETHHW_REG48(x)              (*((volatile uint64_t *)(x)))
#define ETHHW_REG64(x)              (*((volatile uint64_t *)(x)))

extern void ethHw_get64( uint32_t addr, volatile uint64_t *datap );
extern void ethHw_set64_byVal( uint32_t addr, volatile uint64_t data );
extern void ethHw_set64_byRef( uint32_t addr, volatile uint64_t *datap );

static inline uint64_t eth_reg64_read( uint32_t addr )
{
   volatile uint64_t value;

   ethHw_get64( addr, &value );

   return value;
}

static inline void eth_reg64_write( uint32_t addr, uint64_t value )
{
   ethHw_set64_byRef(addr, &value);
}

/* A dummy read outside of the Ethernet block is required before any switch register read
 * to ensure the value read back is correct.  Sometimes a 0 is read without
 * the extra dummy read.  A dummy read is only required for read-modify-write
 * or read, but not write.
*/

static volatile uint32_t dummy;
#define regDummyRead()   dummy = reg32_read( ((void __iomem *)KONA_SRAM_VA) )

#define ETHHW_REG_SET( reg, val ) eth_reg64_write( (uint32_t)(&(reg)), (uint64_t)(val) )

#define ETHHW_REG_GET( reg, val ) { \
                                    regDummyRead(); \
                                    (val) = (uint32_t) eth_reg64_read( (uint32_t)(&(reg)) ); \
				                      }

#define ETHHW_REG_GET_VAL( reg ) ( regDummyRead(), ( (uint32_t)eth_reg64_read( ((uint32_t)&(reg)) ) ) )

#define ETHHW_REG_MASK_SET(reg,mask) ETHHW_REG_MASK_SET64( ((uint32_t)&(reg)), ((uint64_t)(mask)) )
#define ETHHW_REG_MASK_CLR(reg,mask) ETHHW_REG_MASK_CLR64( ((uint32_t)&(reg)), ((uint64_t)(mask)) )

#define ETHHW_REG_WRITE_MASKED( reg, mask, data )  { \
                                                         volatile uint32_t val; \
                                                         ETHHW_REG_GET( (reg), (val) ); \
                                                         val &= ~(mask); \
                                                         val |= ((data) & (mask)); \
                                                         ETHHW_REG_SET( (reg), (val) ); \
                                                      }


#define ethHw_reg_get64(addr, datap) do { \
                                           regDummyRead(); \
                                           ethHw_get64( (addr), (datap) ); \
                                        } while (0);

#if 1
/* Use byRef since the compiler builds images differently for different targets
 * For example, in the byVal code:
 *    -csptest image uses:
 *       -r0 for the addr argument
 *       -r1 and r2 for the data argument
 *    -Linux image uses:
 *       -r0 for the addr argument
 *       -r2 and r3 for the data argument
 * In the byRef code:
 *    -Both csptest and Linux images use:
 *       -r0 for the addr argument
 *       -r1 for the datap argument
 */
/* TODO: Figure out why the same toolchain uses different calling convention
 *       when building certain targets (possibly due to 64-bit data type)
 */
#define ethHw_reg_set64(a, d)       do { \
                                           regDummyRead(); \
                                           ethHw_set64_byRef((a), &(d)); \
                                       } while (0);
#else
#define ethHw_reg_set64(a, d)       do { \
                                           regDummyRead(); \
                                           ethHw_set64_byVal((a), (d)); \
                                       } while (0);
#endif

#define ethHw_reg_mask_set64(addr, mask)  do { \
                                                volatile uint64_t data; \
                                                ethHw_reg_get64( (addr), &(data) ); \
                                                (data) |= (mask); \
                                                ethHw_reg_set64( (addr), (data) ); \
                                             } while (0);

#define ethHw_reg_mask_clr64(addr, mask)  do { \
                                                volatile uint64_t data; \
                                                ethHw_reg_get64( (addr), &(data) ); \
                                                (data) &= ~(mask); \
                                                ethHw_reg_set64( (addr), (data) ); \
                                             } while (0);

#define ETHHW_REG_GET64( addr, datap )                ethHw_reg_get64( (addr), (datap) )
#define ETHHW_REG_SET64( addr, data )                 ethHw_reg_set64( (addr), (data) )
#define ETHHW_REG_MASK_SET64( addr, mask )            ethHw_reg_mask_set64( (addr), (mask) )
#define ETHHW_REG_MASK_CLR64( addr, mask )            ethHw_reg_mask_clr64( (addr), (mask) )
#define ETHHW_REG_WRITE_MASKED64( addr, mask, data )  { \
                                                         volatile uint64_t val; \
                                                         ETHHW_REG_GET64( (addr), &(val) ); \
                                                         val &= ~(mask); \
                                                         val |= ((data) & (mask)); \
                                                         ETHHW_REG_SET64( (addr), (val) ); \
                                                      }

#define ETHHW_REG_PAGEADDR(p, a)             (ETHHW_REG_BASE | (((uint8_t)(p) << 8) + (uint8_t)(a)) * 8)

#define ETHHW_REG_PAGE_CTRL                  0x00
#define ETHHW_REG_PAGE_STATUS                0x01
#define ETHHW_REG_PAGE_MGMT                  0x02
#define ETHHW_REG_PAGE_ARL                   0x05
#define ETHHW_REG_PAGE_MII(port)             (0x10+(port))
#define ETHHW_REG_PAGE_MIB(port)             (0x20+(port))
#define ETHHW_REG_PAGE_IMP_CTRL              0x34
#define ETHHW_REG_PAGE_MII_EXT(port)         (0x80+(port))


/* Page 00h, Control Register */
#define ethHw_regCtrlPortCtrl(port)                   ETHHW_REG8(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_CTRL, (port)))
#define ETHHW_REG_CTRL_PORTCTRL_STP_STATE_ENUM_OFFSET 1
#define ETHHW_REG_CTRL_PORTCTRL_STP_STATE_SHIFT       5
#define ETHHW_REG_CTRL_PORTCTRL_STP_STATE_MASK        (7 << ETHHW_REG_CTRL_PORTCTRL_STP_STATE_SHIFT)
#define ETHHW_REG_CTRL_PORTCTRL_MAC_TX_DISABLE        0x002
#define ETHHW_REG_CTRL_PORTCTRL_MAC_RX_DISABLE        0x001
#define ETHHW_REG_CTRL_PORTCTRL_MAC_DISABLE           (ETHHW_REG_CTRL_PORTCTRL_MAC_TX_DISABLE|ETHHW_REG_CTRL_PORTCTRL_MAC_RX_DISABLE)

#define ethHw_regCtrlImpCtrl                          ETHHW_REG8(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_CTRL, 0x08))
#define ETHHW_REG_CTRL_IMPCTRL_RX_UCAST_EN            0x10
#define ETHHW_REG_CTRL_IMPCTRL_RX_MCAST_EN            0x08
#define ETHHW_REG_CTRL_IMPCTRL_RX_BCAST_EN            0x04

#define ethHw_regCtrlSwitchMode                       ETHHW_REG8(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_CTRL, 0x0B))
#define ETHHW_REG_CTRL_SW_FWDG_EN                     0x02
#define ETHHW_REG_CTRL_SW_FWDG_MODE                   0x01

#define ethHw_regCtrlProtectPortSel                   ETHHW_REG16( ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_CTRL,0x24) )

/* Page 01h, Status Register */
#define ethHw_regStatusLinkStatus                     ETHHW_REG16(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_STATUS, 0x00))
#define ethHw_regStatusLinkStatusChange               ETHHW_REG16(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_STATUS, 0x02))
#define ethHw_regStatusPortSpeed                      ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_STATUS, 0x04))
#define ETHHW_REG_STATUS_PORT_SPEED_MASK              0x3
#define ETHHW_REG_STATUS_PORT_SPEED_10MB              0x0
#define ETHHW_REG_STATUS_PORT_SPEED_100MB             0x1
#define ETHHW_REG_STATUS_PORT_SPEED_1000MB            0x2
#define ETHHW_REG_STATUS_PORT_SPEED_SHIFT(port)       (2*(port))


/* Page 02h, Management Mode Register */
#define ethHw_regMgmtGblMgmtCtrl                      ETHHW_REG8(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MGMT, 0x00))
#define ETHHW_REG_MGMT_GBLMGMTCTRL_IMP_MODE_MASK      0xC0
#define ETHHW_REG_MGMT_GBLMGMTCTRL_IMP_MODE_EN        0x80
#define ETHHW_REG_MGMT_GBLMGMTCTRL_RX_BPDU_EN         0x02


/* Page 05h, ARL Access Register */
#define ethHw_regArlRwCtrl                            ETHHW_REG8(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, 0x00))
#define ETHHW_REG_ARL_RWCTRL_START_DONE               0x80
#define ETHHW_REG_ARL_RWCTRL_READ                     0x01
#define ETHHW_REG_ARL_RWCTRL_WRITE                    0x00
#define ETHHW_REG_ARL_MAC_INDEX                       ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, 0x02)
#define ethHw_regArlMacIndex                          use_ethHw_get64_or_ethHw_set64_function /*ETHHW_REG48(ETHHW_REG_ARL_MAC_INDEX) */
#define ethHw_regArlVlanIndex                         ETHHW_REG16(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, 0x08))
#define ETHHW_REG_ARL_MAC_VID_ENTRY(x)                ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, (0x10 + ((x)*0x10)))
#define ethHw_regArlMacVidEntry(x)                    use_ethHw_get64_or_ethHw_set64_function /*ETHHW_REG64(ETHHW_REG_ARL_MAC_VID_ENTRY((x))) */
#define ethHw_regArlFwdEntry(x)                       ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, (0x18 + ((x)*0x10))))
#define ETHHW_REG_ARL_FWDENTRY_VALID                  0x00010000
#define ETHHW_REG_ARL_FWDENTRY_STATIC                 0x00008000
#define ETHHW_REG_ARL_FWDENTRY_AGE                    0x00004000
#define ETHHW_REG_ARL_FWDENTRY_TC_SHIFT               11
#define ethHw_regArlSearchCtrl                        ETHHW_REG8(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, 0x50))
#define ethHw_regArlSearchAddr                        ETHHW_REG16(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, 0x51))
#define ETHHW_REG_ARL_SEARCH_MAC_VID(x)               ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, (0x60 + ((x)*0x10)))
#define ethHw_regArlSearchMacVid(x)                   use_ethHw_get64_or_ethHw_set64_function /*ETHHW_REG64(ETHHW_REG_ARL_SEARCH_MAC_VID((x)) */
#define ethHw_regArlSearchResult(x)                   ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, (0x68 + ((x)*0x10))))

/* #define ETHHW_REG_ARL_MAC_VID_ENTRY0                  ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, 0x10) */
/* #define ethHw_regArlMacVidEntry0                      use_ethHw_get64_or_ethHw_set64_function ETHHW_REG64(ETHHW_REG_ARL_MAC_VID_ENTRY0) */
/* #define ethHw_regArlFwdEntry0                         ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_ARL, 0x18)) */

#define ETHHW_REG_ARL_TIMEOUT                         100
#define ETHHW_REG_ARL_ENTRY_MIN                       0
#define ETHHW_REG_ARL_ENTRY_MAX                       3

/* Page 34h, IMP Control Register */
#define ethHw_regImpEnable                               ETHHW_REG8( ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_IMP_CTRL,0x06) )
#define ETHHW_REG_IMP_ENABLE                          0x00000001

/* Page 10h-11h, MII Register */
#define ethHw_regMdioAddr                                ETHHW_REG32(ETHHW_REG_BASE + 0x000A0008)
#define ethHw_regMdioWriteData                           ETHHW_REG32(ETHHW_REG_BASE + 0x000A0010)
#define ethHw_regMdioReadData                            ETHHW_REG32(ETHHW_REG_BASE + 0x000A0018)
#define ethHw_regMdioStatus                              ETHHW_REG32(ETHHW_REG_BASE + 0x000A0020)
#define ETHHW_REG_MDIO_SERDES_MASK                    0x00000010
#define ETHHW_REG_MDIO_DONE_MASK                      0x00000008
#define ETHHW_REG_MDIO_EXTRA_MASK                     0x00000004
#define ETHHW_REG_MDIO_READ_MASK                      0x00000002
#define ETHHW_REG_MDIO_WRITE_MASK                     0x00000001

#define ETHHW_REG_MII_TIMEOUT                         10000


/* Use physical address when specifying MDIO address */
#define ETHHW_REG_MII_EXT(port, offset)               ((ETHHW_REG_PAGE_MII_EXT((port)) << 8) | (uint8_t)(offset))
#define ETHHW_REG_MII_GPHY(port, offset)              ((ETHHW_REG_PAGE_MII((port)) << 8) | (uint8_t)(offset))

#define ETHHW_REG_MII                                 ETHHW_REG_MII_GPHY

/* Page 20h, Port MIB Register */
#define ETHHW_REG_MIB_TX_OCTETS(port)                 ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x00)
#define ETHHW_REG_MIB_TxOctets                        ETHHW_REG_MIB_TX_OCTETS
#define ethHw_regMibTxDropPkts(port)                  ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x08))
#define ethHw_regMibTxQ0Pkt(port)                     ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x0C))
#define ethHw_regMibTxBroadcastPkts(port)             ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x10))
#define ethHw_regMibTxMulticastPkts(port)             ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x14))
#define ethHw_regMibTxUnicastPkts(port)               ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x18))
#define ethHw_regMibTxCollisions(port)                ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x1C))
#define ethHw_regMibTxSingleCollision(port)           ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x20))
#define ethHw_regMibTxMultipleCollision(port)         ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x24))
#define ethHw_regMibTxDeferredTransmit(port)          ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x28))
#define ethHw_regMibTxLateCollision(port)             ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x2C))
#define ethHw_regMibTxExcessiveCollision(port)        ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x30))
#define ethHw_regMibTxFrameInDisc(port)               ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x34))
#define ethHw_regMibTxPausePkts(port)                 ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x38))
#define ethHw_regMibTxQ1Pkt(port)                     ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x3C))
#define ethHw_regMibTxQ2Pkt(port)                     ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x40))
#define ethHw_regMibTxQ3Pkt(port)                     ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x44))
#define ethHw_regMibTxQ4Pkt(port)                     ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x48))
#define ethHw_regMibTxQ5Pkt(port)                     ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x4C))
#define ETHHW_REG_MIB_RX_OCTETS(port)                 ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x50)
#define ETHHW_REG_MIB_RxOctets                        ETHHW_REG_MIB_RX_OCTETS
#define ethHw_regMibRxUndersizePkts(port)             ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x58))
#define ethHw_regMibRxPausePkts(port)                 ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x5C))
#define ethHw_regMibPkts64Octets(port)                ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x60))
#define ethHw_regMibPkts65To127Octets(port)           ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x64))
#define ethHw_regMibPkts128To255Octets(port)          ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x68))
#define ethHw_regMibPkts256To511Octets(port)          ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x6C))
#define ethHw_regMibPkts512To1023Octets(port)         ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x70))
#define ethHw_regMibPkts1024ToMaxPktOctets(port)      ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x74))
#define ethHw_regMibRxOversizePkts(port)              ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x78))
#define ethHw_regMibRxJabbers(port)                   ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x7C))
#define ethHw_regMibRxAlignmentErrors(port)           ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x80))
#define ethHw_regMibRxFcsErrors(port)                 ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x84))
#define ETHHW_REG_MIB_RX_GOOD_OCTETS(port)            ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x88)
#define ETHHW_REG_MIB_RxGoodOctets                    ETHHW_REG_MIB_RX_GOOD_OCTETS
#define ethHw_regMibRxDropPkts(port)                  ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x90))
#define ethHw_regMibRxUnicastPkts(port)               ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x94))
#define ethHw_regMibRxMulticastPkts(port)             ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x98))
#define ethHw_regMibRxBroadcastPkts(port)             ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0x9C))
#define ethHw_regMibRxSaChanges(port)                 ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xA0))
#define ethHw_regMibRxFragments(port)                 ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xA4))
#define ethHw_regMibJumboPktCount(port)               ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xA8))
#define ethHw_regMibRxSymbolError(port)               ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xAC))
#define ethHw_regMibInRangeErrorCount(port)           ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xB0))
#define ethHw_regMibOutRangeErrorCount(port)          ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xB4))
#define ethHw_regMibRxDiscard(port)                   ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xC0))
#define ethHw_regMibDosAssertCount(port)              ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xC4))
#define ethHw_regMibDosDropCount(port)                ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xC8))
#define ethHw_regMibIngressRateAssertCount(port)      ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xCC))
#define ethHw_regMibIngressRateDropCount(port)        ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xD0))
#define ethHw_regMibEgressRateAssertCount(port)       ETHHW_REG32(ETHHW_REG_PAGEADDR(ETHHW_REG_PAGE_MIB(port), 0xD4))


/* Switch interface register definitions */
#define ethHw_regPtmConfig                               ETHHW_REG32(ETHHW_REG_BASE | 0x80008)
#define ETHHW_REG_PTM_CONFIG_ENABLE                   0x90000000
#define ETHHW_REG_PTM_CONFIG_BURST(burst)             ((burst) << 1)
#define ETHHW_REG_PTM_CONFIG_BLOCK(block)             ((block) << 10)

#define ethHw_regPtmDmaStatus                            ETHHW_REG32(ETHHW_REG_BASE | 0x80010)

#define ethHw_regPtmFifoConfig                           ETHHW_REG32(ETHHW_REG_BASE | 0x88000)

#define ethHw_regMtpConfig                               ETHHW_REG32(ETHHW_REG_BASE | 0x90000)
#define ETHHW_REG_MTP_CONFIG_ENABLE                   0x90000001
/* Per design, burst size needs to be increased by one for non-A0 silicon */
#define ETHHW_REG_MTP_CONFIG_BURST(burst)             (((burst) + 1) << 1)

#define ethHw_regMtpFifoConfig                           ETHHW_REG32(ETHHW_REG_BASE | 0x90008)

#define ethHw_regMtpDmaStatus                            ETHHW_REG32(ETHHW_REG_BASE | 0x90010)

#define ethHw_regMtpFifoData                             ETHHW_REG32(ETHHW_REG_BASE | 0x98000)

#define ethHw_regImpPhyMisc                              ETHHW_REG32(ETHHW_REG_BASE | 0xa0000)

#define ETHHW_REG_IMP_PHY_MISC_POWERDOWN(port)        ((0x5 << (13 + (port))) & 0x0001e000)
#define ETHHW_REG_IMP_PHY_MISC_READY                  0x00800000
#define ETHHW_REG_READY_TIMEOUT                       100

#define ethHw_regImpSpeed                                ETHHW_REG32(ETHHW_REG_BASE | 0xa0090)
#define ETHHW_REG_IMP_SPEED_TX(s)                     ((s) << 4)
#define ETHHW_REG_IMP_SPEED_RX(s)                     ((s) << 0)
#define ETHHW_REG_IMP_SPEED(s)                        (ETHHW_REG_IMP_SPEED_TX(s) | ETHHW_REG_IMP_SPEED_RX(s))
#define ETHHW_REG_IMP_SPEED_MASK                      0xf
#define ETHHW_REG_IMP_SPEED_MASK_1000MBPS             0x4
#define ETHHW_REG_IMP_SPEED_MASK_100MBPS              0x1
#define ETHHW_REG_IMP_SPEED_MASK_10MBPS               0x3

#endif /* _ETHHW_REG_H */
