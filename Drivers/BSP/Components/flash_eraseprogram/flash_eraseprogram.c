 /******************************************************************************
  * @file    oil_float.c
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    01-June-2017
  * @brief   manages the sensors on the application
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
  /* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "flash_eraseprogram.h"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t PAGEError = 0;
__IO uint32_t data32 = 0 ;
__IO uint32_t data_on_add = 0;
uint32_t PP[4]; 
uint32_t Address = 0;
uint8_t  count_flag2=0;
uint8_t  count_flag3=0;
uint8_t  count_flag4=0;
uint16_t  count_i=0;
static FLASH_EraseInitTypeDef EraseInitStruct;	
extern uint32_t FLASH_USER_COUNT_START_ADDR;
extern uint32_t FLASH_USER_COUNT_END_ADDR;
/*Variable used for Erase procedure*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
		
void  FLASH_erase(uint32_t page_address)
{
	HAL_FLASH_Unlock();
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = page_address;
  EraseInitStruct.NbPages     = 1;

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
  {
    /*
      Error occurred while page erase.
      User can add here some code to deal with this error.
      PAGEError will contain the faulty page and then to know the code error on this page,
      user can call function 'HAL_FLASH_GetError()'
    */
      /* indicate error in Erase operation */
      PPRINTF("error in Erase operation\n\r");
  }
/* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();
}

void read_bsp_data(uint16_t batteryLevel_mV, uint8_t status)
{	
  if(count_flag4==1)
	{
  PP[0]=(int)(batteryLevel_mV>>8)<<24|(batteryLevel_mV&0xff)<<16|(status&0xff)<<8|(0&0xff);
	PP[1]= 0x00000000;
	PP[2]= 0x00000000;
	PP[3]= 0x00000000;
	count_flag4=0;
	}
  else if(count_flag3==1)
	{
	PP[0]=(int)(batteryLevel_mV>>8)<<24|(batteryLevel_mV&0xff)<<16|(status&0xff)<<8|((flash_consult_read(FLASH_USER_COUNT_START_ADDR+count_i*128))&0xff);
	PP[1]= 0x00000000;
	PP[2]= 0x00000000;
	PP[3]= 0x00000000;
	count_flag3=0;
	count_i=0;
	}
	else if(count_flag2==1)
	{
	PP[0]=(int)(batteryLevel_mV>>8)<<24|(batteryLevel_mV&0xff)<<16|(status&0xff)<<8|((flash_consult_read(FLASH_USER_COUNT_END_ADDR-128)+1)&0xff);
	PP[1]= 0x00000000;
	PP[2]= 0x00000000;
	PP[3]= 0x00000000;
	count_flag2=0;
	}
	else if(count_flag2==0)
	{
  PP[0]=(int)(batteryLevel_mV>>8)<<24|(batteryLevel_mV&0xff)<<16|(status&0xff)<<8;
	PP[1]= 0x00000000;
	PP[2]= 0x00000000;
	PP[3]= 0x00000000;
	}
}

void store_data_counter(uint32_t addr)
{
	int i=0;
	
	HAL_FLASH_Unlock();
	
	
	while (addr < FLASH_USER_COUNT_END_ADDR)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, PP[i]) == HAL_OK)
    {
      addr = addr + 4;
			i++;
			if(i==4)
			{
				i=0;
				break;
			}
		}
    else
    {
      /* Error occurred while writing data in Flash memory.
         User can add here some code to deal with this error */
        PPRINTF("error in Write operation\n\r");
    }
  }
	
	HAL_FLASH_Lock();
}

uint32_t find_addr(uint32_t addr)
{
	uint32_t current_address;
	
	current_address=addr;
	
	data_on_add=*(__IO uint32_t *)current_address;
	
	while(data_on_add!=0x00)
	{
		current_address = current_address+16;
		data_on_add=*(__IO uint32_t *)current_address;
	}
	if(current_address>=FLASH_USER_COUNT_END_ADDR)
	{
		if(flash_consult_read(FLASH_USER_COUNT_START_ADDR)==flash_consult_read(FLASH_USER_COUNT_END_ADDR-128)) 
		{
		if(flash_consult_read(FLASH_USER_COUNT_START_ADDR)==100)
		{
		FLASH_erase(FLASH_USER_COUNT_START_ADDR);
		count_flag4=1;
		current_address=FLASH_USER_COUNT_START_ADDR;
		}
		else                                                  
		{
		FLASH_erase(FLASH_USER_COUNT_START_ADDR);
		count_flag2=1;
		current_address=FLASH_USER_COUNT_START_ADDR;
		}
		}
		else
		{
			while(count_i<299)                                  
			{
				if(flash_consult_read(FLASH_USER_COUNT_START_ADDR+count_i*128)==flash_consult_read(FLASH_USER_COUNT_START_ADDR+(count_i+1)*128)) 
				{
				count_i++;
				}
				else                                           
				{
					FLASH_erase(FLASH_USER_COUNT_START_ADDR+(count_i+1)*128);
					count_flag3=1;
					current_address=FLASH_USER_COUNT_START_ADDR+(count_i+1)*128;
					break;					
				}
			}
		}
	}

//	PPRINTF("find addr is 0x%0x\r\n",current_address);
	
	return current_address;
}

uint8_t flash_consult_read(uint32_t addrs)  
{
	uint8_t i;
	__IO uint32_t data;

	data=*(__IO uint32_t *)addrs;
	i=(data&0xFF);
//	PRINTF("consult =%d\r\n",i);
	return i;
}

uint16_t read_flash_size(void)
{
	uint32_t device_flash_size_temp;
	uint16_t device_flash_size;
	
	device_flash_size_temp=FLASH_read(FLASHSIZE_BASE);
	
	device_flash_size=device_flash_size_temp&0xffff;

//	PPRINTF("FLASH size is %d kb\r\n",device_flash_size);
	
	return device_flash_size;
}

void EEPROM_program(uint32_t add, uint32_t *data, uint8_t count)
{
	uint32_t Address=0;
	int i=0;
	Address = add;
	
	BACKUP_PRIMASK();
	
	DISABLE_IRQ( );
	
	HAL_FLASHEx_DATAEEPROM_Unlock();
	while (i<count)
  {
		if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD,Address,data[i])== HAL_OK)
		{
			Address = Address + 4;
				i++;
		}
		else
		{
			RESTORE_PRIMASK();
      PRINTF("error in EEPROM Write error\r");
		}
  }
	HAL_FLASHEx_DATAEEPROM_Lock();
	RESTORE_PRIMASK();
}

uint32_t FLASH_read(uint32_t Address)
{
	  data32 = *(__IO uint32_t *)Address;
		return data32;
}

void EEPROM_erase_one_address(uint32_t address)
{
	BACKUP_PRIMASK();
	
	DISABLE_IRQ( );
	
	HAL_FLASHEx_DATAEEPROM_Unlock();
	
	if(HAL_FLASHEx_DATAEEPROM_Erase(address)!=HAL_OK)
	{
		RESTORE_PRIMASK();
    /* indicate error in Erase operation */
    PRINTF("error in EEPROM Erase operation\n\r");
	}
	
	HAL_FLASHEx_DATAEEPROM_Lock();
	
	RESTORE_PRIMASK();
}

void EEPROM_erase_lora_config(void)
{
	uint32_t address;
	
	address=EEPROM_USER_START_ADDR_CONFIG;
	
	BACKUP_PRIMASK();
	
	DISABLE_IRQ( );
	
	HAL_FLASHEx_DATAEEPROM_Unlock();
	while(address<EEPROM_USER_END_ADDR_CONFIG)
	{
		if(HAL_FLASHEx_DATAEEPROM_Erase(address)!=HAL_OK)
		{
			RESTORE_PRIMASK();
			/* indicate error in Erase operation */
			PRINTF("error in EEPROM Erase operation\n\r");
		}
		address = address + 4;
  }
	
	HAL_FLASHEx_DATAEEPROM_Lock();
	
	RESTORE_PRIMASK();
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
