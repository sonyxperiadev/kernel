#ifndef __MACH_POWERMAN_H
#define __MACH_POWERMAN_H

#include <mach/platform.h>

#define PM_PASSWORD                                              0x5a000000

#define PM_RSTC                                                  (PM_BASE + 0x0000001c)
   #define PM_RSTC_MASK                                          0x00333333
   #define PM_RSTC_WIDTH                                         22
   #define PM_RSTC_RESET                                         0x00000102
      #define PM_RSTC_HRCFG_BITS                                 21:20
      #define PM_RSTC_HRCFG_SET                                  0x00300000
      #define PM_RSTC_HRCFG_CLR                                  0xffcfffff
      #define PM_RSTC_HRCFG_MSB                                  21
      #define PM_RSTC_HRCFG_LSB                                  20
      #define PM_RSTC_FRCFG_BITS                                 17:16
      #define PM_RSTC_FRCFG_SET                                  0x00030000
      #define PM_RSTC_FRCFG_CLR                                  0xfffcffff
      #define PM_RSTC_FRCFG_MSB                                  17
      #define PM_RSTC_FRCFG_LSB                                  16
      #define PM_RSTC_QRCFG_BITS                                 13:12
      #define PM_RSTC_QRCFG_SET                                  0x00003000
      #define PM_RSTC_QRCFG_CLR                                  0xffffcfff
      #define PM_RSTC_QRCFG_MSB                                  13
      #define PM_RSTC_QRCFG_LSB                                  12
      #define PM_RSTC_SRCFG_BITS                                 9:8
      #define PM_RSTC_SRCFG_SET                                  0x00000300
      #define PM_RSTC_SRCFG_CLR                                  0xfffffcff
      #define PM_RSTC_SRCFG_MSB                                  9
      #define PM_RSTC_SRCFG_LSB                                  8
      #define PM_RSTC_WRCFG_BITS                                 5:4
      #define PM_RSTC_WRCFG_SET                                  0x00000030
      #define PM_RSTC_WRCFG_CLR                                  0xffffffcf
      #define PM_RSTC_WRCFG_MSB                                  5
      #define PM_RSTC_WRCFG_LSB                                  4
      #define PM_RSTC_DRCFG_BITS                                 1:0
      #define PM_RSTC_DRCFG_SET                                  0x00000003
      #define PM_RSTC_DRCFG_CLR                                  0xfffffffc
      #define PM_RSTC_DRCFG_MSB                                  1
      #define PM_RSTC_DRCFG_LSB                                  0

#define PM_RSTS                                                  (PM_BASE + 0x00000020)
   #define PM_RSTS_MASK                                          0x00001777
   #define PM_RSTS_WIDTH                                         13
   #define PM_RSTS_RESET                                         0x00001000
      #define PM_RSTS_HADPOR_BITS                                12:12
      #define PM_RSTS_HADPOR_SET                                 0x00001000
      #define PM_RSTS_HADPOR_CLR                                 0xffffefff
      #define PM_RSTS_HADPOR_MSB                                 12
      #define PM_RSTS_HADPOR_LSB                                 12
      #define PM_RSTS_HADSRH_BITS                                10:10
      #define PM_RSTS_HADSRH_SET                                 0x00000400
      #define PM_RSTS_HADSRH_CLR                                 0xfffffbff
      #define PM_RSTS_HADSRH_MSB                                 10
      #define PM_RSTS_HADSRH_LSB                                 10
      #define PM_RSTS_HADSRF_BITS                                9:9
      #define PM_RSTS_HADSRF_SET                                 0x00000200
      #define PM_RSTS_HADSRF_CLR                                 0xfffffdff
      #define PM_RSTS_HADSRF_MSB                                 9
      #define PM_RSTS_HADSRF_LSB                                 9
      #define PM_RSTS_HADSRQ_BITS                                8:8
      #define PM_RSTS_HADSRQ_SET                                 0x00000100
      #define PM_RSTS_HADSRQ_CLR                                 0xfffffeff
      #define PM_RSTS_HADSRQ_MSB                                 8
      #define PM_RSTS_HADSRQ_LSB                                 8
      #define PM_RSTS_HADWRH_BITS                                6:6
      #define PM_RSTS_HADWRH_SET                                 0x00000040
      #define PM_RSTS_HADWRH_CLR                                 0xffffffbf
      #define PM_RSTS_HADWRH_MSB                                 6
      #define PM_RSTS_HADWRH_LSB                                 6
      #define PM_RSTS_HADWRF_BITS                                5:5
      #define PM_RSTS_HADWRF_SET                                 0x00000020
      #define PM_RSTS_HADWRF_CLR                                 0xffffffdf
      #define PM_RSTS_HADWRF_MSB                                 5
      #define PM_RSTS_HADWRF_LSB                                 5
      #define PM_RSTS_HADWRQ_BITS                                4:4
      #define PM_RSTS_HADWRQ_SET                                 0x00000010
      #define PM_RSTS_HADWRQ_CLR                                 0xffffffef
      #define PM_RSTS_HADWRQ_MSB                                 4
      #define PM_RSTS_HADWRQ_LSB                                 4
      #define PM_RSTS_HADDRH_BITS                                2:2
      #define PM_RSTS_HADDRH_SET                                 0x00000004
      #define PM_RSTS_HADDRH_CLR                                 0xfffffffb
      #define PM_RSTS_HADDRH_MSB                                 2
      #define PM_RSTS_HADDRH_LSB                                 2
      #define PM_RSTS_HADDRF_BITS                                1:1
      #define PM_RSTS_HADDRF_SET                                 0x00000002
      #define PM_RSTS_HADDRF_CLR                                 0xfffffffd
      #define PM_RSTS_HADDRF_MSB                                 1
      #define PM_RSTS_HADDRF_LSB                                 1
      #define PM_RSTS_HADDRQ_BITS                                0:0
      #define PM_RSTS_HADDRQ_SET                                 0x00000001
      #define PM_RSTS_HADDRQ_CLR                                 0xfffffffe
      #define PM_RSTS_HADDRQ_MSB                                 0
      #define PM_RSTS_HADDRQ_LSB                                 0

#define PM_WDOG                                                  (PM_BASE + 0x00000024)
   #define PM_WDOG_MASK                                          0x000fffff
   #define PM_WDOG_WIDTH                                         20
   #define PM_WDOG_RESET                                         0000000000
      #define PM_WDOG_TIME_BITS                                  19:0
      #define PM_WDOG_TIME_SET                                   0x000fffff
      #define PM_WDOG_TIME_CLR                                   0xfff00000
      #define PM_WDOG_TIME_MSB                                   19
      #define PM_WDOG_TIME_LSB                                   0

#endif /* __MACH_POWERMAN_H */
