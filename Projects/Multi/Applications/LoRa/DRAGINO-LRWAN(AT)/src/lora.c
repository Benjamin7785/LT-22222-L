/******************************************************************************
  * @file    lora.c
  * @author  MCD Application Team
  * @version V1.1.4
  * @date    08-January-2018
  * @brief   lora API to drive the lora state Machine
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
#include "timeServer.h"
#include "lora.h"
#include "tiny_sscanf.h"
#include "flash_eraseprogram.h"
#include "version.h"
#include "stdlib.h"
#include "at.h"

bool fdr_flag=0;
bool sync_value;
bool group_mode;
uint8_t group_mode_id;
uint8_t txp_value;
uint8_t preamble_value;
uint8_t tx_spreading_value;
uint8_t rx_spreading_value;
uint8_t bandwidth_value;
uint8_t codingrate_value;
uint32_t tx_signal_freqence;
uint32_t rx_signal_freqence;
uint32_t APP_TX_DUTYCYCLE;
uint8_t intmode1,intmode2;
uint16_t intdelay1,intdelay2;
uint8_t group_id[8];
uint32_t Automatic_join_network[1]={0x11};

static uint8_t config_count=0;
static uint32_t s_config[32]; //store config

extern uint8_t DIonetoDO,DIonetoRO,DItwotoDO,DItwotoRO;
/**
 *  lora Init
 */
void Config_Init (void)
{
	PPRINTF("\n\rDRAGINO LT-22222-L PINGPONG\n\r");
	PPRINTF("Image Version: "AT_VERSION_STRING"\n\r");
	
	if(*(__IO uint32_t *)DATA_EEPROM_BASE==0x00)// !=0,Automatic join network
  {
		fdr_config();
		EEPROM_program(DATA_EEPROM_BASE,Automatic_join_network,1);					
    PPRINTF("Please set the parameters or reset Device to apply change\n\r");	
		fdr_flag=1;		
	}
	else if(*(__IO uint32_t *)DATA_EEPROM_BASE==0x12)
	{
		fdr_config();			
		EEPROM_program(DATA_EEPROM_BASE,Automatic_join_network,1);	
		NVIC_SystemReset();				
	}		
  else
  {
		EEPROM_Read_Config();
	}		
}

void fdr_config(void)
{
  group_id[0]='1';
  group_id[1]='2';
  group_id[2]='3';
  group_id[3]='4';
  group_id[4]='5';
  group_id[5]='6';
  group_id[6]='7';
  group_id[7]='8';	
	
	sync_value=1;
	preamble_value=8;
	txp_value=14;
	codingrate_value=1;
	APP_TX_DUTYCYCLE=600000;
	tx_spreading_value=12;
	rx_spreading_value=12;
	tx_signal_freqence=868700000;
	rx_signal_freqence=869000000;
	
	EEPROM_Store_Config();
	EEPROM_Read_Config();	
}

void EEPROM_Store_Config(void)
{
	s_config[config_count++]=APP_TX_DUTYCYCLE;

	s_config[config_count++]=tx_signal_freqence;

	s_config[config_count++]=rx_signal_freqence;
	
	s_config[config_count++]=(group_id[0]<<24) | (group_id[1]<<16) | (group_id[2]<<8) | group_id[3];
	
	s_config[config_count++]=(group_id[4]<<24) | (group_id[5]<<16) | (group_id[6]<<8) | group_id[7];
	
	s_config[config_count++]=(sync_value<<24) | (group_mode<<16) | (txp_value<<8) | preamble_value;
	
	s_config[config_count++]=(tx_spreading_value<<24) | (bandwidth_value<<16) | (rx_spreading_value<<8) | group_mode_id;
	
	s_config[config_count++]=(intdelay1<<16) | intdelay2;
	
	s_config[config_count++]=(codingrate_value<<16) | (intmode1<<8) | intmode2;

	s_config[config_count++]=(DIonetoDO<<24) | (DIonetoRO<<16) | (DItwotoDO<<8) | DItwotoRO;
	
  EEPROM_program(EEPROM_USER_START_ADDR_CONFIG,s_config,config_count);//store config
	
	config_count=0;
}

void EEPROM_Read_Config(void)
{
	uint32_t star_address=0,r_config[10];
	
	star_address=EEPROM_USER_START_ADDR_CONFIG;
	for(int i=0;i<10;i++)
	{
	  r_config[i]=FLASH_read(star_address);
		star_address+=4;
	}
   	
	APP_TX_DUTYCYCLE=r_config[0];
	
	tx_signal_freqence=r_config[1];
	
	rx_signal_freqence=r_config[2];
	
	group_id[0]=(r_config[3]>>24)&0xFF;
	
	group_id[1]=(r_config[3]>>16)&0xFF;
	
	group_id[2]=(r_config[3]>>8)&0xFF;
	
	group_id[3]=r_config[3]&0xFF;	
	
	group_id[4]=(r_config[4]>>24)&0xFF;
	
	group_id[5]=(r_config[4]>>16)&0xFF;
	
	group_id[6]=(r_config[4]>>8)&0xFF;
	
	group_id[7]=r_config[4]&0xFF;		
	
	sync_value=(r_config[5]>>24)&0xFF;
	
	group_mode=(r_config[5]>>16)&0xFF;
	
	txp_value=(r_config[5]>>8)&0xFF;
	
	preamble_value=r_config[5]&0xFF;	
	
	tx_spreading_value=(r_config[6]>>24)&0xFF;
	
	bandwidth_value=(r_config[6]>>16)&0xFF;

	rx_spreading_value=(r_config[6]>>8)&0xFF;
	
	group_mode_id=r_config[6]&0xFF;
	
	intdelay1=(r_config[7]>>16)&0xFFFF;
	
	intdelay2=r_config[7]&0xFFFF;		

	codingrate_value=(r_config[8]>>16)&0xFF;	
	
	intmode1=(r_config[8]>>8)&0xFF;
	
	intmode2=r_config[8]&0xFF;	
	
	DIonetoDO=(r_config[9]>>24)&0xFF; 
	
	DIonetoRO=(r_config[9]>>16)&0xFF;
	
	DItwotoDO=(r_config[9]>>8)&0xFF;
	
	DItwotoRO=r_config[9]&0xFF;		
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

