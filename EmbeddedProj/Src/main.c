/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <stdlib.h>
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char buffer[256];
int bufferCounter = 0;
int bufferSeeker = 0;
uint8_t secbuffer [2], minbuffer [2], hourbuffer [2];
uint8_t secbufferAlram [2], minbufferAlram [2], hourbufferAlram [2] , DaybufferAlram [2];
uint8_t CR[2];
uint8_t status[2];
uint8_t TUpperByte [2], TLowerByte [2];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t hexToAscii(uint8_t n)//4-bit hex value converted to an ascii character
{
 if (n>=0 && n<=9) n = n + '0';
 else n = n - 10 + 'A';
 return n;
}

void toggleLED(){
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
}
void LED(int state) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void setAlarm(uint8_t hours[2], uint8_t minutes[2], uint8_t secs[2]) {
	HAL_UART_Transmit(&huart2, hours, 2, 100);
	HAL_UART_Transmit(&huart2, minutes, 2, 100);
	HAL_UART_Transmit(&huart2, secs, 2, 100);
	hours[0] = hours[0] - '0';
	hours[1] = hours[1] - '0';
	minutes[0] = minutes[0] - '0';
	minutes[1] = minutes[1] - '0';
	secs[0] = secs[0] - '0';
	secs[1] = secs[1] - '0';
	// seconds
	secbufferAlram[1] = (secs[0] << 4) | secs[1];
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, secbufferAlram, 2, 10);
	// minutes
	minbufferAlram[1] = (minutes[0] << 4) | minutes[1];
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, minbufferAlram, 2, 10);
	// hours
	hourbufferAlram[1] = (hours[0] << 4) | hours[1];
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, hourbufferAlram, 2, 10);
	
	DaybufferAlram[1] = 0x80; //data to put in register 10000000 --> AIM4 = 1
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, DaybufferAlram, 2, 10);
}

void setTime(uint8_t hours[2], uint8_t minutes[2], uint8_t secs[2]){
	// seconds
	hours[0] = hours[0] - '0';
	hours[1] = hours[1] - '0';
	minutes[0] = minutes[0] - '0';
	minutes[1] = minutes[1] - '0';
	secs[0] = secs[0] - '0';
	secs[1] = secs[1] - '0';
	secbuffer[1] = (secs[0] << 4) | secs[1];
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, secbuffer, 2, 10);
	// minutes
	minbuffer[1] = (minutes[0] << 4) | minutes[1];
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, minbuffer, 2, 10);
	// hours
	hourbuffer[1] = (hours[0] << 4) | hours[1];
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, hourbuffer, 2, 10);
}

void getAlarm(){
	//Alarm Status Read
	uint8_t out[] = {'A',':',0,'\r','\n'};
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, status, 1, 10);
	HAL_I2C_Master_Receive(&hi2c1, 0xD1, status+1, 1, 10);
	uint8_t A1F = hexToAscii(status[1] & 0x01 );
	out[2] = A1F;
	HAL_UART_Transmit(&huart1,out, sizeof(out), 10);
}

void getTime(){
	uint8_t out[] = {0,0,':',0,0,':',0,0,' ','A',':',0,'\r','\n'};
		//Secs Read
		HAL_I2C_Master_Transmit(&hi2c1, 0xD0, secbuffer, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1, 0xD1, secbuffer+1, 1, 10);
		out[6] = hexToAscii(secbuffer[1] >> 4 );
		out[7] = hexToAscii(secbuffer[1] & 0x0F );
	
		//Minutes Read
		HAL_I2C_Master_Transmit(&hi2c1, 0xD0, minbuffer, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1, 0xD1, minbuffer+1, 1, 10);
		out[3] = hexToAscii(minbuffer[1] >> 4 );
		out[4] = hexToAscii(minbuffer[1] & 0x0F );
		
		//Hours Read
		HAL_I2C_Master_Transmit(&hi2c1, 0xD0, hourbuffer, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1, 0xD1, hourbuffer+1, 1, 10);
		out[0] = hexToAscii((hourbuffer[1] >> 4) & 0x01);
		out[1] = hexToAscii(hourbuffer[1] & 0x0F);
		
		//Alarm Status Read
		HAL_I2C_Master_Transmit(&hi2c1, 0xD0, status, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1, 0xD1, status+1, 1, 10);
		uint8_t A1F = hexToAscii(status[1] & 0x01 );
		out[12] = A1F;
		// transmit time to 
		HAL_UART_Transmit(&huart1,out, sizeof(out), 10);
}

void getTemperature() {
	char fraction[]={'.','0','0',' ','C','\r','\n'};
	char temp[2];
	char out1[3];
	char negSign = '-';
	
	//Initiate Conversion
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, CR, 1, 10);
	HAL_I2C_Master_Receive(&hi2c1, 0xD0, CR+1, 1, 10);
	CR[1] |= 0x20; // 00100000
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, CR, 2, 10);
	do {
		HAL_I2C_Master_Transmit(&hi2c1, 0xD0, status, 1, 10);
		HAL_I2C_Master_Receive(&hi2c1, 0xD0, status+1, 1, 10);
		if ((status[1] & 0x04) == 0x00 ){	 // xxxxx?xx
				HAL_I2C_Master_Transmit(&hi2c1, 0xD0, TUpperByte, 1, 10);
				HAL_I2C_Master_Receive(&hi2c1, 0xD0, TUpperByte+1, 1, 10);
				HAL_I2C_Master_Transmit(&hi2c1, 0xD0, TLowerByte, 1, 10);
				HAL_I2C_Master_Receive(&hi2c1, 0xD0, TLowerByte+1, 1, 10);
			
				temp[1] = hexToAscii((TUpperByte[1] & 0x0F));
				temp[0] = hexToAscii((TUpperByte[1] << 4) & 0x07);
				unsigned int x = strtol(temp, NULL, 16);
				sprintf(out1,"%u", x);
			
				switch(TLowerByte[1]) {
					case 0x00:
						fraction[1] = '0';
						fraction[2] = '0';
						break;
					case 0x40:
						fraction[1] = '2';
						fraction[2] = '5';
						break;
					case 0x80:
						fraction[1] = '5';
						fraction[2] = '0';
						break;
					case 0xc0:
						fraction[1] = '7';
						fraction[2] = '5';
						break;
				}
				
				if((TUpperByte[1] & 0x80) != 0x00)
						HAL_UART_Transmit(&huart1,(uint8_t*) &negSign, 1, 100);
				HAL_UART_Transmit(&huart1,(uint8_t*) out1, strlen(out1), 100);
				HAL_UART_Transmit(&huart1,(uint8_t*) fraction, strlen(fraction), 100);
			}
		} while ((status[1] & 0x04) != 0x00);
}

void ping() {
	char msg[] = "PONG!";
	HAL_UART_Transmit(&huart1,(uint8_t*) msg, strlen(msg), 100);
}

void listen(){
	if(bufferSeeker < bufferCounter && buffer[(bufferCounter-1) % 256] == '\0') {
		if(strncmp(&buffer[bufferSeeker%256],"$LG",3) == 0){
			if(strlen(&buffer[bufferSeeker%256]) == 3)
				toggleLED();
			else
				LED(strlen(&buffer[bufferSeeker%256]) == 6);
		} else if(strncmp(&buffer[bufferSeeker%256],"$G Time",7) == 0) {
			getTime();
		} else if(strncmp(&buffer[bufferSeeker%256],"$G Alarm",8) == 0) {
			getAlarm();
		} else if(strncmp(&buffer[bufferSeeker%256],"$G Temp",7) == 0) {
			getTemperature();
		} else if(strncmp(&buffer[bufferSeeker%256],"$P STM",6) == 0) {
			ping();
		} else if(strncmp(&buffer[bufferSeeker%256],"$S Time",7) == 0) {
			setTime(
			(uint8_t*)&buffer[(bufferSeeker+8) % 256],
			(uint8_t*)&buffer[(bufferSeeker+11) % 256],
			(uint8_t*)&buffer[(bufferSeeker+14) % 256]
			);
		} else if(strncmp(&buffer[bufferSeeker%256], "$S Alarm",8) == 0) {
			setAlarm(
			(uint8_t*)&buffer[(bufferSeeker+9) % 256],
			(uint8_t*)&buffer[(bufferSeeker+12) % 256],
			(uint8_t*)&buffer[(bufferSeeker+15) % 256]
			);
		}
		bufferSeeker += strlen(&buffer[bufferSeeker%256]) + 1;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	secbuffer[0] = 0x00; //register address
	minbuffer[0] = 0x01; //register address
	hourbuffer[0] = 0x02; //register address
	secbufferAlram[0] = 0x07; //register address
	minbufferAlram[0] = 0x08; //register address
	hourbufferAlram[0] = 0x09; //register address
	DaybufferAlram[0] = 0x0A; //register address
	CR[0]	= 0x0E;
	status[0]	= 0x0F;
	TUpperByte[0] = 0x11;
	TLowerByte[0] = 0x12;
	
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, CR, 1, 10);
	HAL_I2C_Master_Receive(&hi2c1, 0xD0, CR+1, 1, 10);
	CR[1] |= 0x05; // 00000101
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, CR, 2, 10);
	
	 
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, status, 1, 10);
	HAL_I2C_Master_Receive(&hi2c1, 0xD0, status+1, 1, 10);
	status[1] &= 0xFC; // xxxxxx00
	HAL_I2C_Master_Transmit(&hi2c1, 0xD0, status, 2, 10);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		listen();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10909CEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
