/*****************************************************************************
*  Copyright 2010 Broadcom Corporation.  All rights reserved.
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

#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include "type_def.h"
#include "aes.h"



const uint8 subBytesTable[16][16] = {
   { 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76 },
   { 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0 },
   { 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15 },
   { 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75 },
   { 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84 },
   { 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf },
   { 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8 },
   { 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2 },
   { 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73 },
   { 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb },
   { 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79 },
   { 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08 },
   { 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a },
   { 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e },
   { 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf },
   { 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }
};


const uint8 invSubBytesTable[16][16] = {
   { 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb },
   { 0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb },
   { 0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e },
   { 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25 },
   { 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92 },
   { 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84 },
   { 0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06 },
   { 0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b },
   { 0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73 },
   { 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e },
   { 0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b },
   { 0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4 },
   { 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f },
   { 0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef },
   { 0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61 },
   { 0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d }
};



uint32 aesErrorCount = 0;



void InitStateArray(const uint8 *inputData, uint8 array[4][4])
{
   uint16 x, y;

   for (x=0; x<4; x++)
   {
      for (y=0; y<4; y++)
      {
         array[x][y] = inputData[x+4*y];
      }
   }
}


uint8 SubByte(uint8 input)
{
   uint16 x, y;

   x = input >> 4;
   y = input & 0x0F;

   return(subBytesTable[x][y]);
}


void SubBytes(uint8 data[4][4])
{
   uint16 x, y;

   for (x=0; x<4; x++)
   {
      for (y=0; y<4; y++)
      {
         data[x][y] = SubByte(data[x][y]);
      }
   }
}


uint8 InvSubByte(uint8 input)
{
   uint16 x, y;

   x = input >> 4;
   y = input & 0x0F;

   return(invSubBytesTable[x][y]);
}


void InvSubBytes(uint8 data[4][4])
{
   uint16 x, y;

   for (x=0; x<4; x++)
   {
      for (y=0; y<4; y++)
      {
         data[x][y] = InvSubByte(data[x][y]);
      }
   }
}


void ShiftRows(uint8 data[4][4])
{
   uint8 temp;
   uint16 y;


   /* Shift row 1 */
   temp = data[1][0];
   for (y=0; y<3; y++)
   {
      data[1][y] = data[1][y+1];
   }
   data[1][3] = temp;

   /* Shift row 2 */
   temp = data[2][0];
   data[2][0] = data[2][2];
   data[2][2] = temp;
   temp = data[2][1];
   data[2][1] = data[2][3];
   data[2][3] = temp;

   /* Shift row 3 */
   temp = data[3][3];
   for (y=3; y>0; y--)
   {
      data[3][y] = data[3][y-1];
   }
   data[3][0] = temp;
}


void InvShiftRows(uint8 data[4][4])
{
   uint8 temp;
   uint16 y;


   /* Shift row 1 */
   temp = data[1][3];
   for (y=3; y>0; y--)
   {
      data[1][y] = data[1][y-1];
   }
   data[1][0] = temp;

   /* Shift row 2 */
   temp = data[2][0];
   data[2][0] = data[2][2];
   data[2][2] = temp;
   temp = data[2][1];
   data[2][1] = data[2][3];
   data[2][3] = temp;

   /* Shift row 3 */
   temp = data[3][0];
   for (y=0; y<3; y++)
   {
      data[3][y] = data[3][y+1];
   }
   data[3][3] = temp;
}


uint8 ModMult(uint8 input, uint8 factor)
{
   uint8 tempResults[4];
   uint8 result = 0;
   uint8 calcFactor = 0;
   uint16 x;
   uint16 temp;


   if (factor > 15)
   {
      /* This point should never be reached */
//      printf("MULTIPLY ERROR %x\n", factor);
      aesErrorCount++;
      return(0);
   }

   /* Loop through each multiplication bit */
   for (x=0; x<4; x++)
   {
      if (x == 0)
      {
         tempResults[x] = input;
      }
      else
      {
         /* Mulitply the previous value by 2 and take modulus */
         temp = 2 * tempResults[x-1];
         if (temp >= 0x100)
         {
            temp = temp ^ 0x11b;
         }
         tempResults[x] = temp;
      }

      /* Add current value if needed */
      if (factor & (1 << x))
      {
         result ^= tempResults[x];
         calcFactor |= (1 << x);
      }

      if (calcFactor >= factor)
      {
         break;
      }
   }

   return(result);
}


void MixColumns(uint8 data[4][4])
{
   uint16 y;
   uint8 newOutput[4][4];


   for (y=0; y<4; y++)
   {
      newOutput[0][y] = ModMult(data[0][y],2) ^ ModMult(data[1][y],3) ^ data[2][y] ^ data[3][y];
      newOutput[1][y] = data[0][y] ^ ModMult(data[1][y],2) ^ ModMult(data[2][y],3) ^ data[3][y];
      newOutput[2][y] = data[0][y] ^ data[1][y] ^ ModMult(data[2][y],2) ^ ModMult(data[3][y],3);
      newOutput[3][y] = ModMult(data[0][y],3) ^ data[1][y] ^ data[2][y] ^ ModMult(data[3][y],2);
   }

   memcpy(data, newOutput, 16);
}


void InvMixColumns(uint8 data[4][4])
{
   uint16 y;
   uint8 newOutput[4][4];


   for (y=0; y<4; y++)
   {
      newOutput[0][y] = ModMult(data[0][y],0xe) ^ ModMult(data[1][y],0xb) ^ ModMult(data[2][y],0xd) ^ ModMult(data[3][y],0x9);
      newOutput[1][y] = ModMult(data[0][y],0x9) ^ ModMult(data[1][y],0xe) ^ ModMult(data[2][y],0xb) ^ ModMult(data[3][y],0xd);
      newOutput[2][y] = ModMult(data[0][y],0xd) ^ ModMult(data[1][y],0x9) ^ ModMult(data[2][y],0xe) ^ ModMult(data[3][y],0xb);
      newOutput[3][y] = ModMult(data[0][y],0xb) ^ ModMult(data[1][y],0xd) ^ ModMult(data[2][y],0x9) ^ ModMult(data[3][y],0xe);
   }

   memcpy(data, newOutput, 16);
}


void AddRoundKey(uint8 data[4][4], const uint32 *roundKey)
{
   uint16 x, y;
   uint32 roundKeyMatrix[4][4];


   for (y=0; y<4; y++)
   {
      roundKeyMatrix[0][y] = (roundKey[y] >> 24) & 0xFF;
      roundKeyMatrix[1][y] = (roundKey[y] >> 16) & 0xFF;
      roundKeyMatrix[2][y] = (roundKey[y] >> 8) & 0xFF;
      roundKeyMatrix[3][y] = (roundKey[y]) & 0xFF;
   }

   for (x=0; x<4; x++)
   {
      for (y=0; y<4; y++)
      {
         data[x][y] ^= roundKeyMatrix[x][y];
      }
   }
}


uint32 RotWord(uint32 input)
{
   uint32 temp;

   temp = input >> 24;
   input <<= 8;
   input |= temp;

   return(input);
}


void KeyExpansion(const uint8 *key, uint32 *w, uint16 aesKeySize)
{
   uint8 rconByte = 1;
   uint16 i;
   uint32 rconWord;
   uint32 temp;
   uint16 Nk, Nr;


   switch (aesKeySize)
   {
      case AES_KEY_SIZE_128:
         Nk = 4;
         Nr = 10;
         break;
      case AES_KEY_SIZE_192:
         Nk = 6;
         Nr = 12;
         break;
      case AES_KEY_SIZE_256:
         Nk = 8;
         Nr = 14;
         break;
      default:
//         printf("ERROR: AES key size\n");
         aesErrorCount++;
         return;
   }

   i = 0;
   while (i < Nk)
   {
      w[i] = key[4*i]<<24 | key[4*i+1]<<16 | key[4*i+2]<<8 | key[4*i+3];
      i++;
   }
   i = Nk;

   while (i < 4*(Nr+1))
   {
      temp = w[i-1];
      if ( (i % Nk) == 0)
      {
         temp = RotWord(temp);
         temp = (SubByte(temp & 0xFF)) | (SubByte((temp>>8) & 0xFF) << 8) |
                (SubByte((temp>>16) & 0xFF) << 16) |
                (SubByte((temp>>24) & 0xFF) << 24);

         if (i > Nk)
         {
            rconByte = ModMult(rconByte, 2);
         }
         rconWord = rconByte << 24;

         temp = temp ^ rconWord;
      }
      else if ( (Nk > 6) && ((i % Nk) == 4) )
      {
         temp = (SubByte(temp & 0xFF)) | (SubByte((temp>>8) & 0xFF) << 8) |
                (SubByte((temp>>16) & 0xFF) << 16) |
                (SubByte((temp>>24) & 0xFF) << 24);
      }

      w[i] = w[i-Nk] ^ temp;
      i++;
   }
}


void Cipher(const uint8 *input, uint8 *output, uint32 *roundKey,
            uint16 aesKeySize)
{
   uint8 stateArray[4][4];
   uint16 x, y;
   uint16 round;
   uint16 Nr;


   switch (aesKeySize)
   {
      case AES_KEY_SIZE_128:
         Nr = 10;
         break;
      case AES_KEY_SIZE_192:
         Nr = 12;
         break;
      case AES_KEY_SIZE_256:
         Nr = 14;
         break;
      default:
//         printf("ERROR: AES key size\n");
         aesErrorCount++;
         return;
   }

   InitStateArray(input, stateArray);

   AddRoundKey(stateArray, (const uint32 *) roundKey);

   for (round = 1; round<Nr; round++)
   {
      SubBytes(stateArray);
      ShiftRows(stateArray);
      MixColumns(stateArray);
      AddRoundKey(stateArray, (const uint32 *) roundKey+4*round);
   }

   SubBytes(stateArray);
   ShiftRows(stateArray);
   AddRoundKey(stateArray, (const uint32 *) roundKey+4*Nr);


   for (x=0; x<4; x++)
   {
      for (y=0; y<4; y++)
      {
         output[x+4*y] = stateArray[x][y];
      }
   }
}


void InvCipher(const uint8 *input, uint8 *output, uint32 *roundKey,
               uint16 aesKeySize)
{
   uint8 stateArray[4][4];
   uint16 x, y;
   uint16 round;
   uint16 Nr;


   switch (aesKeySize)
   {
      case AES_KEY_SIZE_128:
         Nr = 10;
         break;
      case AES_KEY_SIZE_192:
         Nr = 12;
         break;
      case AES_KEY_SIZE_256:
         Nr = 14;
         break;
      default:
//         printf("ERROR: AES key size\n");
         aesErrorCount++;
         return;
   }

   InitStateArray(input, stateArray);

   AddRoundKey(stateArray, (const uint32 *) roundKey+4*Nr);

   for (round = Nr-1; round>0; round--)
   {
      InvShiftRows(stateArray);
      InvSubBytes(stateArray);
      AddRoundKey(stateArray, (const uint32 *) roundKey+4*round);
      InvMixColumns(stateArray);
   }

   InvShiftRows(stateArray);
   InvSubBytes(stateArray);
   AddRoundKey(stateArray, (const uint32 *) roundKey);


   for (x=0; x<4; x++)
   {
      for (y=0; y<4; y++)
      {
         output[x+4*y] = stateArray[x][y];
      }
   }
}


uint32 AesEcb(uint8 *output, const uint8 *input, uint32 dataLen,
              const uint8 *key, uint32 keySize, uint32 operation)
{
   uint32 testRoundKey[61];
   unsigned char testOutput[AES_BLOCK_SIZE];
   int dataIndex = 0;
   int x;


   /* Check for invalid parameters */
   if ( (output == NULL) || (key == NULL) ||
        ( (keySize != AES_KEY_SIZE_128) && (keySize != AES_KEY_SIZE_192) &&
          (keySize != AES_KEY_SIZE_256) ) ||
        ( (operation != AES_OPERATION_ENCRYPT) &&
          (operation != AES_OPERATION_DECRYPT) ) ||
        ( (dataLen % AES_BLOCK_SIZE) != 0) )
   {
      return(0);
   }

   /* Do the key expansion */
   KeyExpansion(key, testRoundKey, keySize);

   /* Loop through complete AES blocks */
   while (dataLen >= AES_BLOCK_SIZE)
   {
      if (operation == AES_OPERATION_ENCRYPT)
      {
         /* Do the encryption */
         Cipher(input + dataIndex, testOutput, testRoundKey, keySize);
      }
      else
      {
         /* Do the decryption */
         InvCipher(input + dataIndex, testOutput, testRoundKey, keySize);
      }

      /* Copy the result to the output */
      for (x=0; x<AES_BLOCK_SIZE; x++)
      {
         output[dataIndex + x] = testOutput[x];
      }
      
      /* Update data counters */
      dataIndex += AES_BLOCK_SIZE;
      dataLen -= AES_BLOCK_SIZE;
   }

   return(1);
}


uint32 AesCbc(uint8 *output, const uint8 *input, uint32 dataLen,
              const uint8 *key, uint32 keySize, const uint8 *startIv,
              uint32 operation)
{
   uint32 testRoundKey[61];
   unsigned char data[AES_BLOCK_SIZE];
   unsigned char iv[AES_BLOCK_SIZE];
   unsigned char testOutput[AES_BLOCK_SIZE];
   int dataIndex = 0;
   int x;


   /* Check for invalid parameters */
   if ( (output == NULL) || (key == NULL) || (startIv == NULL) ||
        ( (keySize != AES_KEY_SIZE_128) && (keySize != AES_KEY_SIZE_192) &&
          (keySize != AES_KEY_SIZE_256) ) ||
        ( (operation != AES_OPERATION_ENCRYPT) &&
          (operation != AES_OPERATION_DECRYPT) ) ||
        ( (dataLen % AES_BLOCK_SIZE) != 0) )
   {
      return(0);
   }

   /* Copy the beginning IV */
   for (x=0; x<AES_BLOCK_SIZE; x++)
   {
      iv[x] = startIv[x];
   }

   /* Do the key expansion */
   KeyExpansion(key, testRoundKey, keySize);

   /* Loop through complete AES blocks */
   while (dataLen >= AES_BLOCK_SIZE)
   {
      /* Do the encryption/decryption */
      if (operation == AES_OPERATION_ENCRYPT)
      {
         /* Set the input date */
         for (x=0; x<AES_BLOCK_SIZE; x++)
         {
            data[x] = input[dataIndex + x] ^ iv[x];
         }

         /* Do the encryption */
         Cipher(data, testOutput, testRoundKey, keySize);

         /* Copy the result to the output and update the IV */
         for (x=0; x<AES_BLOCK_SIZE; x++)
         {
            output[dataIndex + x] = testOutput[x];
            iv[x] = testOutput[x];
         }
      }
      else
      {
         /* Set the input date */
         for (x=0; x<AES_BLOCK_SIZE; x++)
         {
            data[x] = input[dataIndex + x];
         }

         /* Do the decryption */
         InvCipher(data, testOutput, testRoundKey, keySize);

         /* Copy the result to the output and update the IV */
         for (x=0; x<AES_BLOCK_SIZE; x++)
         {
            output[dataIndex + x] = testOutput[x] ^ iv[x];
            iv[x] = data[x];
         }
      }
      
      /* Update data counters */
      dataIndex += AES_BLOCK_SIZE;
      dataLen -= AES_BLOCK_SIZE;
   }

   return(1);
}


uint32 Aes(uint8 *outDataPtr, const uint8 *inDataPtr, uint32 inDataSize,
           const uint8 *inKeyPtr, uint32 inKeySize, const uint8 *inIvPtr,
           uint32 inMode, uint32 inOperation)
{
   if (inMode == AES_MODE_ECB)
   {
      return(AesEcb(outDataPtr, inDataPtr, inDataSize, inKeyPtr, inKeySize,
                    inOperation));
   }
   else if (inMode == AES_MODE_CBC)
   {
      return(AesCbc(outDataPtr, inDataPtr, inDataSize, inKeyPtr, inKeySize,
                    inIvPtr, inOperation));
   }
   else
   {
      /* Invalid AES mode */
      return(0);
   }
}
