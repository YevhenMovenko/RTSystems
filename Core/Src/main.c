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
#include <stdio.h>

//#include "i2c_addrScaner.h"
#define I2C_REQUEST_WRITE 0x00 //I2C scanner

#ifndef USE_FULL_LL_DRIVER
#define USE_FULL_LL_DRIVER
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

uint16_t g =0;
char str[16];
int8_t rx_buffer[16];

//uint8_t data[]="Hello World\n";
uint8_t dataStartBlock1[] = "Block #1\n";
uint8_t dataStartBlock2[] = "Block #2\n";

uint8_t dataCl[]="\r";
uint8_t nums = 123;
uint8_t numarray[4];

uint8_t TxData[10240];
int isSent =1;

int countloop = 0;
int coutinterrupt = 0;



UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

//++++ I2C scanner
I2C_HandleTypeDef hi2c1;

char CDC_tx_buff[64];
char CDC_rx_buff[8];
uint8_t CDC_rx_flag = 0;
uint32_t I2C_speed[4] = { 100000, 200000, 300000, 400000 };

uint8_t countLoop = 0;

//extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	isSent =1;
	//coutinterrupt++;
}
uint8_t I2C_Check(uint16_t addr)
{

    uint8_t timeOut = 10;
    LL_I2C_DisableBitPOS(I2C1); //The POS bit is used when the procedure for reception of 2 bytes
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);//Prepare the generation of a ACKnowledge or Non ACKnowledge condition after the address receive match code
                                                 //or next received byte.
    LL_I2C_GenerateStartCondition(I2C1);//Generate a START or RESTART condition.
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) { //Indicate the status of Start Bit (master mode)
        if (LL_SYSTICK_IsActiveCounterFlag()) { //This function checks if the Systick counter flag is active or not.
        	                                    //It can be used in timeout function on application side
            if (timeOut-- == 0) {
                LL_I2C_GenerateStopCondition(I2C1); //Generate a STOP condition after the current byte transfer (master mode).
                return 0;
            }
        }
    }
    LL_I2C_TransmitData8(I2C1, (addr) << 1 | I2C_REQUEST_WRITE); //Write in Transmit Data Register.
                                                                 //I2Cx: I2C Instance.
                                                                 //Data: Value between Min_Data=0x0 and Max_Data=0xFF
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {   //Indicate the status of Address sent (master mode) or Address matched flag (slave mode).
        if (LL_SYSTICK_IsActiveCounterFlag()) { //This function checks if the Systick counter flag is active or not.
                                                //It can be used in timeout function on application side
            if (timeOut-- == 0) {
                LL_I2C_GenerateStopCondition(I2C1);//Generate a STOP condition after the current byte transfer (master mode).
                return 0;
            }
        }
    }
    LL_I2C_ClearFlag_ADDR(I2C1);        //Clear Address Matched flag.
                                        //Clearing this flag is done by a read access to the I2Cx_SR1 register followed by a read access to the
                                        //I2Cx_SR2 register
    LL_I2C_GenerateStopCondition(I2C1); //Generate a STOP condition after the current byte transfer (master mode).
    return 1;
}



uint8_t I2C_Roll_Speed(uint16_t addr)
{
	LL_RCC_ClocksTypeDef rcc_clocks; //
    uint8_t idx;
    uint32_t freq;
    uint8_t loopOut = 0;

    LL_RCC_GetSystemClocksFreq(&rcc_clocks); //Return the frequencies of different on chip clocks;
                                             //System, AHB, APB1 and APB2 buses clocks.
                                             //Each time SYSCLK, HCLK, PCLK1 and/or PCLK2 clock changes,
                                             //this function must be called to update structure fields.
                                             //Otherwise, any configuration based on this function will be incorrect.
    sprintf(CDC_tx_buff, "Scanning address: 0X%x\n\r", addr);
         if(isSent == 1){
    	// ++++++ Block 1 +++++++
		  HAL_UART_Transmit_IT(&huart1, CDC_tx_buff, 64);
		  isSent = 0;
	         while(!isSent){
	         }
		  HAL_UART_Transmit_IT(&huart1, dataCl, 1);
		  isSent = 0;
         }

    for (idx = 0; idx < 4; idx++) {
        LL_mDelay(3);
        LL_I2C_Disable(I2C1);
        LL_I2C_ConfigSpeed(I2C1, rcc_clocks.PCLK1_Frequency, I2C_speed[idx], LL_I2C_DUTYCYCLE_2);
        LL_I2C_Enable(I2C1);


        if (I2C_Check(addr)& !loopOut) {
            sprintf(CDC_tx_buff, "Devise`s adores is: 0X%x\n\r", addr);
            //CDC_Transmit_FS(CDC_tx_buff, 7);

            if (isSent == 1) {
        		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
                //CDC_Transmit_FS("SCAN...\n\r", 9);
        		HAL_UART_Transmit_IT(&huart1, CDC_tx_buff, 64);
        		isSent == 0;
        		 LL_mDelay(1000);
   	            //while(!isSent){
   	            //}
        		//HAL_UART_Transmit_IT(&huart1, dataCl, 8);
        		//isSent == 0;
                }
            loopOut++;
            LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13); // LED on : I2C slave found
        }
        else {
            //sprintf(CDC_tx_buff, "  ---  ");
            //CDC_Transmit_FS(CDC_tx_buff, 7);

            if (isSent == 1) {
        		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
                //CDC_Transmit_FS("SCAN...\n\r", 9);
        		//HAL_Delay(100);
        		isSent == 0;
                }
        }
    }
    return 0;
}

//+++++++++++


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/


/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  for(uint32_t i= 0; i<10240; i++)
  {
  	TxData[i] = i&(0xff);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */\

	//+++++++++++ I2C scanner code +++++++++

    //CDC_Transmit_FS("\n\rDirty STM32 I2C Scanner V0.01\r\ntype 's' to scan\n\r", 51);
    //HAL_UART_Transmit(CDC_tx_buff, addr, 7);

        LL_mDelay(1000);
        //CDC_Transmit_FS("\n\rDirty STM32 I2C Scanner V0.01\r\ntype 's' to scan\n\r", 51);
/*
        if (isSent == 1) {
			//LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);
            //CDC_Transmit_FS("SCAN...\n\r", 9);
			HAL_UART_Transmit_IT(&huart1, data, 1024);
			HAL_UART_Transmit_IT(&huart1, dataCl, 1024);
			HAL_Delay(100);
			isSent == 0;

            //CDC_Transmit_FS("     100Khz 200Khz 300Khz 400Khz", 35);
            for (int i = 1; i < 128; i++) {
                LL_mDelay(100);
                I2C_Roll_Speed(i);
            }

        }
*/
        for (int i = 1; i < 256; i++) {
            LL_mDelay(100);
            I2C_Roll_Speed(i);
        }

//=======================================

//	  HAL_UART_Transmit(&huart1, TxData, 10240,HAL_MAX_DELAY);


	  //+++++++++ UART study part

/*
	  if(isSent == 1){
		  HAL_UART_Transmit_IT(&huart1, TxData, 10240);
          isSent =0;
	  }

	  HAL_GPIO_TogglePin(GPIOC, LED_Pin);
	  HAL_Delay(500);
	  countloop++;
*/
//======================================

/*

	  HAL_UART_Transmit(&huart1, data, 12, 1000);
	  //HAL_Delay(1000);
	  HAL_UART_Transmit(&huart1, dataCl, 1, 1000);
	  HAL_Delay(100);

	  sprintf(numarray, "%d\n", nums);
	  HAL_UART_Transmit(&huart1, numarray, 4, 100);
	  HAL_UART_Transmit(&huart1, dataCl, 1, 1000);
	  HAL_Delay(1000);


*/

   /* USER CODE BEGIN 3 */
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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

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
