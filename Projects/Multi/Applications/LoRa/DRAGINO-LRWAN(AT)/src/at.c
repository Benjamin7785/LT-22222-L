/*******************************************************************************
 * @file    at.c
 * @author  MCD Application Team
 * @version V1.1.4
 * @date    08-January-2018
 * @brief   at command API
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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "at.h"
#include "utilities.h"
#include "lora.h"
#include "radio.h"
#include "vcom.h"
#include "tiny_sscanf.h"
#include "version.h"
#include "hw_msp.h"
#include "flash_eraseprogram.h"
#include "command.h"
#include "gpio_exti.h"
#include "timeServer.h"

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/**
 * @brief Max size of the data that can be received
 */
extern uint8_t testBuff[256];
extern uint16_t testBuffsize;
extern bool test_uplink_status;
extern bool uplink_data_status;
extern bool fdr_flag;
extern bool sync_value;
extern bool group_mode;
extern uint8_t group_mode_id;
extern uint8_t txp_value;
extern uint8_t preamble_value;
extern uint8_t tx_spreading_value;
extern uint8_t rx_spreading_value;
extern uint8_t bandwidth_value;
extern uint8_t codingrate_value;
extern uint32_t tx_signal_freqence;
extern uint32_t rx_signal_freqence;
extern uint32_t APP_TX_DUTYCYCLE;
extern uint8_t intmode1,intmode2;
extern uint16_t intdelay1,intdelay2;
extern uint8_t group_id[8];
extern uint8_t DIonetoDO,DIonetoRO,DItwotoDO,DItwotoRO;
extern uint32_t uplinkcount;
extern uint32_t downlinkcount;
extern bool sending_flag;

extern uint32_t APP_TX_DUTYCYCLE;
extern TimerEvent_t TxTimer;

/* Exported functions ------------------------------------------------------- */

ATEerror_t at_return_ok(const char *param)
{
  return AT_OK;
}

ATEerror_t at_return_error(const char *param)
{
  return AT_ERROR;
}

ATEerror_t at_reset(const char *param)
{
  NVIC_SystemReset();
  return AT_OK;
}

ATEerror_t at_FDR(const char *param)
{
	EEPROM_erase_one_address(DATA_EEPROM_BASE);
	EEPROM_erase_lora_config();
	AT_PRINTF("OK\n\r");
	HAL_Delay(50);
	NVIC_SystemReset();
  return AT_OK;
}

ATEerror_t at_TransmitPower_get(const char *param)
{
	PPRINTF("%d\r\n",txp_value);

  return AT_OK;
}

ATEerror_t at_TransmitPower_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=20)
	{
		txp_value=temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
  return AT_OK;
}

ATEerror_t at_syncword_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=1)
	{
		sync_value=temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}

  return AT_OK;
}

ATEerror_t at_syncword_get(const char *param)
{
	PPRINTF("%d\r\n",sync_value);
		
	return AT_OK;		
}

ATEerror_t at_preamble_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	preamble_value=temp;
	
	return AT_OK;		
}

ATEerror_t at_preamble_get(const char *param)
{
	PPRINTF("%d\r\n",preamble_value);
		
	return AT_OK;		
}

ATEerror_t at_txCHS_set(const char *param)
{
	uint32_t fre;
	if (tiny_sscanf(param, "%lu", &fre) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if((100000000<fre&&fre<999999999)||fre==0)
	{
	  tx_signal_freqence=fre;
	}
	else 
	{
		return AT_PARAM_ERROR;
	}
	
	return AT_OK;
}

ATEerror_t at_txCHS_get(const char *param)
{ 
	PPRINTF("%u\r\n",tx_signal_freqence);
	return AT_OK;
}

ATEerror_t at_txspreading_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if((temp>=7)&&(temp<=12))
	{
		tx_spreading_value=temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;	
}
	
ATEerror_t at_txspreading_get(const char *param)
{
	PPRINTF("%d\r\n",tx_spreading_value);	
		
	return AT_OK;		
}

ATEerror_t at_rxCHS_set(const char *param)
{
	uint32_t fre;
	if (tiny_sscanf(param, "%lu", &fre) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if((100000000<fre&&fre<999999999)||fre==0)
	{
	  rx_signal_freqence=fre;
		PPRINTF("Attention:Take effect after ATZ\r\n");	
	}
	else 
	{
		return AT_PARAM_ERROR;
	}
	
	return AT_OK;
}

ATEerror_t at_rxCHS_get(const char *param)
{ 
	PPRINTF("%u\r\n",rx_signal_freqence);
	return AT_OK;
}

ATEerror_t at_rxspreading_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if((temp>=7)&&(temp<=12))
	{
		rx_spreading_value=temp;
  	PPRINTF("Attention:Take effect after ATZ\r\n");	
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;	
}
	
ATEerror_t at_rxspreading_get(const char *param)
{
	PPRINTF("%d\r\n",rx_spreading_value);	
		
	return AT_OK;		
}
	
ATEerror_t at_bandwidth_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=2)
	{
		bandwidth_value=temp;
  	PPRINTF("Attention:Take effect after ATZ\r\n");	
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;	
}

ATEerror_t at_bandwidth_get(const char *param)
{
	PPRINTF("%d\r\n",bandwidth_value);
	
	return AT_OK;			
}

ATEerror_t at_codingrate_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if((temp>=1)&&(temp<=4))
	{
		codingrate_value=temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_codingrate_get(const char *param)
{
	PPRINTF("%d\r\n",codingrate_value);		
		
	return AT_OK;		
}

ATEerror_t at_UplinkCounter_get(const char *param)
{
  PPRINTF("%u\r\n",uplinkcount);

  return AT_OK;
}

ATEerror_t at_UplinkCounter_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	uplinkcount=temp;	

  return AT_OK;
}

ATEerror_t at_DownlinkCounter_get(const char *param)
{
  PPRINTF("%u\r\n",downlinkcount);

  return AT_OK;
}

ATEerror_t at_DownlinkCounter_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	downlinkcount=temp;	

  return AT_OK;
}

ATEerror_t at_version_get(const char *param)
{
  AT_PRINTF(AT_VERSION_STRING);
	
  return AT_OK;
}

ATEerror_t at_CFG_run(const char *param)
{
	if(sending_flag==1)
	{
		return AT_BUSY_ERROR;
	}
	
	PPRINTF("\n\rStop Tx events,Please wait for all configurations to print\r\n");	
	PPRINTF("\r\n");
	
	TimerStop(&TxTimer);	
	
	printf_all_config();

	if(fdr_flag==0)
	{	
		PPRINTF("\n\rStart Tx events\r\n");
		TimerStart(&TxTimer);	
	}
	
	return AT_OK;	
}

ATEerror_t at_TDC_set(const char *param)
{ 
	uint32_t txtimeout;
	
	if (tiny_sscanf(param, "%lu", &txtimeout) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(txtimeout<6000)
	{
		PRINTF("TDC setting must be more than 6S\n\r");
		APP_TX_DUTYCYCLE=6000;
		return AT_PARAM_ERROR;
	}
	
	APP_TX_DUTYCYCLE=txtimeout;
	
	return AT_OK;
}

ATEerror_t at_TDC_get(const char *param)
{ 
	PPRINTF("%u\r\n",APP_TX_DUTYCYCLE);
	return AT_OK;
}

ATEerror_t at_trig1_set(const char *param)
{
	uint8_t int_temp=0;
	uint16_t time=0;
	if (tiny_sscanf(param, "%hhd,%hu",&int_temp, &time) != 2)
  {
		if (tiny_sscanf(param, "%hhd",&int_temp) != 1)
		{
			return AT_PARAM_ERROR;
		}
  }
	
  if(int_temp>2)
	{
	 PPRINTF("Range of intmode is 0 to 2\r\n");
   return AT_PARAM_ERROR;
	}
	
  if(time>5000)
	{
	 PPRINTF("Range of intdelay is 0 to 5000\r\n");
   return AT_PARAM_ERROR;		
	}

	intmode1=int_temp;
	intdelay1=time;
  GPIO_EXTI_DI1_IoInit(intmode1);	
	
	return AT_OK;	
}

ATEerror_t at_trig1_get(const char *param)
{
	AT_PRINTF("%d,%u\r\n",intmode1,intdelay1);	
	
	return AT_OK;		
}

ATEerror_t at_trig2_set(const char *param)
{
	uint8_t int_temp=0;
	uint16_t time=0;
	
	if (tiny_sscanf(param, "%hhd,%hu",&int_temp, &time) != 2)
  {
		if (tiny_sscanf(param, "%hhd",&int_temp) != 1)
		{
			return AT_PARAM_ERROR;
		}
  }
	
  if(int_temp>2)
	{
	 PPRINTF("Range of intmode is 0 to 2\r\n");
   return AT_PARAM_ERROR;
	}
	
  if(time>5000)
	{
	 PPRINTF("Range of intdelay is 0 to 5000\r\n");
   return AT_PARAM_ERROR;		
	}
		
	intmode2=int_temp;
	intdelay2=time;
	GPIO_EXTI_DI2_IoInit(intmode2);	
		
	return AT_OK;			
}

ATEerror_t at_trig2_get(const char *param)
{
	AT_PRINTF("%d,%u\r\n",intmode2,intdelay2);			
	
	return AT_OK;			
}

ATEerror_t at_pinDI1topinDO1_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=3)
	{
		DIonetoDO=temp;  
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_pinDI1topinDO1_get(const char *param)
{
	AT_PRINTF("%d\r\n",DIonetoDO);		
	
	return AT_OK;			
}

ATEerror_t at_pinDI1topinRO1_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=3)
	{
		DIonetoRO=temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_pinDI1topinRO1_get(const char *param)
{
	AT_PRINTF("%d\r\n",DIonetoRO);	
	
	return AT_OK;			
}

ATEerror_t at_pinDI2topinDO2_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=3)
	{
		DItwotoDO=temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;			
}

ATEerror_t at_pinDI2topinDO2_get(const char *param)
{
	AT_PRINTF("%d\r\n",DItwotoDO);		
	
	return AT_OK;			
}

ATEerror_t at_pinDI2topinRO2_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=3)
	{
		DItwotoRO=temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_pinDI2topinRO2_get(const char *param)
{
	AT_PRINTF("%d\r\n",DItwotoRO);			
	
	return AT_OK;			
}

ATEerror_t at_groupmode_set(const char *param)
{
	uint8_t temp=0,temp_id=0;

	if (tiny_sscanf(param, "%d,%d", &temp, &temp_id) != 2)
  {	
		if (tiny_sscanf(param, "%d", &temp) != 1)
		{
			return AT_PARAM_ERROR;
		}
	}
	
	if((temp<=1)&&(temp_id<=8))
	{
		group_mode=temp;
		group_mode_id=temp_id;
	}
	else
	{
		PPRINTF("Value1 must be less than or equal to 1, and Value2 must be less than or equal to 8\r\n");
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_groupmode_get(const char *param)
{
	AT_PRINTF("%d,%d\r\n",group_mode,group_mode_id);	
	
	return AT_OK;			
}

ATEerror_t at_groupid_set(const char *param)
{
	if(strlen(param)!=8)
	{
		PPRINTF("Group ID length must be 8 characters\r\n");
		return AT_PARAM_ERROR;
	}
	else
	{
		for(uint8_t j=0;j<8;j++)
		{
			group_id[j]=0x00;
		}
		for(uint8_t i=0;i<strlen(param);i++)
		{
			group_id[i]=param[i];
		}
	}
	
	return AT_OK;	
}

ATEerror_t at_groupid_get(const char *param)
{
  for(uint8_t i=0;i<8;i++)
	{
		if(group_id[i]!='\0')
		{
			PPRINTF("%c",group_id[i]);
		}
	}	
	PPRINTF("\r\n");
	
	return AT_OK;		
}

ATEerror_t at_Send(const char *param)
{
  const char *buf= param;
  unsigned char bufSize= strlen(param);
  unsigned size=0;
  char hex[3];

	if(sending_flag==1)
	{
		return AT_BUSY_ERROR;
	}
	
  hex[2] = 0;
  while ((size < 255) && (bufSize > 1))
  {
    hex[0] = buf[size*2];
    hex[1] = buf[size*2+1];
    if (tiny_sscanf(hex, "%hhx", &testBuff[size]) != 1)
    {
      return AT_PARAM_ERROR;
    }
    size++;
    bufSize -= 2;
  }
	
  if (bufSize != 0)
  {
    return AT_PARAM_ERROR;
  }
	
	testBuffsize=size;
	uplink_data_status=1;
  test_uplink_status=1;
	
  return AT_OK;
}
