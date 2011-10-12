/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
/**
 *
 *  @file    sigmorph-test.c
 *
 *  @brief   SIGMORPH TEST module.
 *
 ****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>
#include <linux/module.h>

#include <linux/broadcom/knllog.h>
#include <linux/broadcom/sigmorph.h>


/* ---- Private Constants and Types -------------------------------------- */

#define SIGMORPH_TESTBUF_SIZE     (10)

#define SIGMORPHTEST_DEBUG( msg, args... )KNLLOG("" msg,##args);
#define SIGMORPHTEST_DEBUG_DUMP_MEM(addr,mem,nbytes)KNLLOG_DUMP_MEM(addr,mem,nbytes);

static void sigmorph_test_mix ( void );
static void sigmorph_test_amplify ( void );

static char gBanner[] __initdata = KERN_INFO "BROADCOM SIGMORPH TEST module\n";

static int16_t op2 [SIGMORPH_TESTBUF_SIZE];
static int16_t op1 [SIGMORPH_TESTBUF_SIZE];
static int16_t exp [SIGMORPH_TESTBUF_SIZE];
static int16_t output [SIGMORPH_TESTBUF_SIZE];

/* ---- Functions -------------------------------------------------------- */

static void sigmorph_test_amplify ()
{
   int i = 0;
   /*
    * Test Cases:
    * 
    * TC1: gain < SIGMORPH_MIN_DB_GAIN
    * TC2: gain = SIGMORPH_MIN_DB_GAIN
    * TC3: gain E (SIGMORPH_MIN_DB_GAIN, 0)
    * TC4: gain = 0
    * TC5: gain E (0, SIGMORPH_MAX_DB_GAIN)
    * TC6: gain = SIGMORPH_MAX_DB_GAIN
    * TC7: gain > SIGMORPH_MAX_DB_GAIN
    * 
    */
#define STOP_TESTS (9999)
   int16_t gain[] = { -100, -50, -6, 0, 5, 18, 100, STOP_TESTS};

   do
   {
      op1[0] = 0xFFFF;  /* -1 */
      op1[1] = 0x7FFF;  /* Max +ve */
      op1[2] = 0x0000;  /* Degenerate case */
      op1[3] = 0x8000;  /* Max -ve */
      op1[4] = 0x1;     /* Additional */

      if (!i)
      {
         SIGMORPHTEST_DEBUG("------------ INPUT ------------");
         SIGMORPHTEST_DEBUG_DUMP_MEM (0, op1, 5 * sizeof (int16_t));
      }
      sigmorph_amplify ( op1, 5 * sizeof (int16_t), gain[i]);

      SIGMORPHTEST_DEBUG("--------- GAIN(0x%04hX) ---------", gain [i]);
      SIGMORPHTEST_DEBUG_DUMP_MEM (0, op1, 5 * sizeof (int16_t));

   } while ( gain [i++] != STOP_TESTS );
} /* sigmorph_test_amplify */

static void sigmorph_test_mix ()
{
   int err = 1;
   int i = 0;

   memset((void *)op1, 0, 5 * sizeof (int16_t));
   memset((void *)op2, 0, 5 * sizeof (int16_t));

   /*
    * Test cases:
    * 
    * TC1. sum > SIGMORPH_SATURATION_MAX
    * TC2. sum = SIGMORPH_SATURATION_MAX
    * TC3. sum < SIGMORPH_SATURATION_MIN
    * TC4. sum = SIGMORPH_SATURATION_MIN
    * TC5. sum E (SIGMORPH_SATURATION_MIN, SIGMORPH_SATURATION_MAX)
    */
   {
      /* TC1. _MAX + _MAX */
      op1[0] = SIGMORPH_SATURATION_MAX;
      op2[0] = 0x7FFF;
      exp[0] = SIGMORPH_SATURATION_MAX;
      SIGMORPHTEST_DEBUG( "TC1: %04hX [+] %04hX",op1[0],op2[0]);
      
      /* TC2. */
      op1[1] = 0x2AAA; 
      op2[1] = 0x5555; 
      exp[1] = SIGMORPH_SATURATION_MAX;
      SIGMORPHTEST_DEBUG( "TC2: %04hX [+] %04hX",op1[1],op2[1]);
      
      /* TC3. _MIN + (-1) */
      op1[2] = SIGMORPH_SATURATION_MIN; 
      op2[2] = 0xFFFF; 
      exp[2] = SIGMORPH_SATURATION_MIN;
      SIGMORPHTEST_DEBUG( "TC3: %04hX [+] %04hX",op1[2],op2[2]);
      
      /* TC4. (-)_MAX + (-1) */
      op1[3] = 0x8001; 
      op2[3] = 0xFFFF; 
      exp[3] = SIGMORPH_SATURATION_MIN;
      SIGMORPHTEST_DEBUG( "TC4: %04hX [+] %04hX",op1[3],op2[3]);
      
      /* TC5. Within range */
      op1[4] = 0x0AAA; 
      op2[4] = 0x0555;
      exp[4] = 0x0FFF;
      SIGMORPHTEST_DEBUG( "TC5: %04hX [+] %04hX",op1[4],op2[4]);
      
      err = sigmorph_mix ( output, op1, op2, 5 * sizeof (int16_t) );
      SIGMORPHTEST_DEBUG( "Return Code... %d", err);
      
      for (i = 0; i< 5; i++)
      {
         SIGMORPHTEST_DEBUG( "RESULT"
               " TC%d: [output %04hX] [expected %04hX] [%s]"
               , i+1
               , output[i]
               , exp[i]
               , ((output[i] == exp[i]) ? "PASS" : "FAIL"));
      }
   }
} /* sigmorph_test_mix */

static int __init sigmorph_test_init( void )
{
   printk (gBanner);
   
   sigmorph_test_mix();
   sigmorph_test_amplify();
   
   return 0;
}

static void __exit sigmorph_test_exit( void )
{
   /* exit */
}

module_init( sigmorph_test_init );
module_exit( sigmorph_test_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "CSX sigmorph TEST module" );
MODULE_LICENSE( "GPL" );
