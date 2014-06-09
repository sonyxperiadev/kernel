////////////////////////////////////////////////////////////////////////////////
///
/// \file   SentralConfig.h
///
/// \authors   Evan Lojewski
///
/// \brief     Firmware configuration structure
///
////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
///
/// \copyright (C) 2013 EM Microelectronic US Inc.
///
/// \copyright Disclosure to third parties or reproduction in any form
///   whatsoever, without prior written consent, is strictly forbidden
///
////////////////////////////////////////////////////////////////////////////////
#ifndef SENTRAL_CONFIG_H
#define SENTRAL_CONFIG_H

#include <types.h>

#define SENTRAL_CONFIG_MAGIC_NUMBER  0xC82B/**< Special value stored in the SentralConfig MagicNumber field */
#define BOOT_PROTOCOL_EEPROM         0x00  /**< Boot from the eeprom has been selected. */
#define FLAGS_INVALIDATE_FIRMWARE    0x01  /**< When Set, force the bootloader to treat the firmware as invalid. */
#define GPIO_PULLS_DISABLED          0x00  /**< GPIOx Pull Selection value when pulls are disabled */
#define GPIO_PULL_DOWN_EN            0x01  /**< GPIOx Pull Selection value when pull downs are enabled*/
#define GPIO_PULL_UP_EN              0x02  /**< GPIOx Pull Selection value when pull ups are enabled */
#define GPIO_FIRMWARE_PULL_SELECTION 0x03  /**< GPIOx Pull Selection when firmware defaults rule */


/**
 * \brief   Configuration structure located at the end of a given firmware image.
 *
 * The configuration structure contains information used to configure non-sensor specific settings.
 *
 */
typedef struct {
   /* Bootlaoder / Structure Information */
   u16 MagicNumber;                                               /**< Config data magic number: 0xC82B */
   u16 Flags;                                                     /**< Configuration Flags, Bit 0 = Invalidate firmware, other = RFU. */
   u32 RFU0;                                                      /**< Reserved */
   u16 RamVersion;                                                /**< Ram version of the firmware image. */
   u8 Version;                                                    /**< Current version: 1 */
   u8 BootProtocol;                                               /**< Protocol used for firmware loading. */

   /* 7180 GPIO Configuration */
   union
   {
      u64 PinSelection;                                           /**< Direct access to all of the PinSelection bits. */
      struct
      {
         u64 HostIRQ:4;                                           /**< The GPIO pin used for the Host Interrupt. A value > 6 corrisponds to no pin being selected. */
         u64 RFU:60;                                              /**< Reserved for future use */
      } bits;
   } PinSelection;                                                   /**< Union to provide access to either the full 64 bits or just the HostIRQ */

   union
   {
      u64 PullSelection;                                          /**< Direct access to all of the conifugration data bits. */
      struct
      {
         u64 GPIO0PullSelection:2;                                /**< Pull selection fro GPIO0. 0 = no pulls, 1 = pull down, 2 = pull up, 3 = use firmware defaults **/
         u64 GPIO1PullSelection:2;                                /**< Pull selection fro GPIO1. 0 = no pulls, 1 = pull down, 2 = pull up, 3 = use firmware defaults **/
         u64 GPIO2PullSelection:2;                                /**< Pull selection fro GPIO2. 0 = no pulls, 1 = pull down, 2 = pull up, 3 = use firmware defaults **/
         u64 GPIO3PullSelection:2;                                /**< Pull selection fro GPIO3. 0 = no pulls, 1 = pull down, 2 = pull up, 3 = use firmware defaults **/
         u64 GPIO4PullSelection:2;                                /**< Pull selection fro GPIO4. 0 = no pulls, 1 = pull down, 2 = pull up, 3 = use firmware defaults **/
         u64 GPIO5PullSelection:2;                                /**< Pull selection fro GPIO5. 0 = no pulls, 1 = pull down, 2 = pull up, 3 = use firmware defaults **/
         u64 GPIO6PullSelection:2;                                /**< Pull selection fro GPIO6. 0 = no pulls, 1 = pull down, 2 = pull up, 3 = use firmware defaults **/
         u64 RFU:50;
      } bits;
   } PullSelection;                                                  /**< Union to provide access to the full 64 bits or each GPIO's pull up/down control bits */

   char DeviceName[16];                                              /**< String associated with the device type. */
} SentralConfig;


#endif /* SENTRAL_CONFIG_H */

