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
#include "sps30.h"

#include "stm32f4xx_hal_def.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // uint8_t turn_on[] = {0x7e, 0x00, 0x00, 0x02, 0x01, 0x03, 0xf9, 0x7e};
  uint8_t read_sensor[] = {0x7e, 0x00, 0x03, 0x00, 0xfc, 0x7e};
  uint8_t stop_measurement[] = {0x7e, 0x00, 0x01, 0x00, 0xfe, 0x7e};
  // receive 47 bytes which is 40 byte data after 5 header bytes, and 2 extra bytes at end
  uint8_t receive_data_raw[200] = {0}; // have extra space for byte stuffing
  // uint8_t receive_buffer[1];
  uint8_t receive_data_unbytestuffed[50]; 
  uint8_t data_flip_endianness[40];
  // uint8_t receive_buffer[1];
  float pm_data[10];
  // char debug_string[200];
  HAL_StatusTypeDef tx_status;

  int err_code;


  while (1)
  {
  //   while ((tx_status = HAL_UART_Transmit(&huart1, turn_on, 8, 2000)) != HAL_OK)
  //  {  
  //   HAL_Delay(1000);

  //  }
  //   HAL_Delay(1000);

  // start the measurement
  if ((err_code = start_measurement(huart1)))
  {
    continue;
  }

    while ( (tx_status = HAL_UART_Transmit(&huart1, read_sensor, 6, 2000) != HAL_OK));

    /* Clear Overrun Error, Noise Error, Frame Error, and Parity Error flags */
    __HAL_UART_CLEAR_PEFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_OREFLAG(&huart1);

    /* Flush any leftover stale bytes out of the hardware register */
    uint32_t dummy = huart1.Instance->DR; 
    (void)dummy;
    __HAL_UART_CLEAR_OREFLAG(&huart1); // clear error flag
    const uint8_t START_STOP_BYTE= 0x7e;
    // unsigned int start_byte_received = 0;
    // int raw_index = 0;
    HAL_StatusTypeDef rx_status;

    // can receive one byte, take one byte at time until get end of frame
    int time_out_counter = 0;
    uint8_t index = 0;
    int start_frame = 0;
    while (index < 200) {
      rx_status = HAL_UART_Receive(&huart1,receive_data_raw + index, 1, 2000);
      if (rx_status == HAL_ERROR || rx_status == HAL_BUSY) {
        continue; // try again
      }
      if (rx_status == HAL_TIMEOUT)
      {
        if (time_out_counter == 5) {
          break;
        }
        HAL_Delay(1000);
        time_out_counter++;
        continue;
      }
      if (!start_frame && receive_data_raw[index] == START_STOP_BYTE) {
        // received byte to start receive frame
        start_frame = 1;
      }
      else if (receive_data_raw[index] == START_STOP_BYTE)
      {
        // received end of frame byte
        break;
      }

      index++; // increment index for next byte
    }
  
    if (receive_data_raw[0] != START_STOP_BYTE)
    {
      // error receiving data
      continue; // restart loop
    }

    // turn off measurement
    HAL_UART_Transmit(&huart1, stop_measurement, 6, 2000);

    // find start of frame
    uint8_t * frame_start = &receive_data_raw[0];
    // data received now need to unbytestuff
    // const int HEADER_OFFSET = 5; // first 5 bytes are header, data starts at index 5
    uint8_t *curr_byte = frame_start + 1;
    const uint8_t STUFFED_BYTE_MARKER = 0x7d;

    uint8_t i = 0;
    while (*curr_byte != START_STOP_BYTE && i < 120)  // checksum byte was added, remove at data conversion step
    {
      if (*curr_byte == STUFFED_BYTE_MARKER)
      {
        if (*(curr_byte+1) == 0x5e)
        {
          receive_data_unbytestuffed[i] = 0x7e;
        }
        else if (*(curr_byte+1) == 0x5d)
        {
          receive_data_unbytestuffed[i] = 0x7d;
        }
        else if (*(curr_byte+1) == 0x31)
        {
          receive_data_unbytestuffed[i] = 0x11;
        } 
        else if (*(curr_byte+1) == 0x33)
        {
          receive_data_unbytestuffed[i] = 0x13;
        }
        
        // increment curr_byte by 2 to skip stuffed byte
        curr_byte += 2;
        i++;
        continue;
      }
      // else, current byte is copied to array and i and curr_byte incremented
      receive_data_unbytestuffed[i++] = *curr_byte++;
    }

    if (i == 120 && (*curr_byte != START_STOP_BYTE))
    {
      continue; // did not get end of frame properly
    }

    // verify checksum
    const uint8_t CHECKSUM_BYTE = receive_data_unbytestuffed[i - 1];
    int byte_sum = 0;
    // to verify checksum, sum all bytes, take LSB of result and invert it
    for (int j = 0; j < (i - 1); j++) // i - 1, sum should not include stop byte or checksum byte
    {
      byte_sum += receive_data_unbytestuffed[j];
    }
    // checksum byte must equal the invert of the byte sum
    // cast byte_sum to uint8_t so its only LSB of the sum
    uint8_t lsb_of_byte_sum = (uint8_t) byte_sum;
    uint8_t invert_of_lsb_of_byte_sum = ~lsb_of_byte_sum;
    if (invert_of_lsb_of_byte_sum != CHECKSUM_BYTE)
    {
      continue; // restart measurement loop
    }
    

      const int LENGTH_BYTE_CORRECT_SIZE = 0x28; // correct size must be 40 bytes
      const int LENGTH_BYTE = 3; // position of length byte
      if (receive_data_unbytestuffed[LENGTH_BYTE] !=  LENGTH_BYTE_CORRECT_SIZE)
      {
        continue; // restart loop
      }


      const int CHUNK_SIZE = 4;
      const int ARRAY_SIZE = 40;
      const int HEADER_OFFSET = 4;

      for (int i = 0; i * CHUNK_SIZE < ARRAY_SIZE; i++)
      {
        curr_byte = &receive_data_unbytestuffed[i*CHUNK_SIZE + HEADER_OFFSET];
        for (int j = 3, k = 0; j >= 0; j--, k++)
        {
          data_flip_endianness[j+(i*CHUNK_SIZE)] =*(curr_byte+k);
        }
      }

      memcpy(pm_data, data_flip_endianness, sizeof(pm_data));
  }




  





    /* USER CODE BEGIN 3 */
  
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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
