 /******************************************************************************
  * @file    mcp3425.c
  * @author  MCD Application Team
  * @version V1.1.4
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
#include "mcp3425.h"

/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;

/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 32 MHz */
//#define I2C_TIMING    0x10A13E56 /* 100 kHz with analog Filter ON, Rise Time 400ns, Fall Time 100ns */ 
#define I2C_TIMING      0x00B1112E /* 400 kHz with analog Filter ON, Rise Time 250ns, Fall Time 100ns */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t mcp3425_write=0x00,mcp3425_read=0x00;
static uint16_t ADC_Average(uint16_t adc_nums[]);

void  BSP_mcp3425_Init( void )
{
  /*##-1- Configure the I2C peripheral ######################################*/
  I2cHandle.Instance              = I2Cx;
  I2cHandle.Init.Timing           = I2C_TIMING;
  I2cHandle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  I2cHandle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;  
  I2cHandle.Init.OwnAddress1      = 0xF0;
  I2cHandle.Init.OwnAddress2      = 0xFE;
  
  if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Enable the Analog I2C Filter */
  HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);
	
  /* Enable the Digital I2C Filter */	
	HAL_I2CEx_ConfigDigitalFilter(&I2cHandle, 0);
  
	check_address_numbere();
}

void check_address_numbere(void)
{
	uint8_t address_flags=0;
	uint8_t txdata[1]={0x1B};
	uint8_t mcp3425_address_temp;
	uint8_t mcp3425_device_code=0x0D;
	uint8_t mcp3425_address_bit=0x00;
	
	for(uint8_t j=0;j<8;j++)
	{
		mcp3425_address_bit=j;
		mcp3425_address_temp=(mcp3425_device_code<<4)|(mcp3425_address_bit<<1);
		for(uint8_t i=0;i<5;i++)
		{
			if(HAL_I2C_Master_Transmit(&I2cHandle,mcp3425_address_temp,txdata,1,1000) == HAL_OK)
			{
				mcp3425_write=mcp3425_address_temp;
				mcp3425_read=mcp3425_address_temp | 0x01;
				address_flags=1;
				break;
			}
		}
		if(address_flags==1)
		{
			address_flags=0;
			break;
		}
	}	
}

/**
  * @brief I2C MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - DMA configuration for transmission request by peripheral 
  *           - NVIC configuration for DMA interrupt request enable
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;
  
  /*##-1- Configure the I2C clock source. The clock is derived from the SYSCLK #*/
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2Cx;
  RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2CxCLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

  /*##-2- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  I2Cx_CLK_ENABLE();
  
  /*##-3- Configure peripheral GPIO ##########################################*/  
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

	S_CLK_ENABLE();
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pin   =S0_GPIO_PIN | S1_GPIO_PIN;
  HAL_GPIO_Init(S_GPIO_PORT, &GPIO_InitStruct); 
	
  /* NVIC for I2Cx */
  HAL_NVIC_SetPriority(I2Cx_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(I2Cx_IRQn);
}
/**
  * @brief I2C MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
  /*##-1- Reset peripherals ##################################################*/
  I2Cx_FORCE_RESET();
  I2Cx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure I2C Tx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
  /* Configure I2C Rx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);
}
uint16_t MCP3425_readdata(void)
{
	  uint8_t txdata[1]={0x1B};
		uint16_t rxdatatemp[6];		
		uint8_t rxdata[2];		
		uint16_t AD_code;

		while(HAL_I2C_Master_Transmit(&I2cHandle,mcp3425_write,txdata,1,0xFFFF) != HAL_OK)
		{
      if(HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
      {}
		}			

		HAL_Delay(100);	
		
		for(int i=0;i<6;i++)
		{
			while(HAL_I2C_Master_Receive(&I2cHandle,mcp3425_read,rxdata,2,0xFFFF) != HAL_OK)
			{
				if(HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
				{}
			}				
			rxdatatemp[i]=((rxdata[0]<<8)+rxdata[1]);	
			HAL_Delay(10);
		}	
		
	AD_code=(ADC_Average(rxdatatemp))&0x07FFF;	
	if(AD_code>0x07FFD)
	{
		return 0;
	}
	else
	   return AD_code;
}

static void ADC_Dxpd(uint16_t adc_nums[])
{
	int i, j, temp, isSorted;  
	for(i=0; i<6-1; i++)
	{
		isSorted = 1;  
		for(j=0; j<6-1-i; j++)
		{
			if(adc_nums[j] > adc_nums[j+1])
			{
				temp = adc_nums[j];
				adc_nums[j] = adc_nums[j+1];
				adc_nums[j+1] = temp;
				isSorted = 0; 
			}
		}
		if(isSorted) break;
	}
}

static uint16_t ADC_Average(uint16_t adc_nums[])
{
	uint32_t sum = 0;

	ADC_Dxpd(adc_nums);
	
	for(uint8_t i=1; i<5; i++)
	{
		sum = sum + adc_nums[i];
	}
	
	return sum/4;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
