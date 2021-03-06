/**
  ******************************************************************************
  * @file    Loader_Src.c
  * @author  MCD Application Team 
  * @brief        
  *      
  @verbatim
  ==============================================================================
                     ##### <Loader_Src.c> features #####
  ==============================================================================
  [..]
    Sample
    (+) Sample
        (++) Samle

                     ##### How to use this <Loader_Src.c> #####
  ==============================================================================
  [..]
    Sample
    (+) Sample
        (++) Samle   

                     ##### <Loader_Src.c> Limitations #####
  ==============================================================================
  [..]
    Sample
    (+) Sample
        (++) Samle   

  @endverbatim 
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "n25q128a.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define QSPI_PAGESIZE  N25Q128A_PAGE_SIZE
#define QSPI_FLASHSIZE N25Q128A_FLASH_SIZE //(16*1024*1024)
#define QSPI_BLOCKSIZE N25Q128A_SECTOR_SIZE

/* Private macro -------------------------------------------------------------*/


/* Public variables ----------------------------------------------------------*/
extern QSPI_HandleTypeDef hqspi;
extern void SystemClock_Config(void);
extern void MX_QUADSPI_Init(void);
extern void MX_GPIO_Init(void);

/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
void *memset(void *buf, int ch, size_t n);

/* Private functions ---------------------------------------------------------*/


/* External functions --------------------------------------------------------*/

/**
 * @brief  The application entry point.
 * @retval int
 */
int Init(uint8_t MemMappedMode)
{
  SystemInit();

  hqspi.Instance = QUADSPI;
  HAL_QSPI_DeInit(&hqspi);
  HAL_DeInit();

  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_QUADSPI_Init();
  N25Q128A_Init();

  return 1;
}

/*******************************************************************************
 Description :
 Write data to the device
 Inputs :
 Address         : Write location
 Size            : Length in bytes
 buffer          : Address where to get the data to write
 outputs :
 "1"             : Operation succeeded
 Info :
 Note : Mandatory for all types except SRAM and PSRAM
 ********************************************************************************/
int Write(uint32_t Address, uint32_t Size, uint8_t *buffer)
{
  uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  uint32_t QSPI_DataNum = 0;

  if (Address >= 0x90000000)
  {
    Address -= 0x90000000;
  }

  Addr = Address % QSPI_PAGESIZE;
  count = QSPI_PAGESIZE - Addr;
  NumOfPage = Size / QSPI_PAGESIZE;
  NumOfSingle = Size % QSPI_PAGESIZE;

  if (Addr == 0) /*!< Address is QSPI_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < QSPI_PAGESIZE */
    {
      QSPI_DataNum = Size;
      if (QSPI_DataNum != 0)
      {
        N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
      }
    }
    else /*!< Size > QSPI_PAGESIZE */
    {
      while (NumOfPage--)
      {
        QSPI_DataNum = QSPI_PAGESIZE;
        N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
        Address += QSPI_PAGESIZE;
        buffer += QSPI_PAGESIZE;

      }

      QSPI_DataNum = NumOfSingle;
      if (QSPI_DataNum != 0)
      {
        N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
      }

    }
  }
  else /*!< Address is not QSPI_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< Size < QSPI_PAGESIZE */
    {
      if (NumOfSingle > count) /*!< (Size + Address) > QSPI_PAGESIZE */
      {
        temp = NumOfSingle - count;
        QSPI_DataNum = count;
        if (QSPI_DataNum != 0)
        {
          N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
        }
        Address += count;
        buffer += count;

        QSPI_DataNum = temp;
        if (QSPI_DataNum != 0)
        {
          N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
        }
      }
      else
      {
        QSPI_DataNum = Size;
        if (QSPI_DataNum != 0)
        {
          N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
        }
      }
    }
    else /*!< Size > QSPI_PAGESIZE */
    {
      Size -= count;
      NumOfPage = Size / QSPI_PAGESIZE;
      NumOfSingle = Size % QSPI_PAGESIZE;

      QSPI_DataNum = count;

      if (QSPI_DataNum != 0)
      {
        N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
      }
      Address += count;
      buffer += count;

      while (NumOfPage--)
      {
        QSPI_DataNum = QSPI_PAGESIZE;

        if (QSPI_DataNum != 0)
        {
          N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
        }
        Address += QSPI_PAGESIZE;
        buffer += QSPI_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        QSPI_DataNum = NumOfSingle;

        if (QSPI_DataNum != 0)
        {
          N25Q128A_QSPI_Write(buffer, Address, QSPI_DataNum);
        }
      }
    }
  }

  return 1;
}

/*******************************************************************************
 Description :
 Erase a full sector in the device
 Inputs :
 SectrorAddress  : Start of sector
 outputs :
 "1" : Operation succeeded
 "0" : Operation failure
 Note : Not Mandatory for SRAM PSRAM and NOR_FLASH
 ********************************************************************************/
int SectorErase(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
  uint32_t BlockAddr;

  if (EraseStartAddress >= 0x90000000)
  {
    EraseStartAddress -= 0x90000000;
  }
  if (EraseEndAddress >= 0x90000000)
  {
    EraseEndAddress -= 0x90000000;
  }

  EraseStartAddress = EraseStartAddress - EraseStartAddress % 0x10000;

  while (EraseEndAddress >= EraseStartAddress)
  {
    BlockAddr = EraseStartAddress & 0x0FFFFFFF;
    N25Q128A_QSPI_Erase_Block(BlockAddr);
    EraseStartAddress += 0x10000;
  }

  return 1;
}

/*******************************************************************************
 Description :
 Erase a full sector in the device
 Inputs :
 SectrorAddress  : Start of sector
 outputs :
 "1" : Operation succeeded
 "0" : Operation failure
 Note : Not Mandatory for SRAM PSRAM and NOR_FLASH
 ********************************************************************************/
int MassErase(void)
{
  uint32_t BlockAddr = 0;

  while (BlockAddr < QSPI_FLASHSIZE)
  {
    N25Q128A_QSPI_Erase_Block(BlockAddr);
    BlockAddr += QSPI_BLOCKSIZE;
  }

  return 1;
}

/*******************************************************************************
 Description :
 Read data from the device
 Inputs :
 Address         : Write location
 Size            : Length in bytes
 buffer          : Address where to get the data to write
 outputs :
 "1"             : Operation succeeded
 "0"             : Operation failure
 Note : Not Mandatory
 ********************************************************************************/
int Read(uint32_t Address, uint32_t Size, uint8_t *Buffer)
{
  int i = 0;

  N25Q128A_QSPI_EnableMemoryMappedMode();

  for (i = 0; i < Size; i++)
  {
    *(uint8_t*)Buffer++ = *(uint8_t*)Address;
    Address++;
  }

  return 1;
}

/**
 * Description :
 * Calculates checksum value of the memory zone
 * Inputs    :
 *      StartAddress  : Flash start address
 *      Size          : Size (in WORD)
 *      InitVal       : Initial CRC value
 * outputs   :
 *     R0             : Checksum value
 * Note: Optional for all types of device
 */
uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal)
{
  uint8_t missalignementAddress = StartAddress % 4;
  uint8_t missalignementSize = Size;
  int cnt;
  uint32_t Val;

  StartAddress -= StartAddress % 4;
  Size += (Size % 4 == 0)?0:4 - (Size % 4);

  for (cnt = 0; cnt < Size; cnt += 4)
  {
    Val = *(uint32_t*)StartAddress;
    if (missalignementAddress)
    {
      switch (missalignementAddress)
      {
        case 1:
          InitVal += (uint8_t)(Val >> 8 & 0xff);
          InitVal += (uint8_t)(Val >> 16 & 0xff);
          InitVal += (uint8_t)(Val >> 24 & 0xff);
          missalignementAddress -= 1;
          break;
        case 2:
          InitVal += (uint8_t)(Val >> 16 & 0xff);
          InitVal += (uint8_t)(Val >> 24 & 0xff);
          missalignementAddress -= 2;
          break;
        case 3:
          InitVal += (uint8_t)(Val >> 24 & 0xff);
          missalignementAddress -= 3;
          break;
      }
    }
    else if ((Size - missalignementSize) % 4 && (Size - cnt) <= 4)
    {
      switch (Size - missalignementSize)
      {
        case 1:
          InitVal += (uint8_t)Val;
          InitVal += (uint8_t)(Val >> 8 & 0xff);
          InitVal += (uint8_t)(Val >> 16 & 0xff);
          missalignementSize -= 1;
          break;
        case 2:
          InitVal += (uint8_t)Val;
          InitVal += (uint8_t)(Val >> 8 & 0xff);
          missalignementSize -= 2;
          break;
        case 3:
          InitVal += (uint8_t)Val;
          missalignementSize -= 3;
          break;
      }
    }
    else
    {
      InitVal += (uint8_t)Val;
      InitVal += (uint8_t)(Val >> 8 & 0xff);
      InitVal += (uint8_t)(Val >> 16 & 0xff);
      InitVal += (uint8_t)(Val >> 24 & 0xff);
    }
    StartAddress += 4;
  }

  return (InitVal);
}

/**
 * Description :
 * Verify flash memory with RAM buffer and calculates checksum value of
 * the programmed memory
 * Inputs    :
 *      FlashAddr     : Flash address
 *      RAMBufferAddr : RAM buffer address
 *      Size          : Size (in WORD)
 *      InitVal       : Initial CRC value
 * outputs   :
 *     R0             : Operation failed (address of failure)
 *     R1             : Checksum value
 * Note: Optional for all types of device
 */
uint64_t Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t missalignement)
{
  uint32_t VerifiedData = 0, InitVal = 0;
  uint64_t checksum;
  Size *= 4;

  N25Q128A_QSPI_EnableMemoryMappedMode();

  checksum = CheckSum((uint32_t)MemoryAddr + (missalignement & 0xF), Size - ((missalignement >> 16) & 0xF), InitVal);
  while (Size > VerifiedData)
  {
    if (*(uint8_t*)MemoryAddr++ != *((uint8_t*)RAMBufferAddr + VerifiedData))
      return ((checksum << 32) + (MemoryAddr + VerifiedData));

    VerifiedData++;
  }

  return (checksum << 32);
}

void* memset(void *buf, int ch, size_t n)
{
  char *p = buf;
  while (n-- > 0)
  {
    *p++ = (char)ch;
  }
  return p;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
