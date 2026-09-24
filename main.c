/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define RELAY_PIN	GPIO_PIN_5
#define RELAY_PORT	GPIOA

//	uint8_t tx_buffer[] = "I'm Kutyn\n";
//	uint8_t rx_buffer[1];
	uint8_t is_sending = 0;
	uint8_t brightness = 0;

	uint8_t tx_buffer[20];
	uint8_t rx_buffer[20];
	uint8_t tx_data;
	uint8_t rx_data;
//	char rx_buffer[20];
	uint8_t rx_index = 0;
	uint8_t tx_index = 0;
	int pwm_value = 0;

	uint16_t readValue[1];
	uint16_t adc = 0;

	ADC_ChannelConfTypeDef sConfigPrivate = {0};


// Fungsi untuk mengosongkan buffer tx_buffer
void clear_tx_buffer(void)
{
	memset(tx_buffer, 0, sizeof(tx_buffer));  // Mengisi seluruh tx_buffer dengan 0
}


//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//	if (huart->Instance == USART3)
//	{
//		adc = (uint16_t)readValue[0];
////			  sprintf((char*)tx_buffer, "ADC Value: %u\n", adc);
//		sprintf((char*)tx_buffer, "ADC Value: %d\n", adc);
//		HAL_UART_Transmit_IT(&huart3, tx_buffer, strlen((char*)tx_buffer));
////		if(tx_data == '\n')
////		{
////			tx_buffer[tx_index] = '\0';
////		}
////		rx_index = 0;
////		memset((char*)rx_buffer, 0, sizeof(rx_buffer));
////
////		HAL_UART_Transmit_DMA(&huart3, tx_buffer, strlen((char*)tx_buffer));
////		HAL_UART_Transmit_DMA(&huart3, tx_buffer, 20);
////		is_sending = 0;
//
////		// Setiap selesai kirim, reset buffer agar data tidak bertabrakan
////		clear_tx_buffer();
//	}
//}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
    	if(rx_data == '\n')
    	{  // Jika data akhir baris (newline)
    		rx_buffer[rx_index] = '\0';   // Akhiri string dengan null terminator

    		if(strncmp((char *)rx_buffer, "PWM_", 4) == 0)
    		{
//    		pwm_value = atoi(rx_buffer);  // Konversi string ke integer
    			pwm_value = atoi((char*)rx_buffer + 4);

    			if (pwm_value >= 0 && pwm_value <= 100)
    			{
    				// Atur nilai PWM, map 0-100 ke 0-999 (dengan timer 10-bit, 0-999)
    				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value *10);
    			}
    		}

    		// Cek perintah relay ON
    		else if (strncmp((char*)rx_buffer, "relay ON", 8) == 0)
    		{
    			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  // Nyalakan relay (misalnya PA5)
    			sprintf((char*)tx_buffer, "Relay ON\n");
//    			HAL_UART_Transmit_DMA(&huart3, tx_buffer, strlen((char*)tx_buffer));
    			HAL_UART_Transmit_IT(&huart3, tx_buffer, strlen((char*)tx_buffer));
//    			sendRelayStatus(1);
    		}
    		// Cek perintah relay OFF
    		else if (strncmp((char*)rx_buffer, "relay OFF", 9) == 0)
    		{
    			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  // Matikan relay
    			sprintf((char*)tx_buffer, "Relay OFF\n");
    			HAL_UART_Transmit_IT(&huart3, tx_buffer, strlen((char*)tx_buffer));
//    			sendRelayStatus(0);
    		}

    		// Reset buffer
    		rx_index = 0;
    		memset((char*)rx_buffer, 0, sizeof(rx_buffer));

//    		HAL_UART_Receive_DMA(&huart3, &rx_data, 1);
    		HAL_UART_Receive_IT(&huart3, &rx_data, 1);
    	}
    	else
    	{
//    		// Tambahkan karakter ke buffer
//    		rx_buffer[rx_index++] = rx_data;
    		// Tambahkan karakter ke buffer

    		if (rx_index < sizeof(rx_buffer) - 1)  // Pastikan buffer tidak overflow
    		{
    			rx_buffer[rx_index++] = rx_data;
    		}
    	}

    	// Menerima byte berikutnya
    	HAL_UART_Receive_IT(&huart3, &rx_data, 1);


//    	if(HAL_UART_Receive_DMA(&huart3, rx_buffer, 1) == HAL_OK)
//        if (rx_buffer[0] == '1')  // Jika menerima karakter '1'
//        {
//        	HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
//            // Stop pengiriman DMA
//            HAL_UART_DMAStop(&huart3);
//            is_sending = 0;  // Mengubah status menjadi tidak mengirim
//        }
//        else if (rx_buffer[0] == '2')  // Jika menerima karakter '2'
//        {
//        	HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
//            // Mulai pengiriman DMA kembali
//            HAL_UART_Transmit_DMA(&huart3, tx_buffer, 10);
//            is_sending = 1;  // Mengubah status menjadi sedang mengirim
//        }
//
//        // Memulai kembali DMA untuk menerima data
//        HAL_UART_Receive_DMA(&huart3, rx_buffer, 1);
    }
}


//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
//{
//    if(hadc->Instance == ADC1)
//    {

//		// Ambil nilai dari ADC yang telah selesai dikonversi
//        adc = (uint16_t)readValue[0];
//
//        // Anda bisa memproses data di sini jika diperlukan
//        // Misalnya mencetak data atau mengirimkannya melalui UART
//        sprintf((char*)tx_buffer, "ADC Value: %d\n", adc);
//        HAL_UART_Transmit_DMA(&huart3, tx_buffer, strlen((char*)tx_buffer));
//
//        // Restart ADC dengan DMA untuk konversi selanjutnya
//        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)readValue, 1);

//        // Callback saat konversi ADC selesai
//        // Anda bisa membaca adcValue di sini
//    	adc = (uint16_t)readValue[0];
//    	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)readValue, 1);
//    }
//}


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
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

//  HAL_UART_Receive_DMA(&huart3, rx_buffer, 1);
//  HAL_UART_Transmit_DMA(&huart3, tx_buffer, 20);
//  is_sending = 1;

  HAL_UART_Transmit_IT(&huart3, tx_buffer, strlen((char*)tx_buffer));
//  HAL_UART_Transmit_DMA(&huart3, tx_buffer, strlen((char*)tx_buffer));
  HAL_UART_Receive_IT(&huart3, &rx_data, 1);
//  HAL_UART_Receive_DMA(&huart3, &rx_data, 1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)readValue, 1);


  adc = (uint16_t)readValue[0];
  sprintf((char*)tx_buffer, "ADC Value: %d\n", adc);
//  HAL_UART_Transmit_DMA(&huart3, tx_buffer, strlen((char*)tx_buffer));
  HAL_UART_Transmit_IT(&huart3, tx_buffer, strlen((char*)tx_buffer));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//	  adc = (uint16_t)readValue[0];
//	  sprintf((char*)tx_buffer, "ADC Value: %u\n", adc);
//	  HAL_UART_Transmit_DMA(&huart3, tx_buffer, strlen((char*)tx_buffer));
	  HAL_Delay(1000);

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
