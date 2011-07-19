/*****************************************************************************
* Copyright 2008 Broadcom Corporation.  All rights reserved.
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

#ifndef MPUHW_H__
#define MPUHW_H__

#include <mach/reg_utils.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_mpu.h>
//#include <mach/csp/reg.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef enum
{
  mpuHw_MEMORY_ACCESS_SECURE        = 0x0,                /* Memory region secure */
  mpuHw_MEMORY_ACCESS_VC_SECURE     = 0x1,                /* Memory region VC secure */
  mpuHw_MEMORY_ACCESS_VC_SUPERVISOR = 0x2,                /* Memory region VC Supervisor */
  mpuHw_MEMORY_ACCESS_OPEN          = 0x3                 /* Memory region open */
}mpuHw_MEMORY_ACCESS_e;


typedef enum
{
  mpuHw_MEMORY_REGION_0K_4K = 0,                  /* SRAM Memory space from 0K to 4K */
  mpuHw_MEMORY_REGION_4K_8K = 1,                  /* SRAM Memory space from 4K to 8K */
  mpuHw_MEMORY_REGION_8K_12K = 2,                 /* SRAM Memory space from 8K to 12K */
  mpuHw_MEMORY_REGION_12K_16K = 3,                /* SRAM Memory space from 12K to 16K */
  mpuHw_MEMORY_REGION_16K_20K = 4,                /* SRAM Memory space from 16K to 20K */
  mpuHw_MEMORY_REGION_20K_24K = 5,                /* SRAM Memory space from 20K to 24K */
  mpuHw_MEMORY_REGION_24K_28K = 6,                /* SRAM Memory space from 24K to 28K */
  mpuHw_MEMORY_REGION_28K_32K = 7,                /* SRAM Memory space from 28K to 32K */
  mpuHw_MEMORY_REGION_32K_36K = 8,                /* SRAM Memory space from 32K to 36K */
  mpuHw_MEMORY_REGION_36K_40K = 9,                /* SRAM Memory space from 36K to 40K */
  mpuHw_MEMORY_REGION_40K_44K = 10,               /* SRAM Memory space from 40K to 44K */
  mpuHw_MEMORY_REGION_44K_48K = 11,               /* SRAM Memory space from 44K to 48K */
  mpuHw_MEMORY_REGION_48K_52K = 12,               /* SRAM Memory space from 48K to 52K */
  mpuHw_MEMORY_REGION_52K_56K = 13,               /* SRAM Memory space from 52K to 56K */
  mpuHw_MEMORY_REGION_56K_60K = 14,               /* SRAM Memory space from 56K to 60K */
  mpuHw_MEMORY_REGION_60K_64K = 15,               /* SRAM Memory space from 60K to 64K */
  mpuHw_MEMORY_REGION_64K_68K = 16,               /* SRAM Memory space from 64K to 68K */
  mpuHw_MEMORY_REGION_68K_72K = 17,               /* SRAM Memory space from 68K to 72K */
  mpuHw_MEMORY_REGION_72K_76K = 18,               /* SRAM Memory space from 72K to 76K */
  mpuHw_MEMORY_REGION_76K_80K = 19,               /* SRAM Memory space from 76K to 80K */
  mpuHw_MEMORY_REGION_80K_84K = 20,               /* SRAM Memory space from 80K to 84K */
  mpuHw_MEMORY_REGION_84K_88K = 21,               /* SRAM Memory space from 84K to 88K */
  mpuHw_MEMORY_REGION_88K_92K = 22,               /* SRAM Memory space from 88K to 92K */
  mpuHw_MEMORY_REGION_92K_96K = 23,               /* SRAM Memory space from 92K to 96K */
  mpuHw_MEMORY_REGION_96K_100K = 24,              /* SRAM Memory space from 96K to 100K */
  mpuHw_MEMORY_REGION_100K_104K = 25,             /* SRAM Memory space from 100K to 104K */
  mpuHw_MEMORY_REGION_104K_108K = 26,             /* SRAM Memory space from 104K to 108K */
  mpuHw_MEMORY_REGION_108K_112K = 27,             /* SRAM Memory space from 108K to 112K */
  mpuHw_MEMORY_REGION_112K_116K = 28,             /* SRAM Memory space from 112K to 116K */
  mpuHw_MEMORY_REGION_116K_120K = 29,             /* SRAM Memory space from 116K to 120K */
  mpuHw_MEMORY_REGION_120K_124K = 30,             /* SRAM Memory space from 120K to 124K */
  mpuHw_MEMORY_REGION_124K_128K = 31,             /* SRAM Memory space from 124K to 128K */
  mpuHw_MEMORY_REGION_128K_132K = 32,             /* SRAM Memory space from 128K to 132K */
  mpuHw_MEMORY_REGION_132K_136K = 33,             /* SRAM Memory space from 132K to 136K */
  mpuHw_MEMORY_REGION_136K_140K = 34,             /* SRAM Memory space from 136K to 140K */
  mpuHw_MEMORY_REGION_140K_144K = 35,             /* SRAM Memory space from 140K to 144K */
  mpuHw_MEMORY_REGION_144K_148K = 36,             /* SRAM Memory space from 144K to 148K */
  mpuHw_MEMORY_REGION_148K_152K = 37,             /* SRAM Memory space from 148K to 152K */
  mpuHw_MEMORY_REGION_152K_156K = 38,             /* SRAM Memory space from 152K to 156K */
  mpuHw_MEMORY_REGION_156K_160K = 39,             /* SRAM Memory space from 156K to 160K */
  mpuHw_MEMORY_REGION_160K_164K = 40,             /* SRAM Memory space from 160K to 164K */
}mpuHw_MEMORY_REGION_e;


/****************************************************************************/
/**
*  @brief   Get the SRAM access permission of a particular 4KB region
*
*  
*  @return  SRAM access mode
*/
/****************************************************************************/
static inline mpuHw_MEMORY_ACCESS_e mpuHw_getSRAM_AccessMode 
( 
   mpuHw_MEMORY_REGION_e region 
)
{
   uint32_t offset = MPU_SRAM_MPU_CTRL0_OFFSET + ( ( region >> 4 ) << 2 );

   return ( mpuHw_MEMORY_ACCESS_e ) ( ( reg32_read ( (uint32_t*) ( KONA_MPU_VA + offset ) ) & ( (0x3) << ( ( region & 0xF ) << 1 ) ) )
                                    >> ( ( region & 0xF ) << 1 ) );
}



/****************************************************************************/
/**
*  @brief   Set the SRAM access permission of a particular 4KB region
*
*  
*  @return  void
*
*/
/****************************************************************************/
static inline void mpuHw_setSRAM_AccessMode 
( 
   mpuHw_MEMORY_REGION_e region, 
   mpuHw_MEMORY_ACCESS_e access
)
{
   uint32_t offset = MPU_SRAM_MPU_CTRL0_OFFSET + ( ( region >> 4 ) << 2 );
   uint32_t mask = (0x3) << ( ( region & 0xF ) << 1 ) ;
   uint32_t val  = ( (0x3) & access ) << ( ( region & 0xF ) << 1 );
   
   reg32_write_masked ( (uint32_t*) (KONA_MPU_VA + offset), mask, val );
   
}


/****************************************************************************/
/**
*  @brief   Lock the SRAM  access permission of a particular 4KB region
*
*  
*  @return  void
*
*  @note    Once acccess mode is locked for a region, it can not be changed
*           until chip is reset
*/
/****************************************************************************/
static inline void mpuHw_lockSRAM_AccessMode 
(
   mpuHw_MEMORY_REGION_e region 
)
{
  uint32_t offset;
  uint32_t val;
  
  /* lock the settings */
   offset = MPU_SRAM_MPU_LOCK_0_OFFSET + ( ( region >> 5 ) << 2 );
   val = (0x1) << ( ( region & 0xF ) << 1 );

   reg32_set_bits ( (uint32_t*) (KONA_MPU_VA + offset), val );
}


/****************************************************************************/
/**
*  @brief   Set the SRAM access permission to open for entire region
*
*  
*  @return  void
*
*/
/****************************************************************************/
static inline void mpuHw_setSRAM_AccessOpen 
( 
   void
)
{
   reg32_write ( (uint32_t*) (KONA_MPU_VA + 0x0), 0xffffffff );
   reg32_write ( (uint32_t*) (KONA_MPU_VA + 0x4), 0xffffffff );
   reg32_write ( (uint32_t*) (KONA_MPU_VA + 0x8), 0x0000ffff );
}

#ifdef __cplusplus
}
#endif

#endif /* MPUHW_H__ */

