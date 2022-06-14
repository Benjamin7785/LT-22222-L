/******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.4
  * @date    08-January-2018
  * @brief   this is the main!
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
#include "low_power_manager.h"
#include "lora.h"
#include "timeServer.h"
#include "vcom.h"
#include "flash_eraseprogram.h"
#include "delay.h"
#include "iwdg.h"
#include "command.h"
#include "at.h"
#include "gpio_exti.h"
#include "npn_output.h"
#include "relay_output.h"
#include "radio.h"	
#include "sx1276.h"

typedef struct
{
  /*point to the LoRa App data buffer*/
  uint8_t* Buff;
  /*LoRa App data buffer size*/
  uint8_t BuffSize; 
} lora_AppData_t;

static lora_AppData_t AppData;
bool data_check_flag=0;
bool sending_flag=0;
bool test_uplink_status=0;
bool uplink_data_status=0;
bool rx_waiting_flag=0;
bool is_time_to_IWDG_Refresh=0;
bool lora_wait_flags=0;
bool retransmission_flag=0;
uint8_t request_flag=0;
uint8_t accept_flag=0;
uint8_t ack_send_num=0;
uint16_t batteryLevel_mV=3300;
uint8_t DI1_flag=0,DI2_flag=0,DO1_flag=0,DO2_flag=0,RO1_flag=0,RO2_flag=0;
uint8_t DIonetoDO=0,DIonetoRO=0;
uint8_t DItwotoDO=0,DItwotoRO=0;
uint32_t uplinkcount=0;
uint32_t downlinkcount=0;

extern bool sync_value;
extern bool group_mode;
extern uint8_t group_mode_id;
extern bool fdr_flag;
extern uint32_t APP_TX_DUTYCYCLE;
extern uint8_t group_id[8];
extern uint8_t txp_value;
extern uint8_t preamble_value;
extern uint8_t tx_spreading_value;
extern uint8_t rx_spreading_value;
extern uint8_t bandwidth_value;
extern uint8_t codingrate_value;
extern uint32_t tx_signal_freqence;
extern uint32_t rx_signal_freqence;
extern bool exitflag1,exitflag2;
extern uint8_t intmode1,intmode2;

static uint8_t txDataBuff[256];
static uint8_t rxDataBuff[256];
static uint16_t txBufferSize=0;
static uint16_t rxBufferSize=0;
uint8_t testBuff[256];
uint16_t testBuffsize=0;

static RadioEvents_t RadioEvents;
/* LoRa endNode send request*/
static void Send_TX( void );
static void Send_test( void );
static void RxData(lora_AppData_t *AppData);
static void send_exti(void);
static uint32_t crc32(uint8_t *data,uint16_t length);

/* start the tx process*/
static void LoraStartTx(void);
static void AckStartTX(void);
static void StartIWDGRefresh(void);

/*!
 * Specifies the state of the application LED
 */
TimerEvent_t TxTimer; //TDC
TimerEvent_t AckTimer; //TDC
static TimerEvent_t IWDGRefreshTimer;//watch dog

/* tx timer callback function*/
static void OnTxTimerEvent( void );
static void OnAckEvent( void );
static void OnIWDGRefreshTimeoutEvent(void);		
static void lora_test_init(void);
static void test_OnTxDone( void );
static void test_OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
static void test_OnTxTimeout( void );
static void test_OnRxTimeout( void );
static void test_OnRxError( void );

extern void LoraStartdelay1(void);
extern void LoraStartdelay2(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main( void )
{
  /* STM32 HAL library initialization*/
  HAL_Init( );
  
  /* Configure the system clock*/
  SystemClock_Config( );
  
  /* Configure the debug mode*/
  DBG_Init( );
  
  /* Configure the hardware*/
  HW_Init( );
 
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */
  CMD_Init();
	
	Config_Init();
	
	iwdg_init();			
 	StartIWDGRefresh(); 
	
	lora_test_init();

	GPIO_EXTI_DI1_IoInit(intmode1);
	GPIO_EXTI_DI2_IoInit(intmode2); 
	BSP_npn_output_Init();
	BSP_relay_output_Init();	
	LED_IoInit();
	
	if(fdr_flag==0)
	{
		LoraStartdelay1(); 
		LoraStartdelay2(); 
		AckStartTX();
		LoraStartTx();
	}
	
  while( 1 )
  {
		/* Handle UART commands */
    CMD_Process();
		
		send_exti();
		
		if((uplink_data_status==1)&&(sending_flag==0))
		{
			sending_flag=1;
			Radio.SetChannel( tx_signal_freqence );	
			Radio.SetTxConfig( MODEM_LORA, txp_value, 0, bandwidth_value, tx_spreading_value, codingrate_value,preamble_value, false, true, 0, 0, false, 3000 );	
			PPRINTF("\r\n***** UpLinkCounter= %u *****\n\r", uplinkcount++ );
			PPRINTF( "TX on freq %u Hz at SF %d\r\n", tx_signal_freqence, tx_spreading_value );
			if(retransmission_flag==1)
			{	
				uint32_t crc_check;
				txDataBuff[9]= request_flag;
				crc_check=crc32(txDataBuff,txBufferSize-4);
				txDataBuff[txBufferSize-4] = crc_check&0xff;
				txDataBuff[txBufferSize-3] = crc_check>>8&0xff;
				txDataBuff[txBufferSize-2] = crc_check>>16&0xff;
				txDataBuff[txBufferSize-1] = crc_check>>24&0xff;				
				Radio.Send( txDataBuff, txBufferSize );						
			}			
			else if(test_uplink_status==1)		
			{
				Send_test();
				test_uplink_status=0;
			}				
			else
			{
				Send_TX();
			}				
			uplink_data_status=0;
		}
		
		if(rx_waiting_flag==1)
		{
			Radio.SetChannel( rx_signal_freqence );
			Radio.SetRxConfig( MODEM_LORA, bandwidth_value, rx_spreading_value, codingrate_value, 0, preamble_value,5, false,0, true, 0, 0, false, true );	
			PPRINTF( "RX on freq %u Hz at SF %d\r\n", rx_signal_freqence, rx_spreading_value );	
			PPRINTF("rxWaiting\r\n");	
      Radio.Rx(0);	
			sending_flag=0;			
			rx_waiting_flag=0;
		}
		
		if(is_time_to_IWDG_Refresh==1)
		{
			IWDG_Refresh();			
			is_time_to_IWDG_Refresh=0;
		}				
  }
}

static void Send_TX( void )
{		
	uint32_t crc_check;
	uint8_t i = 0;
	
	for(uint8_t j=0;j<8;j++)
	{
		txDataBuff[i++] = group_id[j];
	}	

	if(group_mode==0)
	{
		txDataBuff[i++]= 0x00;
	}
	else
	{
		txDataBuff[i++]= group_mode_id;		
	}
	
	if(((exitflag1==1)||(exitflag2==1))&&(group_mode==0)&&(group_mode_id==0))
	{
	  request_flag=1;
	}
	txDataBuff[i++]= request_flag;	
	txDataBuff[i++]= accept_flag;	
	
	if(accept_flag!=0)
	accept_flag=0;
	
	DI1_flag=HAL_GPIO_ReadPin(GPIO_EXTI_PORT,GPIO_EXTI_PIN);	
	txDataBuff[i++]=1<<4 | DI1_flag;	
	if((exitflag1==1)&&(request_flag!=0))
	{
		txDataBuff[i++]=DIonetoDO;
		txDataBuff[i++]=DIonetoRO;
	}
	else
	{
		txDataBuff[i++]=0x00;
		txDataBuff[i++]=0x00;		
	}
	
	DI2_flag=HAL_GPIO_ReadPin(GPIO_EXTI2_PORT,GPIO_EXTI2_PIN);	
	txDataBuff[i++]=2<<4 | DI2_flag;	
	if((exitflag2==1)&&(request_flag!=0))
	{
		txDataBuff[i++]=DItwotoDO;
		txDataBuff[i++]=DItwotoRO;
	}
	else
	{
		txDataBuff[i++]=0x00;
		txDataBuff[i++]=0x00;				
	}
	
	crc_check=crc32(txDataBuff,i);
	txDataBuff[i++] = crc_check&0xff;
	txDataBuff[i++] = crc_check>>8&0xff;
	txDataBuff[i++] = crc_check>>16&0xff;
	txDataBuff[i++] = crc_check>>24&0xff;
	
  txBufferSize = i;		

	if(exitflag1==1)
		exitflag1=0;
	if(exitflag2==1)
		exitflag2=0;
	
	Radio.Send( txDataBuff, txBufferSize );
}

static void Send_test( void )
{
	uint32_t crc_check;
	uint8_t i = 0;	
	
	for(uint8_t j=0;j<8;j++)
	{
		txDataBuff[i++] = group_id[j];
	}	
	
  for(uint8_t k=0;k<testBuffsize;k++)
	{
		txDataBuff[i++] = testBuff[k];
	}
	
	crc_check=crc32(txDataBuff,i);
	txDataBuff[i++] = crc_check&0xff;
	txDataBuff[i++] = crc_check>>8&0xff;
	txDataBuff[i++] = crc_check>>16&0xff;
	txDataBuff[i++] = crc_check>>24&0xff;
	
  txBufferSize = i;		

	Radio.Send( txDataBuff, txBufferSize );		
}

static void RxData(lora_AppData_t *AppData)
{
  for(int i=0;i<2;i++)
	{
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_11);
		HAL_Delay(300);
	}	
					
	if((AppData->BuffSize == 9)&&(AppData->Buff[2] != 0)) //received ack of Group RX
	{
		uint8_t ack_id=0,request_num=0;
		if((group_mode==0)&&(group_mode_id==0))  //point to point
		{
			request_flag=0;
		}
		else                                    //point to multi-point
		{
			request_flag=request_flag&(~(AppData->Buff[2]));			
			for(uint8_t k=0;k<8;k++)
			{
				if((request_flag&(1<<k))!=0)
				{
					request_num++;
				}
				
				if((AppData->Buff[2]&(1<<k))!=0)
				{
					ack_id=k+1;
				}
			}	
		}
		
		if(lora_wait_flags==1)
		{
			if(AppData->Buff[0]==0x00)
			{
				PPRINTF("Received ACK Group ID, sub-id: 0\r\n");			
			}
			else
			{
				PPRINTF("Received ACK Group ID, sub-id: %d. Expected ACK : %d, Recevied ACK %d\r\n",ack_id,group_mode_id,group_mode_id-request_num);
			}	
		}		
		
		if(request_flag==0)
		{
			TimerStop( &AckTimer );
			ack_send_num=0;
			lora_wait_flags=0;	
			retransmission_flag=0;			
		}
	}
	
	uint8_t rece_temp=0;
	if((group_mode==0)&&(group_mode_id==0))
	{
		rece_temp=AppData->Buff[1] & 0xff;
	}
	else
	{
		rece_temp=AppData->Buff[1] & (1<<(group_mode_id-1));
		if(rece_temp!=0)
			rece_temp=0x01;
		else
		  rece_temp=0x00;
	}
	
	switch(rece_temp)
	{
		case 0x01:             //received maps of Group TX
		{		
			if(!((group_mode==0)&&(group_mode_id!=0)))
			{
				if((AppData->BuffSize == 9)&&((AppData->Buff[3]&0xf0)==0x10)&&((AppData->Buff[6]&0xf0)==0x20))
				{
					if((AppData->Buff[4]!=0x00)||(AppData->Buff[5]!=0x00)||(AppData->Buff[7]!=0x00)||(AppData->Buff[8]!=0x00))
					{
						uint8_t level_status1,level_status2;
						level_status1=AppData->Buff[3]&0x0f;
						level_status2=AppData->Buff[6]&0x0f;	
						
						if(AppData->Buff[4]!=0x00)
						{
							if(AppData->Buff[4]==1)
							{
								if(level_status1==1)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET); 
								else if(level_status1==0)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET); 
							}
							else if(AppData->Buff[4]==2)
							{
								if(level_status1==1)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET); 
								else if(level_status1==0)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET); 
							}
							else if(AppData->Buff[4]==3)
							{
								HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_12);
							}
						}
						if(AppData->Buff[7]!=0x00)
						{
							if(AppData->Buff[7]==1)
							{
								if(level_status2==1)
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET); 
								else if(level_status2==0)
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET); 
							}
							else if(AppData->Buff[7]==2)
							{
								if(level_status2==1)
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET); 
								else if(level_status2==0)
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET); 
							}
							else if(AppData->Buff[7]==3)
							{
								HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_14);
							}					
						}
						if(AppData->Buff[5]!=0x00)
						{
							if(AppData->Buff[5]==1)
							{
								if(level_status1==1)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET); 
								else if(level_status1==0)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET); 
							}
							else if(AppData->Buff[5]==2)
							{
								if(level_status1==1)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET); 
								else if(level_status1==0)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET); 
							}
							else if(AppData->Buff[5]==3)
							{
								HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_1);
							}
						}
						if(AppData->Buff[8]!=0x00)
						{
							if(AppData->Buff[8]==1)
							{
								if(level_status2==1)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET); 
								else if(level_status2==0)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET); 
							}
							else if(AppData->Buff[8]==2)
							{
								if(level_status2==1)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET); 
								else if(level_status2==0)
									HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET); 
							}
							else if(AppData->Buff[8]==3)
							{
								HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_0);
							}					
						}
						
						if((group_mode==1)&&(group_mode_id>=1)&&(group_mode_id<=8))   //point to multi-point
						{
							accept_flag=(1<<(group_mode_id-1));
						}
						else                                                        //point to point
						{
							accept_flag=1;
						}
						uplink_data_status=1;
						PPRINTF("Mapping succeeded\r\n");		
						HAL_Delay(1000); //Need to Wait for the RX window of the TX group to open
						for(uint8_t j=0;j<group_mode_id-1;j++)
						{
							IWDG_Refresh();
							HAL_Delay(2500);
						}
				  }
				}
			}
			break;
		}
		
		default:
		break;		
	}
}

static void OnTxTimerEvent( void )
{
  /*Wait for next tx slot*/
	TimerSetValue( &TxTimer,  APP_TX_DUTYCYCLE );
  TimerStart( &TxTimer);
  uplink_data_status=1;
}

static void LoraStartTx(void)
{
  /* send everytime timer elapses */
  TimerInit( &TxTimer, OnTxTimerEvent );
  TimerSetValue( &TxTimer,  APP_TX_DUTYCYCLE ); 
  OnTxTimerEvent();
}

static void OnAckEvent( void )
{
	ack_send_num++;
	if(ack_send_num>=5)
	{
		TimerStop( &AckTimer );
		ack_send_num=0;
		lora_wait_flags=0;	
		retransmission_flag=0;
	}
	else
	{
		if((group_mode==0)&&(group_mode_id==0))
			TimerSetValue( &AckTimer,  6000 );			
		else
			TimerSetValue( &AckTimer,  30000 );
		TimerStart( &AckTimer );
		uplinkcount--;
		uplink_data_status=1;
		retransmission_flag=1;
	}		
}

static void AckStartTX(void)
{
  TimerInit( &AckTimer, OnAckEvent );	
}

static void OnIWDGRefreshTimeoutEvent( void )
{
	TimerSetValue( &IWDGRefreshTimer,  18000);
  TimerStart( &IWDGRefreshTimer);
	is_time_to_IWDG_Refresh=1;
}

static void StartIWDGRefresh(void)
{
  /* send everytime timer elapses */
  TimerInit( &IWDGRefreshTimer, OnIWDGRefreshTimeoutEvent );
  TimerSetValue( &IWDGRefreshTimer,  18000); 
  TimerStart( &IWDGRefreshTimer);
}

static void send_exti(void)
{
	if(exitflag1==1)
	{	
	  if(sending_flag ==0)
		{	
			if((group_mode==0)&&((DIonetoDO!=0)||(DIonetoRO!=0)))
			{
				uplink_data_status=1;
				if((group_mode==0)&&(group_mode_id!=0))
				{
					request_flag=(0xff>>(8-group_mode_id));
				}
				if(group_mode_id==0)
					TimerSetValue( &AckTimer,  6000 );					
				else
					TimerSetValue( &AckTimer,  30000 );
				TimerStart( &AckTimer );
				lora_wait_flags=1;
			}
			else
			{
				exitflag1=0;
			}
		}		
	}
	
	if(exitflag2==1)
	{	
	  if(sending_flag ==0)
	  {	
			if((group_mode==0)&&((DItwotoDO!=0)||(DItwotoRO!=0)))
			{
				uplink_data_status=1;
				if((group_mode==0)&&(group_mode_id!=0))
				{
					request_flag=(0xff>>(8-group_mode_id));
				}
				if(group_mode_id==0)
					TimerSetValue( &AckTimer,  6000 );					
				else
					TimerSetValue( &AckTimer,  30000 );
				TimerStart( &AckTimer );
				lora_wait_flags=1;
			}
      else
			{
				exitflag2=0;
			}
		}							
	}	
}

static void lora_test_init(void)
{
  // Radio initialization
  RadioEvents.TxDone = test_OnTxDone;
  RadioEvents.RxDone = test_OnRxDone;
  RadioEvents.TxTimeout = test_OnTxTimeout;
  RadioEvents.RxTimeout = test_OnRxTimeout;
  RadioEvents.RxError = test_OnRxError;	
	
	Radio.Init( &RadioEvents );
  Radio.SetPublicNetwork( sync_value );
	Radio.SetMaxPayloadLength( MODEM_LORA, 255 );	
}

static void test_OnTxDone( void )
{
    Radio.Sleep( );
	  rx_waiting_flag=1;
}

static void test_OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	if((payload[0]==group_id[0])&&(payload[1]==group_id[1])&&(payload[2]==group_id[2])&&(payload[3]==group_id[3])
	   &&(payload[4]==group_id[4])&&(payload[5]==group_id[5])&&(payload[6]==group_id[6])&&(payload[7]==group_id[7]))
	{
		uint32_t crc_flag=0;
		crc_flag=crc32(payload,size-4);
		if(crc_flag==((payload[size-1]<<24)|(payload[size-2]<<16)|(payload[size-3]<<8)|(payload[size-4])))
		{		
			rxBufferSize=0;
			PPRINTF("\r\nrxDone\r\nData: ");
			for(uint16_t i=8;i<size-4;i++)
			{
				PPRINTF("%02x ",payload[i]);
				rxDataBuff[rxBufferSize++]=payload[i];
				HAL_Delay(6);
			}
			PPRINTF("\r\n");
			
			AppData.Buff=rxDataBuff;
      AppData.BuffSize=rxBufferSize;
			
			RxData(&AppData);
			downlinkcount++;	
		}	
    else
    {
			data_check_flag=1;
		}			
	}
  else
	{
		data_check_flag=1;
	}
}

static void test_OnTxTimeout( void )
{
  PPRINTF("OnTxTimeout\n\r");
  Radio.Sleep( );
	rx_waiting_flag=1;
}

static void test_OnRxTimeout( void )
{
  PPRINTF("OnRxTimeout\n\r");
}

static void test_OnRxError( void )
{
  PPRINTF("OnRxError\n\r");
}

static uint32_t crc32(uint8_t *data,uint16_t length) //CRC_32/ADCCP
{
  uint8_t i;
	uint32_t crc=0xffffffff;
	while(length--)
	{
		crc^=*data++;
		for(i=0;i<8;++i)
		{
			if(crc&1)
				crc=(crc>>1)^0xEDB88320;
			else
				crc=(crc>>1);
		}
	}
	return ~crc;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
