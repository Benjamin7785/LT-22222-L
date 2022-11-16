/*******************************************************************************
 * @file    command.c
 * @author  MCD Application Team
 * @version V1.1.4
 * @date    10-July-2018
 * @brief   main command driver dedicated to command AT
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
#include <stdlib.h>
#include "at.h"
#include "hw.h"
#include "command.h"
#include "lora.h"
#include "delay.h"

/* comment the following to have help message */
/* #define NO_HELP */
/* #define NO_KEY_ADDR_EUI */

/* Private typedef -----------------------------------------------------------*/
/**
 * @brief  Structure defining an AT Command
 */
struct ATCommand_s {
  const char *string;                       /*< command string, after the "AT" */
  const int size_string;                    /*< size of the command string, not including the final \0 */
  ATEerror_t (*get)(const char *param);     /*< =? after the string to get the current value*/
  ATEerror_t (*set)(const char *param);     /*< = (but not =?\0) after the string to set a value */
  ATEerror_t (*run)(const char *param);     /*< \0 after the string - run the command */
#if !defined(NO_HELP)
  const char *help_string;                  /*< to be printed when ? after the string */
#endif
};

/* Private define ------------------------------------------------------------*/
#define CMD_SIZE 128
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * @brief  Array corresponding to the description of each possible AT Error
 */
static const char *const ATError_description[] =
{
  "\r\nOK\r\n",                     /* AT_OK */
  "\r\nAT_ERROR\r\n",               /* AT_ERROR */
  "\r\nAT_PARAM_ERROR\r\n",         /* AT_PARAM_ERROR */
  "\r\nERROR:Not in Range\r\n",     /* AT_PARAM_NOT in Range */	
  "\r\nERROR:Run AT+FDR first\r\n", /* AT_PARAM_FDR */		
  "\r\nAT_BUSY_ERROR\r\n",          /* AT_BUSY_ERROR */
  "\r\nAT_TEST_PARAM_OVERFLOW\r\n", /* AT_TEST_PARAM_OVERFLOW */
  "\r\nAT_NO_NETWORK_JOINED\r\n",   /* AT_NO_NET_JOINED */
  "\r\nAT_RX_ERROR\r\n",            /* AT_RX_ERROR */
  "\r\nerror unknown\r\n",          /* AT_MAX */
};

/**
 * @brief  Array of all supported AT Commands
 */
static const struct ATCommand_s ATCommand[] =
{
  {
    .string = AT_RESET,
    .size_string = sizeof(AT_RESET) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RESET ": Trig a reset of the MCU\r\n",
#endif
    .get = at_return_error,
    .set = at_return_error,
    .run = at_reset,
  },
	
	{
	  .string = AT_FDR,
    .size_string = sizeof(AT_FDR) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_FDR ": Reset Parameters to Factory Default, Keys Reserve\r\n",
#endif
    .get = at_return_error,
    .set = at_return_error,
    .run = at_FDR,
  },
  
  {
    .string = AT_FCU,
    .size_string = sizeof(AT_FCU) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_FCU ": Get or Set the Frame Counter Uplink\r\n",
#endif
    .get = at_UplinkCounter_get,
    .set = at_UplinkCounter_set,
    .run = at_return_error,
  },

  {
    .string = AT_FCD,
    .size_string = sizeof(AT_FCD) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_FCD ": Get or Set the Frame Counter Downlink\r\n",
#endif
    .get = at_DownlinkCounter_get,
    .set = at_DownlinkCounter_set,
    .run = at_return_error,
  },
  
  {
    .string = AT_TXP,
    .size_string = sizeof(AT_TXP) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TXP ": Get or Set the Transmit Power\r\n",
#endif
    .get = at_TransmitPower_get,
    .set = at_TransmitPower_set,
    .run = at_return_error,
  },

  {
    .string = AT_SYNC,
    .size_string = sizeof(AT_SYNC) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_SYNC ": Get or Set the sync word\r\n",
#endif
    .get = at_syncword_get,
    .set = at_syncword_set,
    .run = at_return_error,
  },

  {
    .string = AT_PMB,
    .size_string = sizeof(AT_PMB) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_PMB ": Get or Set the preamble length\r\n",
#endif
    .get = at_preamble_get,
    .set = at_preamble_set,
    .run = at_return_error,
  },

	{
	  .string = AT_TXCHS,
    .size_string = sizeof(AT_TXCHS) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TXCHS ": Get or Set Frequency (Unit: Hz) for TX Single Channel Mode\r\n",
#endif
    .get = at_txCHS_get,
    .set = at_txCHS_set,
    .run = at_return_error,
	},
	
  {
    .string = AT_TXSF,
    .size_string = sizeof(AT_TXSF) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TXSF ": Get or Set the TX Spreading factor\r\n",
#endif
    .get = at_txspreading_get,
    .set = at_txspreading_set,
    .run = at_return_error,
  },

	{
	  .string = AT_RXCHS,
    .size_string = sizeof(AT_RXCHS) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RXCHS ": Get or Set Frequency (Unit: Hz) for RX Single Channel Mode\r\n",
#endif
    .get = at_rxCHS_get,
    .set = at_rxCHS_set,
    .run = at_return_error,
	},
	
  {
    .string = AT_RXSF,
    .size_string = sizeof(AT_RXSF) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_RXSF ": Get or Set the RX Spreading factor\r\n",
#endif
    .get = at_rxspreading_get,
    .set = at_rxspreading_set,
    .run = at_return_error,
  },

  {
    .string = AT_BW,
    .size_string = sizeof(AT_BW) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_BW ": Get or Set the bandwidth\r\n",
#endif
    .get = at_bandwidth_get,
    .set = at_bandwidth_set,
    .run = at_return_error,
  },
	
  {
    .string = AT_CR,
    .size_string = sizeof(AT_CR) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_CR ": Get or Set the codingrate\r\n",
#endif
    .get = at_codingrate_get,
    .set = at_codingrate_set,
    .run = at_return_error,
  },
	
	{
    .string = AT_TDC,
    .size_string = sizeof(AT_TDC) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TDC ": Get or set the application data transmission interval in ms\r\n",
#endif
    .get = at_TDC_get,
    .set = at_TDC_set,
    .run = at_return_error,
  },

		{
	  .string = AT_TRIG1,
    .size_string = sizeof(AT_TRIG1) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TRIG1 ":Get or Set the trigger type,triggering time\r\n",
#endif
    .get = at_trig1_get,
    .set = at_trig1_set,
    .run = at_return_error,
	},

		{
	  .string = AT_TRIG2,
    .size_string = sizeof(AT_TRIG2) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_TRIG2 ":Get or Set the trigger type,triggering time\r\n",
#endif
    .get = at_trig2_get,
    .set = at_trig2_set,
    .run = at_return_error,
	},
 
		{
	  .string = AT_GROUPMOD,
    .size_string = sizeof(AT_GROUPMOD) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_GROUPMOD ":Get or Set the Group mode\r\n",
#endif
    .get = at_groupmode_get,
    .set = at_groupmode_set,
    .run = at_return_error,
	},

		{
	  .string = AT_GROUPID,
    .size_string = sizeof(AT_GROUPID) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_GROUPID ":Get or Set the Group ID\r\n",
#endif
    .get = at_groupid_get,
    .set = at_groupid_set,
    .run = at_return_error,
	},

		{
	  .string = AT_DI1TODO1,
    .size_string = sizeof(AT_DI1TODO1) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DI1TODO1 ":Get or Set the level of the DI1 pin is mapped to the DO1 level\r\n",
#endif
    .get = at_pinDI1topinDO1_get,
    .set = at_pinDI1topinDO1_set,
    .run = at_return_error,
	},

		{
	  .string = AT_DI1TORO1,
    .size_string = sizeof(AT_DI1TORO1) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DI1TORO1 ":Get or Set the level of the DI1 pin is mapped to the RO1 level\r\n",
#endif
    .get = at_pinDI1topinRO1_get,
    .set = at_pinDI1topinRO1_set,
    .run = at_return_error,
	},

		{
	  .string = AT_DI2TODO2,
    .size_string = sizeof(AT_DI2TODO2) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DI2TODO2 ":Get or Set the level of the DI2 pin is mapped to the DO2 level\r\n",
#endif
    .get = at_pinDI2topinDO2_get,
    .set = at_pinDI2topinDO2_set,
    .run = at_return_error,
	},

		{
	  .string = AT_DI2TORO2,
    .size_string = sizeof(AT_DI2TORO2) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DI2TORO2 ":Get or Set the level of the DI2 pin is mapped to the RO2 level\r\n",
#endif
    .get = at_pinDI2topinRO2_get,
    .set = at_pinDI2topinRO2_set,
    .run = at_return_error,
	},

		{
	  .string = AT_DOROSAVE,
    .size_string = sizeof(AT_DOROSAVE) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DOROSAVE ":Get or Set the read saved DORO state after reset\r\n",
#endif
    .get = at_dorosave_get,
    .set = at_dorosave_set,
    .run = at_return_error,
	},

		{
	  .string = AT_DI2SLEEP,
    .size_string = sizeof(AT_DI2SLEEP) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_DI2SLEEP ":Get or Set LT into sleep mode by DI2\r\n",
#endif
    .get = at_di2sleep_get,
    .set = at_di2sleep_set,
    .run = at_return_error,
	},
	
  {
    .string = AT_SEND,
    .size_string = sizeof(AT_SEND) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_SEND ": Send text data along\r\n",
#endif
    .get = at_return_error,
    .set = at_Send,
    .run = at_return_error,
  },
	
  {
    .string = AT_VER,
    .size_string = sizeof(AT_VER) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_VER ": Get current image version and Frequency Band\r\n",
#endif
    .get = at_version_get,
    .set = at_return_error,
    .run = at_return_error,
  },
	
	{
	  .string = AT_CFG,
    .size_string = sizeof(AT_CFG) - 1,
#ifndef NO_HELP
    .help_string = "AT"AT_CFG ": Print all configurations\r\n",
#endif
    .get = at_return_error,
    .set = at_return_error,
		.run = at_CFG_run,
	},			
};


/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Print a string corresponding to an ATEerror_t
 * @param  The AT error code
 * @retval None
 */
static void com_error(ATEerror_t error_type);

/**
 * @brief  Parse a command and process it
 * @param  The command
 * @retval None
 */
static void parse_cmd(const char *cmd);

/* Exported functions ---------------------------------------------------------*/
static void CMD_GetChar( uint8_t* rxChar);
static char command[CMD_SIZE];
static unsigned i = 0;  
static FlagStatus IsCharReceived=RESET;  

void CMD_Init(void)
{
  vcom_ReceiveInit( CMD_GetChar );
  IsCharReceived=RESET;
}

static void CMD_GetChar( uint8_t* rxChar)
{
  command[i] = *rxChar;
  IsCharReceived=SET;
}

void CMD_Process(void)
{
/* Process all commands */
  if (IsCharReceived==SET)
  {
    //ENTER_CRITICAL
    IsCharReceived=RESET;
    //EXIT CRITICAL
#if 0 /* echo On    */
  PRINTF("%c", command[i]);
#endif

    if (command[i] == AT_ERROR_RX_CHAR)
    {
      i = 0;
      com_error(AT_RX_ERROR);
    }
    else if ((command[i] == '\r') || (command[i] == '\n'))
    {
      if (i != 0)
      {
        command[i] = '\0';
				i = 0;	
				parse_cmd(command);
      }
    }
    else if (i == (CMD_SIZE - 1))
    {
      i = 0;
      com_error(AT_TEST_PARAM_OVERFLOW);
    }
    else
    {
      i++;
    }
  }
}

/* Private functions ---------------------------------------------------------*/

static void com_error(ATEerror_t error_type)
{
  if (error_type > AT_MAX)
  {
    error_type = AT_MAX;
  }
  AT_PRINTF(ATError_description[error_type]);
}


static void parse_cmd(const char *cmd)
{
	uint8_t store_config_status=1;
  ATEerror_t status = AT_OK;
  const struct ATCommand_s *Current_ATCommand;
  int i;

  if ((cmd[0] != 'A') || (cmd[1] != 'T'))
  {
    status = AT_ERROR;
  }
  else
  if (cmd[2] == '\0')
  {
    /* status = AT_OK; */
  }
  else
  if (cmd[2] == '?')
  {
#ifdef NO_HELP
#else
    PPRINTF("AT+<CMD>?        : Help on <CMD>\r\n"
              "AT+<CMD>         : Run <CMD>\r\n"
              "AT+<CMD>=<value> : Set the value\r\n"
              "AT+<CMD>=?       : Get the value\r\n");
    for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++)
    {
      PPRINTF(ATCommand[i].help_string);
    }
#endif
  }
  else
  {
    /* point to the start of the command, excluding AT */
    status = AT_ERROR;
    cmd += 2;
    for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++)
    {
      if (strncmp(cmd, ATCommand[i].string, ATCommand[i].size_string) == 0)
      {
        Current_ATCommand = &(ATCommand[i]);
        /* point to the string after the command to parse it */
        cmd += Current_ATCommand->size_string;

        /* parse after the command */
        switch (cmd[0])
        {
          case '\0':    /* nothing after the command */
            status = Current_ATCommand->run(cmd);
            break;
          case '=':
            if ((cmd[1] == '?') && (cmd[2] == '\0'))
            {
              status = Current_ATCommand->get(cmd + 1);
            }
            else
            {
              status = Current_ATCommand->set(cmd + 1);
							cmd=Current_ATCommand->string;
							
							if(strcmp(cmd,AT_SEND)==0)
							{
								store_config_status=0;
							}		
							
							if(store_config_status>0)
							{
								if(status==AT_OK)
								{
									EEPROM_Store_Config();
								}
							}
            }
            break;
          case '?':
#ifndef NO_HELP
            AT_PRINTF(Current_ATCommand->help_string);
#endif
            status = AT_OK;
            break;
          default:
            /* not recognized */
            break;
        }

        /* we end the loop as the command was found */
        break;
		 }
    }
  }

  com_error(status);
}

uint8_t printf_all_config(void)
{
	char *cmd;
	for (int j = 0; j< (sizeof(ATCommand) / sizeof(struct ATCommand_s)); j++)
	{
		if((ATCommand[j].get)!=at_return_error)
		{
			PPRINTF("AT%s=",ATCommand[j].string);
			ATCommand[j].get(( char *)cmd);	
			HAL_Delay(50);				
		}
	}
	
	return 1;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
