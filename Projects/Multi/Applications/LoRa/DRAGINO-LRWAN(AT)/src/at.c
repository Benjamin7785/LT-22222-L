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
extern TimerEvent_t syncDI1Timer;
extern TimerEvent_t syncDI2Timer; 
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
extern uint16_t inttime1,inttime2;
extern uint16_t DI1toDO1_time,DI1toRO1_time,DI2toDO2_time,DI2toRO2_time;
extern uint8_t DI1toDO1_statu,DI1toRO1_statu,DI2toDO2_statu,DI2toRO2_statu;
extern uint8_t DO1_init,DO2_init,RO1_init,RO2_init;
extern uint8_t group_id[8];
extern uint8_t DIonetoDO,DIonetoRO,DItwotoDO,DItwotoRO;
extern uint32_t uplinkcount;
extern uint32_t downlinkcount;
// OLD CODE - Race condition prone
// extern bool sending_flag;

// NEW CODE - Use atomic radio state management
extern volatile radio_state_t radio_state;
extern bool radio_is_idle(void);

// NEW CODE - Test function declaration
extern void test_run_comprehensive_test(void);

extern uint8_t befor_RODO;
extern bool sleep_status;
extern uint32_t FLASH_USER_COUNT_START_ADDR;
extern uint32_t FLASH_USER_COUNT_END_ADDR;
extern uint32_t Address;
extern bool sync1_begin,sync2_begin;

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
	// OLD CODE - Race condition prone
	// if(sending_flag==1)
	// {
	//     return AT_BUSY_ERROR;
	// }
	
	// NEW CODE - Use atomic radio state check
	if(!radio_is_idle())
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
	
	// OLD CODE - Enforced minimum 6 second interval
	// if(txtimeout<6000)
	// {
	// 	PRINTF("TDC setting must be more than 6S\n\r");
	// 	APP_TX_DUTYCYCLE=6000;
	// 	return AT_PARAM_ERROR;
	// }
	
	// NEW CODE - Allow 0 to disable automatic TX (for DI1/DI2 only mode)
	if(txtimeout > 0 && txtimeout < 6000)
	{
		PRINTF("TDC setting must be 0 (disabled) or >= 6000ms\n\r");
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
	uint16_t tran_time=0;
	
	if (tiny_sscanf(param, "%d,%d,%d",&int_temp, &time, &tran_time) != 3)
  {	
		if (tiny_sscanf(param, "%hhd,%hu",&int_temp, &time) != 2)
		{
			if (tiny_sscanf(param, "%hhd",&int_temp) != 1)
			{
				return AT_PARAM_ERROR;
			}
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
	inttime1=tran_time;
	if(inttime1==0)
	{
		sync1_begin=0;
		TimerStop( &syncDI1Timer );
	}
  GPIO_EXTI_DI1_IoInit(intmode1);	
	
	return AT_OK;	
}

ATEerror_t at_trig1_get(const char *param)
{
	if(inttime1!=0)
	{
		AT_PRINTF("%d,%d,%d\r\n",intmode1,intdelay1,inttime1);	
	}
	else
	{
		AT_PRINTF("%d,%d\r\n",intmode1,intdelay1);			
	}
	
	return AT_OK;		
}

ATEerror_t at_trig2_set(const char *param)
{
	uint8_t int_temp=0;
	uint16_t time=0;
	uint16_t tran_time=0;
	
	if (tiny_sscanf(param, "%d,%d,%d",&int_temp, &time, &tran_time) != 3)
  {		
		if (tiny_sscanf(param, "%hhd,%hu",&int_temp, &time) != 2)
		{
			if (tiny_sscanf(param, "%hhd",&int_temp) != 1)
			{
				return AT_PARAM_ERROR;
			}
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
	inttime2=tran_time;
	if(inttime2==0)
	{
		sync2_begin=0;
		TimerStop( &syncDI2Timer );
	}	
	
	if(sleep_status==0)
	{
		GPIO_EXTI_DI2_IoInit(intmode2);	
	}
	
	return AT_OK;			
}

ATEerror_t at_trig2_get(const char *param)
{
	if(inttime2!=0)
	{
		AT_PRINTF("%d,%d,%d\r\n",intmode2,intdelay2,inttime2);	
	}
	else
	{
		AT_PRINTF("%d,%d\r\n",intmode2,intdelay2);			
	}
	
	return AT_OK;			
}

ATEerror_t at_pinDI1topinDO1_set(const char *param)
{
	uint8_t temp=0;
	uint16_t time_tmp=0;
  uint8_t status_temp=0;

	if (tiny_sscanf(param, "%d,%d,%d", &temp,&time_tmp,&status_temp) != 3)
  {		
		if (tiny_sscanf(param, "%d,%d", &temp,&time_tmp) != 2)
		{	
			if (tiny_sscanf(param, "%d", &temp) != 1)
			{
				return AT_PARAM_ERROR;
			}
		}
	}
	
	if((temp<=3)&&(status_temp<=3))
	{
		DIonetoDO=temp; 
    DI1toDO1_time=time_tmp;		
		DI1toDO1_statu=status_temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_pinDI1topinDO1_get(const char *param)
{
	if(DI1toDO1_time!=0)
	{
		AT_PRINTF("%d,%d,%d\r\n",DIonetoDO,DI1toDO1_time,DI1toDO1_statu);		
	}
	else
	{
		AT_PRINTF("%d\r\n",DIonetoDO);		
	}
	
	return AT_OK;			
}

ATEerror_t at_pinDI1topinRO1_set(const char *param)
{
	uint8_t temp=0;
	uint16_t time_tmp=0;
  uint8_t status_temp=0;

	if (tiny_sscanf(param, "%d,%d,%d", &temp,&time_tmp,&status_temp) != 3)
  {		
		if (tiny_sscanf(param, "%d,%d", &temp,&time_tmp) != 2)
		{	
			if (tiny_sscanf(param, "%d", &temp) != 1)
			{
				return AT_PARAM_ERROR;
			}
		}
	}
	
	if((temp<=3)&&(status_temp<=3))
	{
		DIonetoRO=temp;
		DI1toRO1_time=time_tmp;
		DI1toRO1_statu=status_temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_pinDI1topinRO1_get(const char *param)
{
	if(DI1toRO1_time!=0)
	{
		AT_PRINTF("%d,%d,%d\r\n",DIonetoRO,DI1toRO1_time,DI1toRO1_statu);	
	}
	else
	{	
		AT_PRINTF("%d\r\n",DIonetoRO);	
	}
	
	return AT_OK;			
}

ATEerror_t at_pinDI2topinDO2_set(const char *param)
{
	uint8_t temp=0;
	uint16_t time_tmp=0;
  uint8_t status_temp=0;

	if (tiny_sscanf(param, "%d,%d,%d", &temp,&time_tmp,&status_temp) != 3)
  {		
		if (tiny_sscanf(param, "%d,%d", &temp,&time_tmp) != 2)
		{		
			if (tiny_sscanf(param, "%d", &temp) != 1)
			{
				return AT_PARAM_ERROR;
			}
		}
	}
	
	if((temp<=3)&&(status_temp<=3))
	{
		DItwotoDO=temp;
		DI2toDO2_time=time_tmp;
		DI2toDO2_statu=status_temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;			
}

ATEerror_t at_pinDI2topinDO2_get(const char *param)
{
	if(DI2toDO2_time!=0)
	{
		AT_PRINTF("%d,%d,%d\r\n",DItwotoDO,DI2toDO2_time,DI2toDO2_statu);	
	}
	else
	{	
		AT_PRINTF("%d\r\n",DItwotoDO);		
	}
	
	return AT_OK;			
}

ATEerror_t at_pinDI2topinRO2_set(const char *param)
{
	uint8_t temp=0;
	uint16_t time_tmp=0;
  uint8_t status_temp=0;

	if (tiny_sscanf(param, "%d,%d,%d", &temp,&time_tmp,&status_temp) != 3)
  {	
		if (tiny_sscanf(param, "%d,%d", &temp,&time_tmp) != 2)
		{			
			if (tiny_sscanf(param, "%d", &temp) != 1)
			{
				return AT_PARAM_ERROR;
			}
		}
	}
	
	if((temp<=3)&&(status_temp<=3))
	{
		DItwotoRO=temp;
		DI2toRO2_time=time_tmp;
		DI2toRO2_statu=status_temp;
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;		
}

ATEerror_t at_pinDI2topinRO2_get(const char *param)
{
	if(DI2toRO2_time!=0)
	{
		AT_PRINTF("%d,%d,%d\r\n",DItwotoRO,DI2toRO2_time,DI2toRO2_statu);			
	}
	else
	{	
		AT_PRINTF("%d\r\n",DItwotoRO);			
	}
	
	return AT_OK;			
}

ATEerror_t at_dorosave_set(const char *param)
{
	uint8_t temp=0;
	uint8_t sta1=0,sta2=0,sta3=0,sta4=0;

	if (tiny_sscanf(param, "%d,%d,%d,%d,%d", &temp, &sta1, &sta2, &sta3, &sta4) != 5)
  {	
		if (tiny_sscanf(param, "%d", &temp) != 1)
		{
			return AT_PARAM_ERROR;
		}
	}	
	
	if(temp<=2)
	{
		befor_RODO=temp;
		if(befor_RODO==2)
		{
			DO1_init=sta1;
			DO2_init=sta2;
			RO1_init=sta3;
			RO2_init=sta4;
		}
	}
	else
	{
		return AT_PARAM_ERROR;		
	}
	
	return AT_OK;			
}

ATEerror_t at_dorosave_get(const char *param)
{
	if(befor_RODO==2)
	{
		AT_PRINTF("%d,%d,%d,%d,%d\r\n",befor_RODO,DO1_init,DO2_init,RO1_init,RO2_init);			
	}
	else
	{
		AT_PRINTF("%d\r\n",befor_RODO);		
	}
	
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

ATEerror_t at_di2sleep_set(const char *param)
{
	uint8_t temp=0;
	
	if (tiny_sscanf(param, "%d", &temp) != 1)
  {
    return AT_PARAM_ERROR;
  }
	
	if(temp<=1)
	{
		sleep_status=temp;
		if(sleep_status==0)
		{
			GPIO_EXTI_DI2_IoInit(intmode2); 	
		}
		else
		{
			GPIO_EXTI_DI2_IoInit(2); 			
		}		
	}
	else
	{
		return AT_PARAM_ERROR;			
	}
	
	return AT_OK;		
}

ATEerror_t at_di2sleep_get(const char *param)
{
	PPRINTF("%d\r\n",sleep_status);
		
	return AT_OK;	
}

ATEerror_t at_Send(const char *param)
{
  const char *buf= param;
  unsigned char bufSize= strlen(param);
  unsigned size=0;
  char hex[3];

	// OLD CODE - Race condition prone
	// if(sending_flag==1)
	// {
	//     return AT_BUSY_ERROR;
	// }
	
	// NEW CODE - Use atomic radio state check
	if(!radio_is_idle())
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

void count_clean(void)
{
	uint16_t i=0; 
		
	uint32_t current_address11;
		
	__IO uint32_t data_on_add11 = 0;
		
	current_address11=FLASH_USER_COUNT_START_ADDR;
	data_on_add11=*(__IO uint32_t *)current_address11;
	while(current_address11+(i*16*8)<FLASH_USER_COUNT_END_ADDR) 
	{
		data_on_add11=*(__IO uint32_t *)(current_address11+(i*16*8));
		if(data_on_add11!=0x00)
		{
			FLASH_erase(current_address11+(i*16*8));
		}
		i++;
	}
	Address=FLASH_USER_COUNT_START_ADDR;
}

// NEW CODE - Stop radio command implementation
ATEerror_t at_STOP_run(const char *param)
{
	AT_PRINTF("Stopping radio operations...\r\n");
	radio_force_idle();
	AT_PRINTF("Radio stopped. System idle.\r\n");
	AT_PRINTF("You can now run AT+TEST\r\n");
	
	return AT_OK;
}

// NEW CODE - Preset configuration command
ATEerror_t at_PRESET_run(const char *param)
{
	AT_PRINTF("Loading preset configuration...\r\n");
	
	// Radio parameters
	txp_value = 20;
	sync_value = 1;
	preamble_value = 8;
	
	// TX/RX Frequencies and Spreading Factors
	tx_signal_freqence = 869000000;
	tx_spreading_value = 12;
	rx_signal_freqence = 868700000;
	rx_spreading_value = 12;
	
	// Modulation
	bandwidth_value = 0;
	codingrate_value = 1;
	
	// Timing
	APP_TX_DUTYCYCLE = 0;  // Event-driven mode
	
	// Trigger configuration
	intmode1 = 2;      // Both edges
	intdelay1 = 250;   // 250ms debounce
	inttime1 = 0;
	
	intmode2 = 2;      // Both edges
	intdelay2 = 250;   // 250ms debounce
	inttime2 = 0;
	
	// Group configuration
	group_mode = 0;
	group_mode_id = 0;
	
	// Group ID
	group_id[0] = '1';
	group_id[1] = '2';
	group_id[2] = '3';
	group_id[3] = '4';
	group_id[4] = '5';
	group_id[5] = '6';
	group_id[6] = '7';
	group_id[7] = '8';
	
	// DI to DO/RO mapping
	DI1toDO1_statu = 2;
	DI1toRO1_statu = 2;
	DI2toDO2_statu = 2;
	DI2toRO2_statu = 2;
	
	DI1toDO1_time = 0;
	DI1toRO1_time = 0;
	DI2toDO2_time = 0;
	DI2toRO2_time = 0;
	
	// Output save mode
	befor_RODO = 2;
	DO1_init = 0;
	DO2_init = 0;
	RO1_init = 0;
	RO2_init = 0;
	
	// Sleep mode
	sleep_status = 0;
	
	// Save to EEPROM
	EEPROM_Store_Config();
	
	AT_PRINTF("Preset configuration loaded!\r\n");
	AT_PRINTF("Configuration:\r\n");
	AT_PRINTF("  TX: %lu Hz, SF%d\r\n", tx_signal_freqence, tx_spreading_value);
	AT_PRINTF("  RX: %lu Hz, SF%d\r\n", rx_signal_freqence, rx_spreading_value);
	AT_PRINTF("  Group ID: %c%c%c%c%c%c%c%c\r\n", 
	          group_id[0], group_id[1], group_id[2], group_id[3],
	          group_id[4], group_id[5], group_id[6], group_id[7]);
	AT_PRINTF("  TDC: %lu ms (0=event-driven)\r\n", APP_TX_DUTYCYCLE);
	AT_PRINTF("\r\nReset device (ATZ) to apply.\r\n");
	
	return AT_OK;
}

// NEW CODE - Test command implementation
ATEerror_t at_TEST_run(const char *param)
{
	// Check if radio is idle before running tests
	if(!radio_is_idle())
	{
		AT_PRINTF("Radio busy, cannot run tests\r\n");
		AT_PRINTF("Send AT+STOP first to stop radio operations\r\n");
		return AT_BUSY_ERROR;
	}
	
	AT_PRINTF("Starting comprehensive test suite...\r\n");
	test_run_comprehensive_test();
	
	return AT_OK;
}
