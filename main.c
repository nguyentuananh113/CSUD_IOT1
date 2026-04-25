	/* USER CODE BEGIN Header */
	/**
		******************************************************************************
		* @file           : main.c
		* @brief          : Main program body
		******************************************************************************
		* @attention
		*
		* Copyright (c) 2026 STMicroelectronics.
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
	#include "i2c-lcd.h"
	#include <stdio.h>


	char buffer[100];
	char rec;
	uint8_t pass_detected = 0;
	uint8_t tt=0;
	int product_A = 0;
	int product_B = 0;
	int total = 0;
	
int order_count = 0;
int item_in_order = 0;

	uint8_t buzzer_triggered = 0;
	uint32_t buzzer_time = 0;

	#define BUZZER_PORT GPIOA
	#define BUZZER_PIN  GPIO_PIN_5   
	#define TIMEOUT_MS  5000 

	uint8_t system_enable = 1; 
	uint8_t servo_state = 0;
	uint32_t servo_time = 0;

	uint8_t ir_low, ir_high, ir_pass;
	uint8_t detected = 0; 
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

	TIM_HandleTypeDef htim2;

	UART_HandleTypeDef huart2;

	/* USER CODE BEGIN PV */

	/* USER CODE END PV */

	/* Private function prototypes -----------------------------------------------*/
	void SystemClock_Config(void);
	static void MX_GPIO_Init(void);
	static void MX_I2C1_Init(void);
	static void MX_TIM2_Init(void);
	static void MX_USART2_UART_Init(void);
	/* USER CODE BEGIN PFP */

	/* USER CODE END PFP */

	/* Private user code ---------------------------------------------------------*/
	/* USER CODE BEGIN 0 */

	void send_product_data(UART_HandleTypeDef *huart, int A, int B, int T,int order)
	{
			char buff[100];

			int len = snprintf(buff, sizeof(buff),
						"in=%d;out=%d;total=%d;ORDER=%d\r\n",
								A, B, T, order_count);
			if (len > 0)
			{
					HAL_UART_Transmit(huart, (uint8_t*)buff, len, HAL_MAX_DELAY);
			}
	}


	void LCD_Display(void)
	{
				if(tt == 1)
				{
				lcd_clear();
				lcd_put_cur(0,3);
				lcd_send_string("Product A");
			}
				else if(tt == 2)
			{
				lcd_clear();
				lcd_put_cur(0,3);
				lcd_send_string("Product B");
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
		MX_I2C1_Init();
		MX_TIM2_Init();
		MX_USART2_UART_Init();
		/* USER CODE BEGIN 2 */
		HAL_UART_Receive_IT(&huart2, (uint8_t*)&rec, 1);
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
		lcd_init();
		lcd_clear();
		
		
	 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,700);
	//////////////////////////
			 

		/* USER CODE END 2 */

		/* Infinite loop */
		/* USER CODE BEGIN WHILE */
		while (1)
		{

			/* USER CODE END WHILE */

			/* USER CODE BEGIN 3 */
				 
		 ir_low  = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
			ir_high = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9);
			ir_pass = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);              


		 
			if(ir_low == 0 && detected == 0)
			{	HAL_Delay(20);
			total++;
			item_in_order++;
			if(item_in_order >= 10)
{
    order_count++;        // +1 đơn hàng
    item_in_order = 0;    // reset đơn hiện tại

    buzzer_triggered = 1;
    buzzer_time = HAL_GetTick();
}
			

			if(ir_high == 0)
			{
					product_A++;
					tt = 1;

					// mở servo
					__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1700);
				
					servo_state = 1;
			}
			else
			{
					product_B++;
					tt = 2;
			}
			detected = 1;
	}

	// reset IR1
	if(ir_low == 1)
	{
			detected = 0;
	}

	// IR3 đóng servo
	if(ir_pass == 0 && pass_detected == 0)
	{
			pass_detected = 1;

			if(servo_state == 1)
			{
					__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 700);
					servo_state = 0;
			}
	}

	if(ir_pass == 1)
	{
			pass_detected = 0;
	}
			// Hien thi LCD
			lcd_put_cur(1,0);
			sprintf(buffer, "A=%dB=%d TONG=%d", product_A, product_B, total);
			lcd_send_string(buffer);
		 HAL_Delay(70);	
					LCD_Display();


	static uint32_t last_send = 0;

	if(HAL_GetTick() - last_send >= 1000)
	{
			last_send = HAL_GetTick();
			send_product_data(&huart2, product_A, product_B, total, order_count);
	}
	if(buzzer_triggered == 1)
	{
			if(HAL_GetTick() - buzzer_time < 200) {
					HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
			}
			else
			{
					HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
					buzzer_triggered = 0;  
				
			}
	}
	}
	}
		/* USER CODE END 3 */


	/**
		* @brief System Clock Configuration
		* @retval None
		*/
	void SystemClock_Config(void)
	{
		RCC_OscInitTypeDef RCC_OscInitStruct = {0};
		RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

		/** Initializes the RCC Oscillators according to the specified parameters
		* in the RCC_OscInitTypeDef structure.
		*/
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
		RCC_OscInitStruct.HSIState = RCC_HSI_ON;
		RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
		RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
		hi2c1.Init.ClockSpeed = 100000;
		hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
		hi2c1.Init.OwnAddress1 = 0;
		hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		hi2c1.Init.OwnAddress2 = 0;
		hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
		if (HAL_I2C_Init(&hi2c1) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN I2C1_Init 2 */

		/* USER CODE END I2C1_Init 2 */

	}

	/**
		* @brief TIM2 Initialization Function
		* @param None
		* @retval None
		*/
	static void MX_TIM2_Init(void)
	{

		/* USER CODE BEGIN TIM2_Init 0 */

		/* USER CODE END TIM2_Init 0 */

		TIM_ClockConfigTypeDef sClockSourceConfig = {0};
		TIM_MasterConfigTypeDef sMasterConfig = {0};
		TIM_OC_InitTypeDef sConfigOC = {0};

		/* USER CODE BEGIN TIM2_Init 1 */

		/* USER CODE END TIM2_Init 1 */
		htim2.Instance = TIM2;
		htim2.Init.Prescaler = 63;
		htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim2.Init.Period = 19999;
		htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
		htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
		{
			Error_Handler();
		}
		sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
		if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
		{
			Error_Handler();
		}
		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
		{
			Error_Handler();
		}
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = 0;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE BEGIN TIM2_Init 2 */

		/* USER CODE END TIM2_Init 2 */
		HAL_TIM_MspPostInit(&htim2);

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
		huart2.Init.BaudRate = 9600;
		huart2.Init.WordLength = UART_WORDLENGTH_8B;
		huart2.Init.StopBits = UART_STOPBITS_1;
		huart2.Init.Parity = UART_PARITY_NONE;
		huart2.Init.Mode = UART_MODE_TX_RX;
		huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart2.Init.OverSampling = UART_OVERSAMPLING_16;
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
		/* USER CODE BEGIN MX_GPIO_Init_1 */

		/* USER CODE END MX_GPIO_Init_1 */

		/* GPIO Ports Clock Enable */
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/*Configure GPIO pin Output Level */
		
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_SET);

		/*Configure GPIO pins : PA5 PA6 */
		GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/*Configure GPIO pins : PA8 PA9 PA10 */
		GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
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
	#ifdef USE_FULL_ASSERT
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
