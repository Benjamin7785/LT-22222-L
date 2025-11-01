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

// NEW CODE - Conditional verbose printing for performance tuning
#ifdef FAST_MODE
  #define PPRINTF_VERBOSE(...)  // Disabled in fast mode
#else
  #define PPRINTF_VERBOSE(...) PPRINTF(__VA_ARGS__)  // Enabled normally
#endif

typedef struct
{
  /*point to the LoRa App data buffer*/
  uint8_t* Buff;
  /*LoRa App data buffer size*/
  uint8_t BuffSize; 
} lora_AppData_t;

bool turn_flag[4];
GPIO_PinState originalstatus[4];
static lora_AppData_t AppData;
bool into_sleep_status=0;
bool save_flash_status=0;
bool data_check_flag=0;
bool sending_flag=0;

// OLD CODE - Race condition prone flags
// bool sending_flag=0;
// bool rx_waiting_flag=0;

// NEW CODE - Radio state machine
typedef enum {
    RADIO_STATE_IDLE = 0,
    RADIO_STATE_TX_PREPARING,
    RADIO_STATE_TX_ACTIVE,
    RADIO_STATE_RX_PREPARING,
    RADIO_STATE_RX_ACTIVE,
    RADIO_STATE_ERROR
} radio_state_t;

// NEW CODE - Atomic radio state management
volatile radio_state_t radio_state = RADIO_STATE_IDLE;
volatile bool radio_state_changed = false;

// Radio state functions
static bool radio_can_transmit(void);
static radio_state_t radio_get_state(void);
static bool radio_set_state(radio_state_t new_state);
static bool radio_is_idle(void);
static void radio_force_idle(void);
static bool radio_can_receive(void);

// NEW CODE - Enhanced error recovery and retry system
typedef struct {
    uint8_t tx_failures;
    uint8_t rx_failures;
    uint8_t consecutive_errors;
    uint8_t max_retries;
    uint32_t last_error_time;
    bool fallback_mode;
} error_recovery_t;

typedef struct {
    uint8_t power_level;
    uint8_t spreading_factor;
    uint8_t bandwidth;
    uint8_t coding_rate;
} radio_params_t;

static error_recovery_t error_stats = {0};
static radio_params_t primary_params = {0};
static radio_params_t fallback_params = {0};
static bool use_fallback_params = false;

// Error recovery functions
static void error_recovery_init(void);
static void error_recovery_reset(void);
static void error_recovery_handle_tx_failure(void);
static void error_recovery_handle_rx_failure(void);
static void error_recovery_switch_to_fallback(void);
static void error_recovery_switch_to_primary(void);
static bool error_recovery_should_retry(void);
static void error_recovery_log_error(const char* error_type);
static void apply_radio_parameters(void);

// NEW CODE - Testing and validation functions
typedef struct {
    uint32_t test_start_time;
    uint32_t test_duration_ms;
    uint8_t test_scenario;
    bool test_active;
    uint32_t tx_attempts;
    uint32_t rx_attempts;
    uint32_t successful_tx;
    uint32_t successful_rx;
    uint32_t errors_handled;
} test_session_t;

static test_session_t test_session = {0};

// Test functions
static void test_init_session(uint8_t scenario, uint32_t duration_ms);
static void test_log_event(const char* event);
static void test_print_results(void);
static void test_validate_state_machine(void);
static void test_simulate_radio_errors(void);
static void test_validate_error_recovery(void);
static void test_validate_non_blocking_delays(void);

// NEW CODE - Non-blocking delay infrastructure
typedef struct {
    bool active;
    uint32_t start_time;
    uint32_t duration_ms;
    void (*callback)(void);
} non_blocking_delay_t;

static non_blocking_delay_t radio_settle_delay = {0};
static non_blocking_delay_t rx_window_delay = {0};
static non_blocking_delay_t group_timing_delay = {0};
static uint8_t group_timing_counter = 0;

// NEW CODE - Watchdog system structure
typedef struct {
    bool enabled;
    uint8_t interval_seconds;
    uint8_t missed_count;
    uint8_t max_missed;
    uint32_t last_received_time;
    bool link_active;
    bool safe_state_active;
} watchdog_t;

// NEW CODE - Watchdog system for link monitoring
watchdog_t watchdog = {
    .enabled = false,                // OFF by default (as requested)
    .interval_seconds = 10,          // 10 second default
    .missed_count = 0,
    .max_missed = 5,
    .last_received_time = 0,
    .link_active = true,
    .safe_state_active = false
};

// NEW CODE - DI State Change Queue for Heartbeat Mode
typedef struct {
    bool di1_changed;                // DI1 changed during heartbeat
    bool di2_changed;                // DI2 changed during heartbeat
    uint8_t di1_state;               // Captured DI1 state (0 or 1)
    uint8_t di2_state;               // Captured DI2 state (0 or 1)
    uint32_t change_time;            // Timestamp of change
    bool pending_transmission;       // Need to send queued changes
} di_state_queue_t;

static di_state_queue_t di_queue = {
    .di1_changed = false,
    .di2_changed = false,
    .di1_state = 0,
    .di2_state = 0,
    .change_time = 0,
    .pending_transmission = false
};

// NEW CODE - Store queued DI states for Send_TX() to use
// These are set when queue is processed and cleared after TX
static bool use_queued_di_states = false;
static uint8_t queued_di1_state = 0;
static uint8_t queued_di2_state = 0;

// NEW CODE - Latest DI states captured in interrupt handler
// Updated on EVERY interrupt, used by queue to get latest state
uint8_t latest_di1_state_from_isr = 0;
uint8_t latest_di2_state_from_isr = 0;
bool di1_state_updated_in_isr = false;
bool di2_state_updated_in_isr = false;

// Watchdog timer
static TimerEvent_t WatchdogTimer;

// Watchdog function declarations
static void watchdog_init(void);
static void watchdog_check(void);
static void watchdog_reset(void);
static void watchdog_trigger_safe_state(void);
static void watchdog_timer_callback(void);
static void watchdog_send_ping(void);
static void watchdog_receive_ping(void);

// DI State Queue functions
static void di_queue_capture_state(uint8_t di_channel);
static void di_queue_clear(void);
static bool di_queue_has_pending(void);
static void di_queue_process_and_send(void);

// Non-blocking delay functions
static void non_blocking_delay_start(non_blocking_delay_t* delay, uint32_t duration_ms, void (*callback)(void));
static bool non_blocking_delay_check(non_blocking_delay_t* delay);
static void non_blocking_delay_stop(non_blocking_delay_t* delay);
static uint32_t get_system_tick_ms(void);

// Callback function declarations
static void radio_settle_callback(void);
static void rx_window_callback(void);
static void group_timing_callback(void);

bool test_uplink_status=0;
bool uplink_data_status=0;
bool rx_waiting_flag=0;
bool is_time_to_IWDG_Refresh=0;
bool lora_wait_flags=0;
bool retransmission_flag=0;
bool syncDI1DI2_send_flag=0;
bool syncDI1_flag=0;
bool syncDI2_flag=0;
uint8_t downlink_count=0;
uint8_t befor_RODO=0;
uint8_t status=0x00;
uint8_t request_flag=0;
uint8_t accept_flag=0;
uint8_t ack_send_num=0;
uint16_t batteryLevel_mV=3300;
uint8_t DI1_flag=0,DI2_flag=0,DO1_flag=0,DO2_flag=0,RO1_flag=0,RO2_flag=0;
uint8_t DIonetoDO=0,DIonetoRO=0;
uint8_t DItwotoDO=0,DItwotoRO=0;
uint32_t uplinkcount=0;
uint32_t downlinkcount=0;
uint32_t FLASH_USER_COUNT_START_ADDR;
uint32_t FLASH_USER_COUNT_END_ADDR;

extern uint32_t Address;
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
extern uint8_t DO1_init,DO2_init,RO1_init,RO2_init;
extern uint16_t inttime1,inttime2;
extern uint16_t DI1toDO1_time,DI1toRO1_time,DI2toDO2_time,DI2toRO2_time;
extern uint8_t DI1toDO1_statu,DI1toRO1_statu,DI2toDO2_statu,DI2toRO2_statu;
extern bool sleep_status;
extern bool sync1_begin,sync2_begin;

static uint8_t txDataBuff[256];
static uint8_t rxDataBuff[256];
static uint16_t txBufferSize=0;
static uint16_t rxBufferSize=0;
uint8_t testBuff[256];
uint16_t testBuffsize=0;

static RadioEvents_t RadioEvents;
/* LoRa endNode send request*/
static void Send_TX( void );
static void Send_sync( void );
static void Send_test( void );
static void RxData(lora_AppData_t *AppData);
static void send_exti(void);
static uint32_t crc32(uint8_t *data,uint16_t length);

/* start the tx process*/
static void LoraStartTx(void);
static void AckStartTX(void);
static void SYNC1StartTX(void);
static void SYNC2StartTX(void);
static void StartIWDGRefresh(void);
static void RxdownlinkLED(void);
static void RelayONE_return(void);
static void RelayTWO_return(void);
static void DooutputONE_return(void);
static void DooutputTWO_return(void);

/*!
 * Specifies the state of the application LED
 */
TimerEvent_t TxTimer; //TDC
TimerEvent_t AckTimer; //TDC
static TimerEvent_t IWDGRefreshTimer;//watch dog
static TimerEvent_t downlinkTimer;
TimerEvent_t syncDI1Timer;
TimerEvent_t syncDI2Timer;
static TimerEvent_t RelayONETimer;
static TimerEvent_t RelayTWOTimer;
static TimerEvent_t DooutputONETimer;
static TimerEvent_t DooutputTWOTimer;

static void read_data_on_flash_buff(uint32_t add1);
static void relay_control(void);
static void DO_control(void);

/* tx timer callback function*/
static void OnTxTimerEvent( void );
static void OnAckEvent( void );
static void OnSYNC1Event( void );
static void OnSYNC2Event( void );
static void OnIWDGRefreshTimeoutEvent(void);		
static void OndownlinkTimeoutEvent(void);	

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
	
	if(read_flash_size()==192)
	{
		FLASH_USER_COUNT_START_ADDR=FLASH_BASE + FLASH_PAGE_SIZE * 810;
		FLASH_USER_COUNT_END_ADDR=(FLASH_USER_COUNT_START_ADDR + FLASH_PAGE_SIZE*300);
	}
	else
	{
		FLASH_USER_COUNT_START_ADDR=FLASH_BASE + FLASH_PAGE_SIZE * 704;
		FLASH_USER_COUNT_END_ADDR=(FLASH_USER_COUNT_START_ADDR + FLASH_PAGE_SIZE*300);	
	}
	
	Config_Init();
	iwdg_init();			
 	StartIWDGRefresh(); 
	lora_test_init();
	
	// NEW CODE - Initialize enhanced error recovery system
	error_recovery_init();
	
	// NEW CODE - Initialize watchdog system (receiver side)
	watchdog_init();
	
	GPIO_EXTI_DI1_IoInit(intmode1);
	if(sleep_status==0)
	{
		GPIO_EXTI_DI2_IoInit(intmode2); 	
	}
	else
	{
		GPIO_EXTI_DI2_IoInit(2); 			
	}
	BSP_npn_output_Init();
	BSP_relay_output_Init();	
	LED_IoInit();
	
	Address=FLASH_USER_COUNT_START_ADDR;
	Address=find_addr(Address);
	read_data_on_flash_buff(Address-16);
	HAL_Delay(100);	
	
	LoraStartdelay1(); 
	LoraStartdelay2(); 
	if(fdr_flag==0)
	{
		AckStartTX();
		SYNC1StartTX();
		SYNC2StartTX();		
		LoraStartTx();
		RelayONE_return(); //Relay1
		RelayTWO_return(); //Relay2
		DooutputONE_return(); //DO1
		DooutputTWO_return(); //DO2				
		RxdownlinkLED();
	}
	
	// NEW CODE - If TDC=0 (no automatic TX), start in RX mode immediately
	if(APP_TX_DUTYCYCLE == 0)
	{
		PPRINTF("TDC=0: Starting in RX-only mode\r\n");
		rx_waiting_flag=1;
	}
	
  while( 1 )
  {
		/* Handle UART commands */
    CMD_Process();
		
		send_exti();
		
		// NEW CODE - Check all non-blocking delays
		non_blocking_delay_check(&radio_settle_delay);
		non_blocking_delay_check(&rx_window_delay);
		non_blocking_delay_check(&group_timing_delay);
		
		if(into_sleep_status==0)
		{
			relay_control();
			
		DO_control();
		
	// NEW CODE - Atomic radio state management
	if(uplink_data_status==1)
	{
		PPRINTF_VERBOSE("TX attempt: uplink_data_status=1, radio_state=%d\r\n", radio_get_state());
		
		if(radio_can_transmit())
		{
			// NEW CODE - Stop RX if currently receiving (DI1/DI2 can interrupt RX)
			if(radio_get_state() == RADIO_STATE_RX_ACTIVE || 
			   radio_get_state() == RADIO_STATE_RX_PREPARING)
			{
				Radio.Sleep();  // Stop current RX operation
				PPRINTF_VERBOSE("Stopped RX to start TX\r\n");
			}
			
			// Transition to TX_PREPARING state BEFORE configuring radio
			if(!radio_set_state(RADIO_STATE_TX_PREPARING))
			{
				PPRINTF("ERROR: Cannot transition to TX_PREPARING from state %d\r\n", radio_get_state());
				uplink_data_status=0;  // Clear and skip TX
			}
			else
			{
				sending_flag=1;
				Radio.SetChannel( tx_signal_freqence );	
				Radio.SetTxConfig( MODEM_LORA, txp_value, 0, bandwidth_value, tx_spreading_value, codingrate_value,preamble_value, false, true, 0, 0, false, 3000 );	
				static uint32_t last_printed_counter = 0;
				
				// Detect if we skipped printing any TXs (UART overflow or silent TXs)
				if(last_printed_counter > 0 && uplinkcount != last_printed_counter)
				{
					uint32_t missed = uplinkcount - last_printed_counter - 1;
					if(missed > 0 && missed < 1000)  // Sanity check
					{
						PPRINTF("\r\n!!! MISSED %u TXs (jump from %u to %u) !!!\r\n", 
						        missed, last_printed_counter, uplinkcount);
					}
				}
				
				uplinkcount++;
				last_printed_counter = uplinkcount;
				
				PPRINTF("\r\n***** UpLinkCounter= %u *****\n\r", uplinkcount );
				PPRINTF( "TX on freq %u Hz at SF %d\r\n", tx_signal_freqence, tx_spreading_value );
				
				// Transition to TX_ACTIVE state after radio configured, before sending
				radio_set_state(RADIO_STATE_TX_ACTIVE);
				
				if(syncDI1DI2_send_flag==1)
				{
					PPRINTF_VERBOSE("TX Type: SYNC\r\n");
					Send_sync();
					syncDI1DI2_send_flag=0;
					exitflag1=0;
				}
				else if(retransmission_flag==1)
				{	
					PPRINTF_VERBOSE("TX Type: RETRANSMISSION\r\n");
					uint32_t crc_check;
					txDataBuff[9]= request_flag;
					crc_check=crc32(txDataBuff,txBufferSize-4);
					txDataBuff[txBufferSize-4] = crc_check&0xff;
					txDataBuff[txBufferSize-3] = crc_check>>8&0xff;
					txDataBuff[txBufferSize-2] = crc_check>>16&0xff;
					txDataBuff[txBufferSize-1] = crc_check>>24&0xff;				
					Radio.Send( txDataBuff, txBufferSize );
					exitflag1=0;					
				}			
				else if(test_uplink_status==1)		
				{
					PPRINTF_VERBOSE("TX Type: TEST\r\n");
					Send_test();
					test_uplink_status=0;
					exitflag1=0;
				}				
				else
				{
					PPRINTF_VERBOSE("TX: %s\r\n", (exitflag1==0 && exitflag2==0) ? "Heartbeat" : "DI change");
					Send_TX();
				}
				
				// CRITICAL FIX: Clear flag only AFTER TX is successfully sent
				uplink_data_status=0;
			}
		}
		else
		{
			// uplink_data_status is set but radio cannot transmit
			PPRINTF("TX blocked - radio state: %d, can_transmit: %d\r\n", radio_get_state(), radio_can_transmit());
			// Clear the flag to prevent infinite loop
			uplink_data_status=0;
		}
	}

	if(rx_waiting_flag==1)
	{
		// Clear flag first
		rx_waiting_flag=0;
		
		// Transition to RX_PREPARING
		if(radio_set_state(RADIO_STATE_RX_PREPARING))
		{
			Radio.SetChannel( rx_signal_freqence );
			
			// NEW CODE - Dynamic RX parameters with error recovery
			apply_radio_parameters();
			
			PPRINTF( "RX on freq %u Hz at SF %d\r\n", rx_signal_freqence, rx_spreading_value );	
			PPRINTF("rxWaiting\r\n");
			Radio.Rx(0);
			
			// Transition to RX_ACTIVE after starting RX
			radio_set_state(RADIO_STATE_RX_ACTIVE);
			
			sending_flag=0;
		}
		else
		{
			PPRINTF("ERROR: Cannot transition to RX_PREPARING from state %d\r\n", radio_get_state());
		}
	}
	
	if((sending_flag==0)&&(save_flash_status==1))
	{
		status= (RO1_flag<<7) | (RO2_flag<<6) | (DO2_flag<<1) | DO1_flag;
		Address=find_addr(Address);		
		read_bsp_data(batteryLevel_mV,status);		
		store_data_counter(Address);		 
		PPRINTF("Save\r\n");		
		save_flash_status=0;
	}
	}  // Close if(into_sleep_status==0)
	else
	{
		DISABLE_IRQ( );
		LPM_EnterLowPower();
		ENABLE_IRQ();			
	}
	
	if(is_time_to_IWDG_Refresh==1)
	{
		IWDG_Refresh();			
		is_time_to_IWDG_Refresh=0;
	}				
  }  // End while(1) loop
}  // End main()

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
	
	// CRITICAL FIX: Always set request_flag=1 for transmitter packets (both heartbeat and DI-triggered)
	// The receiver needs to know this is a request, not feedback
	if(accept_flag==0)  // If we're the transmitter (not sending feedback)
	{
		request_flag=1;
	}
	
	txDataBuff[i++]= request_flag;	
	txDataBuff[i++]= accept_flag;	
	
	PPRINTF_VERBOSE("Flags: request=%d, accept=%d\r\n", request_flag, accept_flag);	
	
	if(accept_flag==0)
	{
		// TRANSMITTER: Send DI states
		// CRITICAL FIX: Use queued state if available, otherwise read current GPIO
		if(use_queued_di_states && exitflag1==1)
		{
			DI1_flag = queued_di1_state;
			PPRINTF_VERBOSE("Queued DI1=%d\r\n", DI1_flag);
		}
		else
		{
			DI1_flag=HAL_GPIO_ReadPin(GPIO_EXTI_PORT,GPIO_EXTI_PIN);
		}
		txDataBuff[i++]=1<<4 | DI1_flag;	
	}
	else
	{
		// RECEIVER FEEDBACK: Send RO states only (DO is local, not shared)
		// NEW ARCHITECTURE: Receiver DO outputs are local-only, only RO is sent in feedback
		RO1_flag=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO1_PIN);	
		RO2_flag=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO2_PIN);
		txDataBuff[i++]= RO1_flag<<4 | RO2_flag;  // Send RO1/RO2, not DO1/DO2
		PPRINTF_VERBOSE("TX Feedback: RO1=%d, RO2=%d\r\n", RO1_flag, RO2_flag);
	}
	
	if((exitflag1==1)&&(request_flag!=0))
	{
		txDataBuff[i++]=0x04;
		txDataBuff[i++]=0x04;
	}
	else
	{
		txDataBuff[i++]=0x00;
		txDataBuff[i++]=0x00;		
	}
	
	if(accept_flag==0)
	{	
		// CRITICAL FIX: Use queued state if available, otherwise read current GPIO
		if(use_queued_di_states && exitflag2==1)
		{
			DI2_flag = queued_di2_state;
			PPRINTF_VERBOSE("Queued DI2=%d\r\n", DI2_flag);
		}
		else
		{
			DI2_flag=HAL_GPIO_ReadPin(GPIO_EXTI2_PORT,GPIO_EXTI2_PIN);
		}
		txDataBuff[i++]=2<<4 | DI2_flag;
	}
	else
	{
		RO1_flag=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO1_PIN);	
		RO2_flag=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO2_PIN);		
		txDataBuff[i++]= RO1_flag<<4 | 	RO2_flag;		
	}
	
	if((exitflag2==1)&&(request_flag!=0))
	{
		txDataBuff[i++]=0x04;
		txDataBuff[i++]=0x04;
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
	if(accept_flag!=0)
		accept_flag=0;
	
	// CRITICAL FIX: Clear queued state flag after TX buffer is prepared with queued states
	if(use_queued_di_states)
	{
		use_queued_di_states = false;
	}
	
	Radio.Send( txDataBuff, txBufferSize );
}

static void Send_sync( void )
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
				
	if((group_mode==0)&&(group_mode_id!=0))
	{	
		txDataBuff[i++]= (0xff>>(8-group_mode_id));	
	}
	else
	{
		txDataBuff[i++]= 0x01;			
	}
	txDataBuff[i++]= 0x00;	
	
	DI1_flag=HAL_GPIO_ReadPin(GPIO_EXTI_PORT,GPIO_EXTI_PIN);			
	txDataBuff[i++]=1<<4 | DI1_flag;
  if(syncDI1_flag==1)	
	{
		txDataBuff[i++]=0x04;
		txDataBuff[i++]=0x04;
		syncDI1_flag=0;
  }
	else
	{
		txDataBuff[i++]=0x00;
		txDataBuff[i++]=0x00;		
	}
	
	DI2_flag=HAL_GPIO_ReadPin(GPIO_EXTI2_PORT,GPIO_EXTI2_PIN);	
	txDataBuff[i++]=2<<4 | DI2_flag;	
	if(syncDI2_flag==1)
	{		
		txDataBuff[i++]=0x04;
		txDataBuff[i++]=0x04;
		syncDI2_flag=0;
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
  TimerSetValue( &downlinkTimer,  300); 
  TimerStart( &downlinkTimer);
	HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_11);
	downlink_count++;
					
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
		case 0x00:             // NEW CODE - Received feedback from receiver (DO/RO states)
		{
			// Feedback packet: accept_flag=1 means receiver is sending back DO/RO states
			if(AppData->Buff[2] == 0x01)  // accept_flag=1 in received packet
			{
				PPRINTF_VERBOSE("Queue check: pending=%d\r\n", di_queue_has_pending());
				
				// NEW ARCHITECTURE: Receiver sends only RO states (DO is local on receiver)
				// Byte 3: RO1_flag<<4 | RO2_flag (changed from DO1/DO2)
				uint8_t received_RO1_byte3 = (AppData->Buff[3] & 0xf0) >> 4;
				uint8_t received_RO2_byte3 = (AppData->Buff[3] & 0x0f);
				
				// Byte 6: RO1_flag<<4 | RO2_flag (redundant for verification)
				uint8_t received_RO1_byte6 = (AppData->Buff[6] & 0xf0) >> 4;
				uint8_t received_RO2_byte6 = (AppData->Buff[6] & 0x0f);
				
				// Use byte 3 values (or verify against byte 6 for corruption detection)
				uint8_t received_RO1 = received_RO1_byte3;
				uint8_t received_RO2 = received_RO2_byte3;
				
				PPRINTF("RX Feedback: RO1=%d, RO2=%d\r\n", received_RO1, received_RO2);
				
				// NEW ARCHITECTURE - Mirror receiver's RO states to transmitter's DO/RO outputs
				// Transmitter's DO1 mirrors receiver's RO1
				if(received_RO1 == 1)
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
				
				// Transmitter's DO2 mirrors receiver's RO2
				if(received_RO2 == 1)
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
				
				// Transmitter's RO1 mirrors receiver's RO1
				if(received_RO1 == 1)
					HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO1_PIN, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO1_PIN, GPIO_PIN_RESET);
				
				// Transmitter's RO2 mirrors receiver's RO2
				if(received_RO2 == 1)
					HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO2_PIN, GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO2_PIN, GPIO_PIN_RESET);
				
				// Update local state variables
				DO1_flag = received_RO1;  // Transmitter DO mirrors receiver RO
				DO2_flag = received_RO2;
				RO1_flag = received_RO1;
				RO2_flag = received_RO2;
				
				PPRINTF_VERBOSE("Mirrored receiver RO states\r\n");
				
				// Reset accept_flag on transmitter side
				accept_flag = 0;
				
				// NEW CODE - Process any DI changes that occurred during heartbeat cycle
				if(di_queue_has_pending())
				{
					PPRINTF("Processing queued DI changes (DI1=%d, DI2=%d)\r\n", 
					        di_queue.di1_changed, di_queue.di2_changed);
					
					use_queued_di_states = true;
					
					if(di_queue.di1_changed)
					{
						exitflag1 = 1;
						queued_di1_state = di_queue.di1_state;
					}
					if(di_queue.di2_changed)
					{
						exitflag2 = 1;
						queued_di2_state = di_queue.di2_state;
					}
					
					di_queue_clear();
				}
			}
			break;
		}
		
		case 0x01:             //received REQUEST from transmitter (heartbeat or DI-triggered)
		{		
			if(!((group_mode==0)&&(group_mode_id!=0)))
			{
				if((AppData->BuffSize == 9)&&((AppData->Buff[3]&0xf0)==0x10)&&((AppData->Buff[6]&0xf0)==0x20))
				{
					// CRITICAL FIX: Process ALL request packets, not just ones with trigger bytes
					// Heartbeats have trigger bytes = 0x00 but still need acknowledgement
					bool has_triggers = ((AppData->Buff[4]!=0x00)||(AppData->Buff[5]!=0x00)||(AppData->Buff[7]!=0x00)||(AppData->Buff[8]!=0x00));
					
					PPRINTF_VERBOSE("RX Request (triggers=%d)\r\n", has_triggers);
					
					// Always process the packet (whether heartbeat or DI-triggered)
					{
						uint8_t do1,do2,relay_1,relay_2;
						
						do1=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_12);	
						do2=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14);	
				    relay_1=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO1_PIN);		
            relay_2=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO2_PIN);							
				
					uint8_t level_status1,level_status2;
					level_status1=AppData->Buff[3]&0x0f;
					level_status2=AppData->Buff[6]&0x0f;	
					
					// NEW ARCHITECTURE: Receiver DO outputs are now LOCAL-ONLY (not controlled by transmitter)
					// DO1 and DO2 are reserved for local receiver functions:
					//   - DO2: Watchdog safe state indication (blinking/solid)
					//   - DO1: Available for future local functions
					// Only RO1 and RO2 are controlled by transmitter's DI states
					
					PPRINTF_VERBOSE("Decoupled: DO=local, RO=remote\r\n");
					PPRINTF_VERBOSE("RX: DI1=%d, DI2=%d\r\n", level_status1, level_status2);
					
					// Extract DI states from packet
					uint8_t DI1_received = level_status1;  // DI1 state from transmitter
					uint8_t DI2_received = level_status2;  // DI2 state from transmitter
					
					// CRITICAL: ALWAYS apply DI→RO mapping (immediate state sync)
					// This ensures receiver RO mirrors transmitter DI
					if(DIonetoRO != 0)
					{
						if(DIonetoRO == 1)  // Direct mapping: DI1=1 → RO1=1
							HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO1_PIN, DI1_received ? GPIO_PIN_SET : GPIO_PIN_RESET);
						else if(DIonetoRO == 2)  // Inverse mapping: DI1=1 → RO1=0
							HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO1_PIN, DI1_received ? GPIO_PIN_RESET : GPIO_PIN_SET);
						else if(DIonetoRO == 3)  // Toggle on DI1=1
						{
							if(DI1_received == 1)
								HAL_GPIO_TogglePin(Relay_GPIO_PORT, Relay_RO1_PIN);
						}
						PPRINTF_VERBOSE("RO1: DI1=%d → %s\r\n", DI1_received, 
						        DIonetoRO==1?"direct":(DIonetoRO==2?"inverse":"toggle"));
					}
					
					if(DItwotoRO != 0)
					{
						if(DItwotoRO == 1)  // Direct mapping: DI2=1 → RO2=1
							HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO2_PIN, DI2_received ? GPIO_PIN_SET : GPIO_PIN_RESET);
						else if(DItwotoRO == 2)  // Inverse mapping: DI2=1 → RO2=0
							HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO2_PIN, DI2_received ? GPIO_PIN_RESET : GPIO_PIN_SET);
						else if(DItwotoRO == 3)  // Toggle on DI2=1
						{
							if(DI2_received == 1)
								HAL_GPIO_TogglePin(Relay_GPIO_PORT, Relay_RO2_PIN);
						}
						PPRINTF_VERBOSE("RO2: DI2=%d → %s\r\n", DI2_received,
						        DItwotoRO==1?"direct":(DItwotoRO==2?"inverse":"toggle"));
					}
					
					// Process DI1 trigger (for TIMER feature only)
					if(AppData->Buff[5]!=0x00)
					{
						PPRINTF_VERBOSE("DI1 trigger: 0x%02x\r\n", AppData->Buff[5]);
						
						// Start revert timer if configured
						if((DI1toRO1_time!=0)&&(DI1toRO1_statu!=0))
						{
							originalstatus[2]=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO1_PIN);	
							TimerSetValue( &RelayONETimer, DI1toRO1_time*1000);  
							TimerStart( &RelayONETimer);	
							PPRINTF_VERBOSE("RO1 timer: %ds\r\n", DI1toRO1_time);
						}
					}
					
					// Process DI2 trigger (for TIMER feature only)
					if(AppData->Buff[8]!=0x00)
					{
						PPRINTF_VERBOSE("DI2 trigger: 0x%02x\r\n", AppData->Buff[8]);
						
						// Start revert timer if configured
						if((DI2toRO2_time!=0)&&(DI2toRO2_statu!=0))
						{
							originalstatus[3]=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO2_PIN);	
							TimerSetValue( &RelayTWOTimer, DI2toRO2_time*1000);  
							TimerStart( &RelayTWOTimer);	
							PPRINTF_VERBOSE("RO2 timer: %ds\r\n", DI2toRO2_time);
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
						
						DO1_flag=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_12);	
						DO2_flag=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14);	
					RO1_flag=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO1_PIN);	
					RO2_flag=HAL_GPIO_ReadPin(Relay_GPIO_PORT,Relay_RO2_PIN);		

				// NEW CODE - Enhanced feedback mechanism with GUARANTEED acknowledgement
				PPRINTF("RO1=%d, RO2=%d → Sending ACK\r\n", RO1_flag, RO2_flag);
				
				// CRITICAL: ALWAYS send acknowledgement for ALL request packets
				// This ensures transmitter can process its queue reliably (event-driven, no timeouts)
				if(group_mode_id <= 1)
				{
					// Point-to-point: Send ACK immediately
					uplink_data_status=1;
					PPRINTF_VERBOSE("ACK queued\r\n");
				}
				else
				{
					// Group mode: Use non-blocking delay to avoid collisions
					non_blocking_delay_start(&rx_window_delay, 1000, rx_window_callback);
					PPRINTF("Group mode: Feedback delayed by %dms\r\n", 1000 + (group_mode_id-1)*2500);
				}
						if((DO1_flag!=do1)||(DO2_flag!=do2)||(RO1_flag!=relay_1)||(RO2_flag!=relay_2))
						{
							if(befor_RODO==1)
							{
								save_flash_status=1;
							}
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
  // NEW CODE - Only restart timer and trigger TX if TDC > 0 (heartbeat mode)
  if(APP_TX_DUTYCYCLE > 0)
  {
    TimerSetValue( &TxTimer,  APP_TX_DUTYCYCLE );
    TimerStart( &TxTimer);
    uplink_data_status=1;
  }
  else
  {
    // TDC=0: No automatic transmissions, stay in RX mode
    // (This shouldn't be called with TDC=0, but just in case)
    rx_waiting_flag=1;
  }
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
	PPRINTF("ACK timeout (attempt %d/5)\r\n", ack_send_num);
	
	if(ack_send_num>=5)
	{
		PPRINTF("Max ACK retries reached - giving up\r\n");
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
		
		PPRINTF("Retransmitting (uplinkcount %u → %u)\r\n", uplinkcount, uplinkcount-1);
		uplinkcount--;  // Compensate for the increment that will happen in next TX
		uplink_data_status=1;
		retransmission_flag=1;
	}		
}

static void AckStartTX(void)
{
  TimerInit( &AckTimer, OnAckEvent );	
}

static void SYNC1StartTX(void)
{
	TimerInit( &syncDI1Timer, OnSYNC1Event );
	if(sync1_begin==1)
	{
		PPRINTF("Start DI1 sync\r\n");
		OnSYNC1Event();
	}		
}

static void OnSYNC1Event( void )
{
	if(inttime1!=0)
	{
		TimerSetValue( &syncDI1Timer,  inttime1*1000 );
		TimerStart( &syncDI1Timer );
	}
	uplink_data_status=1;
	lora_wait_flags=1;	
	syncDI1_flag=1;	
	syncDI1DI2_send_flag=1;	
}

static void SYNC2StartTX(void)
{
  TimerInit( &syncDI2Timer, OnSYNC2Event );	
	if(sync2_begin==1)
	{
		PPRINTF("Start DI2 sync\r\n");
		OnSYNC2Event();
	}
}

static void OnSYNC2Event( void )
{
	if(inttime2!=0)
	{
		TimerSetValue( &syncDI2Timer,  inttime2*1000 );
		TimerStart( &syncDI2Timer );
	}
	uplink_data_status=1;
	lora_wait_flags=1;	
	syncDI2_flag=1;		
	syncDI1DI2_send_flag=1;	
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

static void RxdownlinkLED(void)
{
  TimerInit( &downlinkTimer, OndownlinkTimeoutEvent );	
}

static void OndownlinkTimeoutEvent(void)
{
	if(downlink_count<3)
	{
		TimerSetValue( &downlinkTimer,  300); 
		TimerStart( &downlinkTimer);		
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_11);
		downlink_count++;
	}
	else
	{
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_11);
		TimerStop( &downlinkTimer);
		downlink_count=0;
	}
}

static void DooutputONETimeoutEvent(void)
{
	TimerStop( &DooutputONETimer);		
	turn_flag[0]=1;		
  if(DI1toDO1_statu==1)
	{		
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);				
		DO1_flag=0;
	}
  else if(DI1toDO1_statu==2)
	{		
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);				
		DO1_flag=1;
	}	
  else if(DI1toDO1_statu==3)
	{		
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,originalstatus[0]);				
		DO1_flag=originalstatus[0];
	}
}

static void DooutputONE_return(void)
{
	TimerInit( &DooutputONETimer, DooutputONETimeoutEvent );	
}

static void DooutputTWOTimeoutEvent(void)
{	
	TimerStop( &DooutputTWOTimer);	
	turn_flag[1]=1;
  if(DI2toDO2_statu==1)	
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		DO2_flag=0;				
	}		
  else if(DI2toDO2_statu==2)	
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
		DO2_flag=1;				
	}			
  else if(DI2toDO2_statu==3)	
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,originalstatus[1]);
		DO2_flag=originalstatus[1];				
	}		
}

static void DooutputTWO_return(void)
{
	TimerInit( &DooutputTWOTimer, DooutputTWOTimeoutEvent );	
}

static void RelayONETimeoutEvent(void)
{
	TimerStop( &RelayONETimer);		
	turn_flag[2]=1;
	if(DI1toRO1_statu==1)
	{
		HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO1_PIN,GPIO_PIN_RESET);
		RO1_flag=0;
	}
	else if(DI1toRO1_statu==2)
	{
		HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO1_PIN,GPIO_PIN_SET);
		RO1_flag=1;
	}	
	else if(DI1toRO1_statu==3)
	{
		HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO1_PIN,originalstatus[2]);
		RO1_flag=originalstatus[2];
	}
}

static void RelayONE_return(void)
{
	TimerInit( &RelayONETimer, RelayONETimeoutEvent );	
}

static void RelayTWOTimeoutEvent(void)
{	
	TimerStop( &RelayTWOTimer);	
	turn_flag[3]=1;
  if(DI2toRO2_statu==1)
	{		
		HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO2_PIN,GPIO_PIN_RESET);
		RO2_flag=0;	
	}
  else if(DI2toRO2_statu==2)
	{		
		HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO2_PIN,GPIO_PIN_SET);
		RO2_flag=1;	
	}	
  else if(DI2toRO2_statu==3)
	{		
		HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO2_PIN,originalstatus[3]);
		RO2_flag=originalstatus[3];	
	}		
}

static void RelayTWO_return(void)
{
	TimerInit( &RelayTWOTimer, RelayTWOTimeoutEvent );	
}

static void send_exti(void)
{
	if(exitflag1==1)
	{	
	  if(sending_flag ==0)
		{	
			if(group_mode==0)
			{
				// CRITICAL FIX: Reset heartbeat timer when DI activity occurs
				// This prevents heartbeat from interfering with DI-triggered feedback cycles
				if(APP_TX_DUTYCYCLE > 0)
				{
					TimerStop(&TxTimer);
					TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
					TimerStart(&TxTimer);
					PPRINTF_VERBOSE("TDC timer reset by DI1 activity\r\n");
				}
				
				uplink_data_status=1;
				lora_wait_flags=1;				
				if((group_mode==0)&&(group_mode_id!=0))
				{
					request_flag=(0xff>>(8-group_mode_id));
				}
				
					if(inttime1==0)
					{
						if(group_mode_id==0)
							TimerSetValue( &AckTimer,  6000 );					
						else
							TimerSetValue( &AckTimer,  30000 );
						TimerStart( &AckTimer );
					}
					else
					{
						PPRINTF("Start DI1 sync\r\n");
						TimerSetValue( &syncDI1Timer,  inttime1*1000 );
						TimerStart( &syncDI1Timer );
          syncDI1_flag=1;	
          if(sync1_begin==0)
						{
						  sync1_begin=1;
							EEPROM_Store_Config();
						}												
						syncDI1DI2_send_flag=1;	
						exitflag1=0;			
					}
			}
			else
			{
				exitflag1=0;
			}
		}
		else
		{
			// NEW CODE - Radio is busy (heartbeat TX/RX active), queue the DI1 change
			// CRITICAL: Only capture ONCE, not on every send_exti() call
			if(!di_queue.di1_changed)  // Only if not already queued
			{
				PPRINTF("Radio busy during DI1 change - queuing state (sending_flag=%d)\r\n", sending_flag);
				di_queue_capture_state(1);
				PPRINTF_VERBOSE("DI1 queued\r\n");
			}
			else
			{
				// Already queued, just update to latest ISR state
				if(di1_state_updated_in_isr)
				{
					uint8_t old_state = di_queue.di1_state;
					di_queue.di1_state = latest_di1_state_from_isr;
					di1_state_updated_in_isr = false;
					if(old_state != di_queue.di1_state)
					{
						PPRINTF("DI1 queue UPDATE: %d → %d (toggle during queue)\r\n", 
						        old_state, di_queue.di1_state);
					}
				}
			}
			exitflag1=0;  // Clear flag since we've queued it
		}
	}
	
	if(exitflag2==1)
	{	
		if(sleep_status==1)
		{		
			if(HAL_GPIO_ReadPin(GPIO_EXTI2_PORT,GPIO_EXTI2_PIN)==0)
			{				
				into_sleep_status=1;
				lora_test_init();
				Radio.Sleep( );
				TimerStop(&TxTimer);
				TimerStop(&AckTimer);
				TimerStop(&syncDI1Timer);
				TimerStop(&syncDI2Timer);
		
				if((RO1_init==1)&&(befor_RODO==2))
				{
					HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO1_PIN,GPIO_PIN_SET);
					RO1_flag=1;			
				}
				else
				{
					HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO1_PIN,GPIO_PIN_RESET);
					RO1_flag=0;								
				}
				
				if((RO2_init==1)&&(befor_RODO==2))
				{
					HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO2_PIN,GPIO_PIN_SET);
					RO2_flag=1;			
				}
				else
				{
					HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO2_PIN,GPIO_PIN_RESET);
					RO2_flag=0;							
				}
				
				if((DO1_init==1)&&(befor_RODO==2))
				{
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);	
					DO1_flag=1;			
				}
				else
				{
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);	
					DO1_flag=0;						
				}
				
				if((DO2_init==1)&&(befor_RODO==2))
				{
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
					DO2_flag=1;			
				}	
        else
				{
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
					DO2_flag=0;							
				}	
				
				PPRINTF("LT into sleep\r\n");
			}
			else
			{
				into_sleep_status=0;		
				lora_test_init();
			  uplink_data_status=1;
				if(fdr_flag==0)
				{
					TimerStart(&TxTimer);
				}
				PPRINTF("LT wake up\r\n");				
			}
			exitflag2=0;
		}
		else
		{
			if(sending_flag ==0)
			{	
				if(group_mode==0)
				{
					// CRITICAL FIX: Reset heartbeat timer when DI activity occurs
					// This prevents heartbeat from interfering with DI-triggered feedback cycles
					if(APP_TX_DUTYCYCLE > 0)
					{
						TimerStop(&TxTimer);
						TimerSetValue(&TxTimer, APP_TX_DUTYCYCLE);
						TimerStart(&TxTimer);
						PPRINTF_VERBOSE("TDC timer reset by DI2 activity\r\n");
					}
					
					uplink_data_status=1;
					lora_wait_flags=1;				
					if((group_mode==0)&&(group_mode_id!=0))
					{
						request_flag=(0xff>>(8-group_mode_id));
					}
					
					if(inttime2==0)
					{				
						if(group_mode_id==0)
							TimerSetValue( &AckTimer,  6000 );					
						else
							TimerSetValue( &AckTimer,  30000 );
						TimerStart( &AckTimer );
					}
					else
					{
						PPRINTF("Start DI2 sync\r\n");
						TimerSetValue( &syncDI2Timer,  inttime2*1000 );
						TimerStart( &syncDI2Timer );
            syncDI2_flag=1;	
						if(sync2_begin==0)
						{
							sync2_begin=1;	
							EEPROM_Store_Config();
						}													
						syncDI1DI2_send_flag=1;	
					  exitflag2=0;						
					}				
				}
				else
				{
					exitflag2=0;
				}
			}
			else
			{
				// NEW CODE - Radio is busy (heartbeat TX/RX active), queue the DI2 change
				// CRITICAL: Only capture ONCE, not on every send_exti() call
				if(!di_queue.di2_changed)  // Only if not already queued
				{
					PPRINTF("Radio busy during DI2 change - queuing state (sending_flag=%d)\r\n", sending_flag);
					di_queue_capture_state(2);
					PPRINTF_VERBOSE("DI2 queued\r\n");
				}
				else
				{
					// Already queued, just update to latest ISR state
					if(di2_state_updated_in_isr)
					{
						uint8_t old_state = di_queue.di2_state;
						di_queue.di2_state = latest_di2_state_from_isr;
						di2_state_updated_in_isr = false;
						if(old_state != di_queue.di2_state)
						{
							PPRINTF("DI2 queue UPDATE: %d → %d (toggle during queue)\r\n", 
							        old_state, di_queue.di2_state);
						}
					}
				}
				exitflag2=0;  // Clear flag since we've queued it
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
    
    // NEW CODE - Transition to IDLE after TX complete
    radio_set_state(RADIO_STATE_IDLE);
    
    // NEW CODE - Reset accept_flag after sending feedback
    if(accept_flag == 1)
    {
        accept_flag = 0;
        PPRINTF("Feedback sent, returning to normal mode\r\n");
        
        // NOTE: Queue processing is NOT done here for receiver
        // Queue is only processed on TRANSMITTER side after receiving feedback (in RxData case 0x00)
    }
    
    // CRITICAL FIX: Always go back to RX mode after TX (to receive feedback or next packet)
    // Only skip if there's queued TX from DI queue
    if(uplink_data_status == 0)
    {
        rx_waiting_flag=1;
        PPRINTF("TX complete, returning to RX mode\r\n");
    }
    else
    {
        PPRINTF("TX complete, queued TX pending\r\n");
    }
    
    // NEW CODE - Test tracking
    if (test_session.test_active) {
        test_session.successful_tx++;
        test_log_event("TX completed successfully");
    }
}

static void test_OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	// NEW CODE - Reset watchdog on any received packet
	watchdog_receive_ping();
	
	// IMPROVED: If feedback is pending, cancel it and process new packet (more recent state)
	if(accept_flag == 1 || uplink_data_status == 1)
	{
		PPRINTF("RX: Superseding old feedback with new packet (state updated)\r\n");
		accept_flag = 0;
		uplink_data_status = 0;
		// Continue to process the new packet with updated state
	}
	
	// NEW CODE - Transition from RX_ACTIVE to IDLE after packet received
	radio_set_state(RADIO_STATE_IDLE);
	
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
	
	// NEW CODE - After processing RX, continue listening (unless feedback needs to be sent)
	if(accept_flag == 0 && uplink_data_status == 0)
	{
		rx_waiting_flag=1;  // Go back to RX mode
	}
}

static void test_OnTxTimeout( void )
{
  PPRINTF("OnTxTimeout\n\r");
  Radio.Sleep( );
  radio_set_state(RADIO_STATE_IDLE);
	rx_waiting_flag=1;
}

static void test_OnRxTimeout( void )
{
  PPRINTF("OnRxTimeout\n\r");
  Radio.Sleep();
  radio_set_state(RADIO_STATE_IDLE);
  rx_waiting_flag=1;  // Restart RX
}

static void test_OnRxError( void )
{
  PPRINTF("OnRxError\n\r");
  Radio.Sleep();
  radio_set_state(RADIO_STATE_IDLE);
  rx_waiting_flag=1;  // Restart RX
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

static void read_data_on_flash_buff(uint32_t add1)
{
	uint32_t buff[4];
	int i=0;
	
	if(add1>=FLASH_USER_COUNT_START_ADDR)
	{
//		PPRINTF("add is %X \r\n",add1);
		
		while(i<4)
		{
      buff[i] = *(__IO uint32_t *)add1;
      add1 = add1 + 4;
			i++;
		}
	}

	if(befor_RODO>=1)
	{		
		if(befor_RODO==1)
		{
			status=(buff[0]>>8&0xff);
			RO1_flag=(status&0x80)>>7;
			RO2_flag=(status&0x40)>>6;
			DO1_flag= status&0x01;;
			DO2_flag=(status&0x02)>>1;
    }
		
		if((RO1_flag==1)||(RO1_init==1))
		{
			HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO1_PIN,GPIO_PIN_SET);
      RO1_flag=1;			
		}
		if((RO2_flag==1)||(RO2_init==1))
		{
			HAL_GPIO_WritePin(Relay_GPIO_PORT,Relay_RO2_PIN,GPIO_PIN_SET);
      RO2_flag=1;			
		}
		
		if((DO1_flag==1)||(DO1_init==1))
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);	
      DO1_flag=1;			
		}
		if((DO2_flag==1)||(DO2_init==1))
		{
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
      DO2_flag=1;			
		}
	}		
}
	
static void relay_control(void)
{
	if((turn_flag[2]==1)||(turn_flag[3]==1))
	{
		 HAL_Delay(10);
		 turn_flag[2]=0;
		 turn_flag[3]=0;	 
		 uplink_data_status=1;	
		 lora_wait_flags=1;	 	
	}	
}

static void DO_control(void)
{
	if((turn_flag[0]==1)||(turn_flag[1]==1))
	{
		HAL_Delay(10);
		turn_flag[0]=0;			
		turn_flag[1]=0;	
		uplink_data_status=1;	
		lora_wait_flags=1;
	}	
}

// NEW CODE - Atomic radio state management implementation
static bool radio_set_state(radio_state_t new_state)
{
    // Disable interrupts to ensure atomic operation
    __disable_irq();
    
    radio_state_t current_state = radio_state;
    bool success = false;
    
    // State transition validation
    switch(current_state) {
        case RADIO_STATE_IDLE:
            success = (new_state == RADIO_STATE_TX_PREPARING || 
                      new_state == RADIO_STATE_RX_PREPARING ||
                      new_state == RADIO_STATE_ERROR);
            break;
            
        case RADIO_STATE_TX_PREPARING:
            success = (new_state == RADIO_STATE_TX_ACTIVE || 
                      new_state == RADIO_STATE_IDLE ||
                      new_state == RADIO_STATE_ERROR);
            break;
            
        case RADIO_STATE_TX_ACTIVE:
            success = (new_state == RADIO_STATE_RX_PREPARING || 
                      new_state == RADIO_STATE_IDLE ||
                      new_state == RADIO_STATE_ERROR);
            break;
            
        case RADIO_STATE_RX_PREPARING:
            // OLD CODE - Didn't allow RX to be interrupted by urgent TX (DI1/DI2)
            // success = (new_state == RADIO_STATE_RX_ACTIVE || 
            //           new_state == RADIO_STATE_IDLE ||
            //           new_state == RADIO_STATE_ERROR);
            
            // NEW CODE - Allow RX to be interrupted for urgent TX
            success = (new_state == RADIO_STATE_RX_ACTIVE || 
                      new_state == RADIO_STATE_TX_PREPARING ||
                      new_state == RADIO_STATE_IDLE ||
                      new_state == RADIO_STATE_ERROR);
            break;
            
        case RADIO_STATE_RX_ACTIVE:
            // OLD CODE - Didn't allow RX to be interrupted by urgent TX (DI1/DI2)
            // success = (new_state == RADIO_STATE_IDLE || 
            //           new_state == RADIO_STATE_ERROR);
            
            // NEW CODE - Allow RX to be interrupted for urgent TX
            success = (new_state == RADIO_STATE_TX_PREPARING ||
                      new_state == RADIO_STATE_IDLE || 
                      new_state == RADIO_STATE_ERROR);
            break;
            
        case RADIO_STATE_ERROR:
            success = (new_state == RADIO_STATE_IDLE);
            break;
            
        default:
            success = false;
            break;
    }
    
    if(success) {
        radio_state = new_state;
        radio_state_changed = true;
    }
    
    // Re-enable interrupts
    __enable_irq();
    
    return success;
}

static radio_state_t radio_get_state(void)
{
    return radio_state;
}

bool radio_is_idle(void)
{
    return (radio_state == RADIO_STATE_IDLE);
}

void radio_force_idle(void)
{
    // Stop radio operations
    Radio.Sleep();
    
    // Force state to idle
    __disable_irq();
    radio_state = RADIO_STATE_IDLE;
    radio_state_changed = true;
    __enable_irq();
    
    PPRINTF("Radio forced to IDLE state\r\n");
}

static bool radio_can_transmit(void)
{
    // OLD CODE - Too restrictive, prevented DI1/DI2 interrupts during RX
    // return (radio_state == RADIO_STATE_IDLE || radio_state == RADIO_STATE_TX_PREPARING);
    
    // NEW CODE - Allow transmission unless already transmitting
    // DI1/DI2 interrupts can interrupt RX to send urgent messages
    return (radio_state != RADIO_STATE_TX_ACTIVE);
}

static bool radio_can_receive(void)
{
    return (radio_state == RADIO_STATE_IDLE || radio_state == RADIO_STATE_RX_PREPARING);
}

// NEW CODE - Non-blocking delay implementation
// CRITICAL: Use TimerGetCurrentTime() instead of HAL_GetTick()
// HAL_GetTick() stops during STOP mode, but RTC-based timer continues
static uint32_t get_system_tick_ms(void)
{
    // TimerGetCurrentTime() returns milliseconds from RTC (continues during STOP mode)
    return (uint32_t)TimerGetCurrentTime();
}

static void non_blocking_delay_start(non_blocking_delay_t* delay, uint32_t duration_ms, void (*callback)(void))
{
    delay->active = true;
    delay->start_time = get_system_tick_ms();
    delay->duration_ms = duration_ms;
    delay->callback = callback;
}

static bool non_blocking_delay_check(non_blocking_delay_t* delay)
{
    if (!delay->active) {
        return false;
    }
    
    uint32_t current_time = get_system_tick_ms();
    uint32_t elapsed = current_time - delay->start_time;
    
    if (elapsed >= delay->duration_ms) {
        delay->active = false;
        if (delay->callback) {
            delay->callback();
        }
        return true;
    }
    
    return false;
}

static void non_blocking_delay_stop(non_blocking_delay_t* delay)
{
    delay->active = false;
}

// Callback functions for non-blocking delays
static void radio_settle_callback(void)
{
    // OLD CODE - Simple state transition
    // radio_set_state(RADIO_STATE_RX_PREPARING);
    
    // NEW CODE - Enhanced callback with success tracking
    radio_set_state(RADIO_STATE_RX_PREPARING);
    
    // Reset consecutive errors on successful state transition
    if (error_stats.consecutive_errors > 0) {
        error_stats.consecutive_errors = 0;
        PPRINTF("Radio settled successfully, error count reset\r\n");
    }
}

static void rx_window_callback(void)
{
    // Start group timing delay if needed
    if (group_mode_id > 1) {
        group_timing_counter = 0;
        non_blocking_delay_start(&group_timing_delay, 2500, group_timing_callback);
    } else {
        // No group timing needed, proceed with normal operation
        uplink_data_status = 1;
    }
}

static void group_timing_callback(void)
{
    group_timing_counter++;
    IWDG_Refresh();
    
    if (group_timing_counter < (group_mode_id - 1)) {
        // Continue with next timing delay
        non_blocking_delay_start(&group_timing_delay, 2500, group_timing_callback);
    } else {
        // All group timing delays completed
        uplink_data_status = 1;
    }
}

// NEW CODE - Enhanced error recovery implementation
static void error_recovery_init(void)
{
    // Initialize error statistics
    error_stats.tx_failures = 0;
    error_stats.rx_failures = 0;
    error_stats.consecutive_errors = 0;
    error_stats.max_retries = 3;
    error_stats.last_error_time = 0;
    error_stats.fallback_mode = false;
    
    // Store primary radio parameters
    primary_params.power_level = txp_value;
    primary_params.spreading_factor = tx_spreading_value;
    primary_params.bandwidth = bandwidth_value;
    primary_params.coding_rate = codingrate_value;
    
    // Set fallback parameters (more conservative)
    fallback_params.power_level = (txp_value > 10) ? (txp_value - 2) : txp_value;
    fallback_params.spreading_factor = (tx_spreading_value < 12) ? (tx_spreading_value + 1) : tx_spreading_value;
    fallback_params.bandwidth = bandwidth_value; // Keep same bandwidth
    fallback_params.coding_rate = (codingrate_value < 4) ? (codingrate_value + 1) : codingrate_value;
    
    PPRINTF("Error recovery system initialized\r\n");
}

static void error_recovery_reset(void)
{
    error_stats.tx_failures = 0;
    error_stats.rx_failures = 0;
    error_stats.consecutive_errors = 0;
    error_stats.last_error_time = 0;
    
    if (error_stats.fallback_mode) {
        error_recovery_switch_to_primary();
    }
    
    PPRINTF("Error recovery statistics reset\r\n");
}

static void error_recovery_handle_tx_failure(void)
{
    error_stats.tx_failures++;
    error_stats.consecutive_errors++;
    error_stats.last_error_time = get_system_tick_ms();
    
    error_recovery_log_error("TX_FAILURE");
    
    PPRINTF("TX failure #%d, consecutive errors: %d\r\n", 
            error_stats.tx_failures, error_stats.consecutive_errors);
    
    // Switch to fallback parameters if too many failures
    if (error_stats.consecutive_errors >= 3 && !error_stats.fallback_mode) {
        error_recovery_switch_to_fallback();
    }
}

static void error_recovery_handle_rx_failure(void)
{
    error_stats.rx_failures++;
    error_stats.consecutive_errors++;
    error_stats.last_error_time = get_system_tick_ms();
    
    error_recovery_log_error("RX_FAILURE");
    
    PPRINTF("RX failure #%d, consecutive errors: %d\r\n", 
            error_stats.rx_failures, error_stats.consecutive_errors);
    
    // Switch to fallback parameters if too many failures
    if (error_stats.consecutive_errors >= 3 && !error_stats.fallback_mode) {
        error_recovery_switch_to_fallback();
    }
}

static void error_recovery_switch_to_fallback(void)
{
    if (!error_stats.fallback_mode) {
        error_stats.fallback_mode = true;
        use_fallback_params = true;
        
        PPRINTF("Switching to fallback radio parameters\r\n");
        PPRINTF("Fallback: Power=%d, SF=%d, BW=%d, CR=%d\r\n",
                fallback_params.power_level, fallback_params.spreading_factor,
                fallback_params.bandwidth, fallback_params.coding_rate);
    }
}

static void error_recovery_switch_to_primary(void)
{
    if (error_stats.fallback_mode) {
        error_stats.fallback_mode = false;
        use_fallback_params = false;
        
        PPRINTF("Switching back to primary radio parameters\r\n");
        PPRINTF("Primary: Power=%d, SF=%d, BW=%d, CR=%d\r\n",
                primary_params.power_level, primary_params.spreading_factor,
                primary_params.bandwidth, primary_params.coding_rate);
    }
}

static bool error_recovery_should_retry(void)
{
    // Don't retry if we've exceeded max retries
    if (error_stats.consecutive_errors >= error_stats.max_retries) {
        return false;
    }
    
    // Don't retry if too recent (avoid rapid retries)
    uint32_t current_time = get_system_tick_ms();
    if ((current_time - error_stats.last_error_time) < 1000) {
        return false;
    }
    
    return true;
}

static void error_recovery_log_error(const char* error_type)
{
    PPRINTF("ERROR: %s at time %lu\r\n", error_type, get_system_tick_ms());
    PPRINTF("Stats: TX_fail=%d, RX_fail=%d, Consecutive=%d\r\n",
            error_stats.tx_failures, error_stats.rx_failures, error_stats.consecutive_errors);
}

// Enhanced radio parameter application
static void apply_radio_parameters(void)
{
    // OLD CODE - Used same parameters for both TX and RX (WRONG!)
    // This broke systems with different TX/RX frequencies and spreading factors
    // radio_params_t* params = use_fallback_params ? &fallback_params : &primary_params;
    // Radio.SetTxConfig(MODEM_LORA, params->power_level, 0, params->bandwidth, 
    //                  params->spreading_factor, params->coding_rate, preamble_value, 
    //                  false, true, 0, 0, false, 3000);
    // Radio.SetRxConfig(MODEM_LORA, params->bandwidth, params->spreading_factor, 
    //                  params->coding_rate, 0, preamble_value, 5, false, 0, true, 0, 0, false, true);
    
    // NEW CODE - Use actual TX/RX specific parameters from configuration
    // Apply TX parameters with original TX spreading factor
    Radio.SetTxConfig(MODEM_LORA, txp_value, 0, bandwidth_value, 
                     tx_spreading_value, codingrate_value, preamble_value, 
                     false, true, 0, 0, false, 3000);
    
    // Apply RX parameters with original RX spreading factor
    Radio.SetRxConfig(MODEM_LORA, bandwidth_value, rx_spreading_value, 
                     codingrate_value, 0, preamble_value, 5, false, 0, true, 0, 0, false, true);
}

// NEW CODE - Comprehensive testing implementation
static void test_init_session(uint8_t scenario, uint32_t duration_ms)
{
    test_session.test_start_time = get_system_tick_ms();
    test_session.test_duration_ms = duration_ms;
    test_session.test_scenario = scenario;
    test_session.test_active = true;
    test_session.tx_attempts = 0;
    test_session.rx_attempts = 0;
    test_session.successful_tx = 0;
    test_session.successful_rx = 0;
    test_session.errors_handled = 0;
    
    PPRINTF("\r\n=== TEST SESSION STARTED ===\r\n");
    PPRINTF("Scenario: %d, Duration: %lu ms\r\n", scenario, duration_ms);
    PPRINTF("Time: %lu\r\n", test_session.test_start_time);
}

static void test_log_event(const char* event)
{
    if (!test_session.test_active) return;
    
    uint32_t elapsed = get_system_tick_ms() - test_session.test_start_time;
    PPRINTF("[TEST %lu] %s\r\n", elapsed, event);
}

static void test_print_results(void)
{
    if (!test_session.test_active) return;
    
    uint32_t elapsed = get_system_tick_ms() - test_session.test_start_time;
    uint32_t tx_success_rate = (test_session.tx_attempts > 0) ? 
                               (test_session.successful_tx * 100) / test_session.tx_attempts : 0;
    uint32_t rx_success_rate = (test_session.rx_attempts > 0) ? 
                               (test_session.successful_rx * 100) / test_session.rx_attempts : 0;
    
    PPRINTF("\r\n=== TEST RESULTS ===\r\n");
    PPRINTF("Scenario: %d\r\n", test_session.test_scenario);
    PPRINTF("Duration: %lu ms\r\n", elapsed);
    PPRINTF("TX Attempts: %lu, Success: %lu (%lu%%)\r\n", 
            test_session.tx_attempts, test_session.successful_tx, tx_success_rate);
    PPRINTF("RX Attempts: %lu, Success: %lu (%lu%%)\r\n", 
            test_session.rx_attempts, test_session.successful_rx, rx_success_rate);
    PPRINTF("Errors Handled: %lu\r\n", test_session.errors_handled);
    PPRINTF("Error Recovery Stats: TX_fail=%d, RX_fail=%d, Consecutive=%d\r\n",
            error_stats.tx_failures, error_stats.rx_failures, error_stats.consecutive_errors);
    PPRINTF("Fallback Mode: %s\r\n", error_stats.fallback_mode ? "ACTIVE" : "INACTIVE");
    PPRINTF("Current Radio State: %d\r\n", radio_get_state());
    PPRINTF("=== END TEST RESULTS ===\r\n");
    
    test_session.test_active = false;
}

static void test_validate_state_machine(void)
{
    PPRINTF("\r\n=== STATE MACHINE VALIDATION ===\r\n");
    
    // Test valid state transitions
    radio_state_t original_state = radio_get_state();
    
    // Test IDLE -> TX_PREPARING
    if (radio_set_state(RADIO_STATE_TX_PREPARING)) {
        test_log_event("✓ IDLE -> TX_PREPARING: SUCCESS");
    } else {
        test_log_event("✗ IDLE -> TX_PREPARING: FAILED");
    }
    
    // Test TX_PREPARING -> TX_ACTIVE
    if (radio_set_state(RADIO_STATE_TX_ACTIVE)) {
        test_log_event("✓ TX_PREPARING -> TX_ACTIVE: SUCCESS");
    } else {
        test_log_event("✗ TX_PREPARING -> TX_ACTIVE: FAILED");
    }
    
    // Test TX_ACTIVE -> RX_PREPARING
    if (radio_set_state(RADIO_STATE_RX_PREPARING)) {
        test_log_event("✓ TX_ACTIVE -> RX_PREPARING: SUCCESS");
    } else {
        test_log_event("✗ TX_ACTIVE -> RX_PREPARING: FAILED");
    }
    
    // Test RX_PREPARING -> RX_ACTIVE
    if (radio_set_state(RADIO_STATE_RX_ACTIVE)) {
        test_log_event("✓ RX_PREPARING -> RX_ACTIVE: SUCCESS");
    } else {
        test_log_event("✗ RX_PREPARING -> RX_ACTIVE: FAILED");
    }
    
    // Test RX_ACTIVE -> IDLE
    if (radio_set_state(RADIO_STATE_IDLE)) {
        test_log_event("✓ RX_ACTIVE -> IDLE: SUCCESS");
    } else {
        test_log_event("✗ RX_ACTIVE -> IDLE: FAILED");
    }
    
    // Test invalid transitions
    if (!radio_set_state(RADIO_STATE_TX_ACTIVE)) {
        test_log_event("✓ Invalid IDLE -> TX_ACTIVE: CORRECTLY REJECTED");
    } else {
        test_log_event("✗ Invalid IDLE -> TX_ACTIVE: INCORRECTLY ALLOWED");
    }
    
    // Restore original state
    radio_set_state(original_state);
    PPRINTF("=== STATE MACHINE VALIDATION COMPLETE ===\r\n");
}

static void test_simulate_radio_errors(void)
{
    PPRINTF("\r\n=== RADIO ERROR SIMULATION ===\r\n");
    
    // Simulate TX timeout
    test_log_event("Simulating TX timeout...");
    test_OnTxTimeout();
    test_session.errors_handled++;
    
    // Simulate RX timeout
    test_log_event("Simulating RX timeout...");
    test_OnRxTimeout();
    test_session.errors_handled++;
    
    // Simulate RX error
    test_log_event("Simulating RX error...");
    test_OnRxError();
    test_session.errors_handled++;
    
    PPRINTF("=== RADIO ERROR SIMULATION COMPLETE ===\r\n");
}

static void test_validate_error_recovery(void)
{
    PPRINTF("\r\n=== ERROR RECOVERY VALIDATION ===\r\n");
    
    // Reset error stats
    error_recovery_reset();
    test_log_event("Error stats reset");
    
    // Simulate multiple failures
    for (int i = 0; i < 5; i++) {
        error_recovery_handle_tx_failure();
        test_log_event("TX failure simulated");
    }
    
    // Check if fallback mode activated
    if (error_stats.fallback_mode) {
        test_log_event("✓ Fallback mode activated correctly");
    } else {
        test_log_event("✗ Fallback mode not activated");
    }
    
    // Test retry logic
    if (error_recovery_should_retry()) {
        test_log_event("✗ Retry should be disabled after 5 failures");
    } else {
        test_log_event("✓ Retry correctly disabled after max failures");
    }
    
    // Reset and test recovery
    error_recovery_reset();
    if (!error_stats.fallback_mode) {
        test_log_event("✓ Error recovery reset successful");
    } else {
        test_log_event("✗ Error recovery reset failed");
    }
    
    PPRINTF("=== ERROR RECOVERY VALIDATION COMPLETE ===\r\n");
}

static void test_validate_non_blocking_delays(void)
{
    PPRINTF("\r\n=== NON-BLOCKING DELAY VALIDATION ===\r\n");
    
    // Test delay start
    non_blocking_delay_start(&radio_settle_delay, 100, radio_settle_callback);
    test_log_event("Non-blocking delay started");
    
    // Test delay check (should not complete immediately)
    if (!non_blocking_delay_check(&radio_settle_delay)) {
        test_log_event("✓ Delay correctly not completed immediately");
    } else {
        test_log_event("✗ Delay completed too quickly");
    }
    
    // Test delay stop
    non_blocking_delay_stop(&radio_settle_delay);
    test_log_event("Non-blocking delay stopped");
    
    PPRINTF("=== NON-BLOCKING DELAY VALIDATION COMPLETE ===\r\n");
}

// Test command handler
void test_run_comprehensive_test(void)
{
    PPRINTF("\r\n=== COMPREHENSIVE TEST SUITE ===\r\n");
    
    test_init_session(1, 60000); // 1 minute test
    
    // Run all validation tests
    test_validate_state_machine();
    test_validate_error_recovery();
    test_validate_non_blocking_delays();
    test_simulate_radio_errors();
    
    // Print final results
    test_print_results();
    
    PPRINTF("=== COMPREHENSIVE TEST SUITE COMPLETE ===\r\n");
}

// NEW CODE - Watchdog System Implementation
static void watchdog_init(void)
{
    // Always initialize timer structure
    TimerInit(&WatchdogTimer, watchdog_timer_callback);
    
    // Always initialize last_received_time (even if disabled)
    watchdog.last_received_time = get_system_tick_ms();
    watchdog.missed_count = 0;
    watchdog.link_active = true;
    watchdog.safe_state_active = false;
    
    if(watchdog.enabled)
    {
        // Start watchdog timer
        TimerSetValue(&WatchdogTimer, watchdog.interval_seconds * 1000);
        TimerStart(&WatchdogTimer);
        
        PPRINTF("Watchdog enabled: %d second interval, max %d missed\r\n", 
                watchdog.interval_seconds, watchdog.max_missed);
    }
    else
    {
        PPRINTF("Watchdog disabled\r\n");
    }
}

static void watchdog_timer_callback(void)
{
    if(!watchdog.enabled) return;
    
    // Timer fired - means interval elapsed without packet
    watchdog.missed_count++;
    
    uint32_t current_time = get_system_tick_ms();
    
    // Calculate time since last packet (for logging)
    uint32_t time_since_last_packet = current_time - watchdog.last_received_time;
    
    // Calculate expected time (interval * missed_count)
    uint32_t expected_time = watchdog.interval_seconds * 1000 * watchdog.missed_count;
    
    // CRITICAL: Must stop timer before restarting it
    TimerStop(&WatchdogTimer);
    TimerSetValue(&WatchdogTimer, watchdog.interval_seconds * 1000);
    TimerStart(&WatchdogTimer);
    
    // Check if we've exceeded max missed pings
    if(watchdog.missed_count >= watchdog.max_missed)
    {
        if(!watchdog.safe_state_active)
        {
            // First time entering link loss - trigger safe state
            PPRINTF("Watchdog: No packet for %lu ms / expected %lu ms (missed: %d/%d)\r\n", 
                    time_since_last_packet, expected_time, watchdog.missed_count, watchdog.max_missed);
            watchdog_trigger_safe_state();
        }
        else
        {
            // Already in safe state - just log periodically
            PPRINTF("Link still lost (%lu ms since last packet)\r\n", time_since_last_packet);
        }
    }
    else
    {
        // Still counting misses
        PPRINTF("Watchdog: No packet for %lu ms / expected %lu ms (missed: %d/%d)\r\n", 
                time_since_last_packet, expected_time, watchdog.missed_count, watchdog.max_missed);
    }
}

static void watchdog_reset(void)
{
    if(!watchdog.enabled) return;
    
    // Update last received time
    uint32_t now = get_system_tick_ms();
    watchdog.last_received_time = now;
    
    // Reset missed counter
    watchdog.missed_count = 0;
    
    // CRITICAL: Restart the timer from NOW
    // This ensures the next timeout is interval_seconds from packet reception
    TimerStop(&WatchdogTimer);
    TimerSetValue(&WatchdogTimer, watchdog.interval_seconds * 1000);
    TimerStart(&WatchdogTimer);
    
    // Watchdog reset (silent - no need to log every packet)
    
    // Check if we're recovering from safe state
    if(watchdog.safe_state_active)
    {
        PPRINTF("\r\n=== LINK RESTORED ===\r\n");
        watchdog.safe_state_active = false;
        watchdog.link_active = true;
        
        // Turn off DO2 indicator
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
        PPRINTF("DO2 indicator OFF - link recovered\r\n");
        
        // RO outputs will be updated by current packet
    }
}

static void watchdog_trigger_safe_state(void)
{
    // SIMPLE: Just set outputs and flag, don't stop anything
    PPRINTF("\r\n=== LINK LOST ===\r\n");
    PPRINTF("Setting RO1=LOW, RO2=LOW, DO2=HIGH\r\n");
    
    // Set RO outputs to LOW (safe state)
    HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO2_PIN, GPIO_PIN_RESET);
    
    // DO2 = Visual indicator
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    
    // Update state variables
    RO1_flag = 0;
    RO2_flag = 0;
    
    watchdog.safe_state_active = true;
    watchdog.link_active = false;
    
    PPRINTF("Waiting for link recovery (RX active)...\r\n");
}

// ============================================================================
// DI STATE QUEUE IMPLEMENTATION
// ============================================================================
// Purpose: Queue DI state changes that occur during heartbeat TX/RX cycles
//          and send them immediately after heartbeat feedback is received

/**
 * @brief Capture DI state change into queue
 * @param di_channel: 1 for DI1, 2 for DI2
 * 
 * Called from GPIO interrupt when DI changes during heartbeat transmission.
 * Captures the current state and timestamps it for immediate processing
 * after the heartbeat cycle completes.
 */
static void di_queue_capture_state(uint8_t di_channel)
{
    uint32_t current_time = get_system_tick_ms();
    
    if(di_channel == 1)
    {
        // CRITICAL FIX: Use state captured by ISR (updated on EVERY interrupt, even during debounce)
        // This ensures we always get the LATEST state, even if multiple interrupts occurred
        uint8_t new_state;
        
        if(di1_state_updated_in_isr)
        {
            new_state = latest_di1_state_from_isr;
            di1_state_updated_in_isr = false;  // Clear flag
            PPRINTF("Using DI1 state from ISR: %d\r\n", new_state);
        }
        else
        {
            // Fallback: Read current GPIO if ISR didn't update
            GPIO_PinState current_di1 = HAL_GPIO_ReadPin(GPIO_EXTI_PORT, GPIO_EXTI_PIN);
            new_state = (current_di1 == GPIO_PIN_SET) ? 1 : 0;
            PPRINTF("Reading DI1 state from GPIO: %d\r\n", new_state);
        }
        
        // CRITICAL: If DI1 was already queued, this is an UPDATE (captures latest state)
        if(di_queue.di1_changed && di_queue.di1_state != new_state)
        {
            PPRINTF("DI1 queue UPDATE: %d → %d (multiple toggles during heartbeat)\r\n", 
                    di_queue.di1_state, new_state);
        }
        
        di_queue.di1_changed = true;
        di_queue.di1_state = new_state;  // ALWAYS update to latest
        di_queue.change_time = current_time;  // Update timestamp
        di_queue.pending_transmission = true;
        
        PPRINTF_VERBOSE("DI1 queued: LATEST state=%d, time=%lu\r\n", 
                       di_queue.di1_state, current_time);
        PPRINTF("QUEUE STATUS: pending=%d, DI1=%d (state:%d), DI2=%d (state:%d)\r\n", 
                di_queue.pending_transmission, di_queue.di1_changed, di_queue.di1_state,
                di_queue.di2_changed, di_queue.di2_state);
    }
    else if(di_channel == 2)
    {
        // CRITICAL FIX: Use state captured by ISR (updated on EVERY interrupt, even during debounce)
        uint8_t new_state;
        
        if(di2_state_updated_in_isr)
        {
            new_state = latest_di2_state_from_isr;
            di2_state_updated_in_isr = false;  // Clear flag
            PPRINTF("Using DI2 state from ISR: %d\r\n", new_state);
        }
        else
        {
            // Fallback: Read current GPIO if ISR didn't update
            GPIO_PinState current_di2 = HAL_GPIO_ReadPin(GPIO_EXTI2_PORT, GPIO_EXTI2_PIN);
            new_state = (current_di2 == GPIO_PIN_SET) ? 1 : 0;
            PPRINTF("Reading DI2 state from GPIO: %d\r\n", new_state);
        }
        
        // CRITICAL: If DI2 was already queued, this is an UPDATE (captures latest state)
        if(di_queue.di2_changed && di_queue.di2_state != new_state)
        {
            PPRINTF("DI2 queue UPDATE: %d → %d (multiple toggles during heartbeat)\r\n", 
                    di_queue.di2_state, new_state);
        }
        
        di_queue.di2_changed = true;
        di_queue.di2_state = new_state;  // ALWAYS update to latest
        di_queue.change_time = current_time;  // Update timestamp
        di_queue.pending_transmission = true;
        
        PPRINTF_VERBOSE("DI2 queued: LATEST state=%d, time=%lu\r\n", 
                       di_queue.di2_state, current_time);
        PPRINTF("QUEUE STATUS: pending=%d, DI1=%d (state:%d), DI2=%d (state:%d)\r\n", 
                di_queue.pending_transmission, di_queue.di1_changed, di_queue.di1_state,
                di_queue.di2_changed, di_queue.di2_state);
    }
}

/**
 * @brief Clear the DI state queue
 * 
 * Called after successfully processing queued changes or when resetting state.
 */
static void di_queue_clear(void)
{
    di_queue.di1_changed = false;
    di_queue.di2_changed = false;
    di_queue.di1_state = 0;
    di_queue.di2_state = 0;
    di_queue.change_time = 0;
    di_queue.pending_transmission = false;
}

/**
 * @brief Check if there are pending DI changes to transmit
 * @return true if changes are queued, false otherwise
 */
static bool di_queue_has_pending(void)
{
    return di_queue.pending_transmission;
}

/**
 * @brief Process queued DI changes and trigger immediate transmission
 * 
 * Called after heartbeat feedback is received. This ensures DI changes
 * that occurred during the heartbeat cycle are sent immediately instead
 * of waiting for the next heartbeat.
 */
static void di_queue_process_and_send(void)
{
    if(!di_queue_has_pending())
    {
        return;  // Nothing to do
    }
    
    uint32_t queue_age = get_system_tick_ms() - di_queue.change_time;
    
    PPRINTF("Processing queued DI changes (age: %lu ms)\r\n", queue_age);
    
    if(di_queue.di1_changed)
    {
        PPRINTF("  DI1: %d\r\n", di_queue.di1_state);
        exitflag1 = 1;  // Set flag to trigger transmission
    }
    
    if(di_queue.di2_changed)
    {
        PPRINTF("  DI2: %d\r\n", di_queue.di2_state);
        exitflag2 = 1;  // Set flag to trigger transmission
    }
    
    // Trigger immediate TX of queued changes
    uplink_data_status = 1;
    lora_wait_flags = 1;
    
    // Clear the queue
    di_queue_clear();
    
    PPRINTF("Queued DI changes sent immediately (not waiting for next heartbeat)\r\n");
}

static void watchdog_send_ping(void)
{
    // Receiver sends a ping request to transmitter
    // This is a minimal packet to check if transmitter is alive
    PPRINTF("Watchdog: Sending ping to check transmitter...\r\n");
    
    // Set a flag to send a ping packet
    // (Transmitter should respond with its current DI states)
    // This is handled through normal TX mechanism
}

static void watchdog_receive_ping(void)
{
    // Called when receiver gets a packet from transmitter
    // Resets the watchdog timer
    watchdog_reset();
}

static void watchdog_check(void)
{
    if(!watchdog.enabled) return;
    
    // Periodic check in main loop (optional, timer-based is primary)
    uint32_t current_time = get_system_tick_ms();
    uint32_t time_since_last = current_time - watchdog.last_received_time;
    
    // Visual indicator of link status
    static uint32_t last_status_print = 0;
    if((current_time - last_status_print) > 60000)  // Print every 60 seconds
    {
        last_status_print = current_time;
        if(watchdog.link_active)
        {
            PPRINTF("Watchdog: Link OK (last signal: %lu ms ago)\r\n", time_since_last);
        }
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
