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
#include "dma.h"     // Direct Memory Access support
#include "fatfs.h"   // File system (FAT) support, likely for SD cards
#include "i2c.h"     // Inter-Integrated Circuit (I2C) peripheral interface
#include "mdma.h"    // Master DMA support for memory transfers
#include "rtc.h"     // Real-Time Clock functionality
#include "sdmmc.h"   // SD/MMC support (likely for SD card interfacing)
#include "spi.h"     // Serial Peripheral Interface (SPI) support
#include "usart.h"   // Universal Synchronous/Asynchronous Receiver/Transmitter (USART) for serial communication
#include "gpio.h"    // General Purpose Input/Output configuration
#include "fmc.h"     // Flexible Memory Controller, used to interface external memories

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "setup.h"
#include "diagnosis.h"
#include "app_control.h"

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

/* USER CODE BEGIN PV */
// int rgb_cnt = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();
/* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_MDMA_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_FMC_Init();
  MX_RTC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  Node_Init();
  // Record_Sensing();
  // LED_RGB(0,1,0);
//	for(int i = 0; i < 100; i++)
//  {
//    Record_Sensing();

//    HAL_Delay(5000);
//  }
//	char ch;
//  printf("Please input a character: \n\r");

//  // use while to wait for the input
//  scanf("%c", &ch);

//  printf("The character you input is: %c\n\r", ch);
	
	// Sample sensor data
	  SensorData sensor_data = {
        {3.5, 3.3, 3.5, 3.4, 3.5},  // battery_voltage
        {20, 21, 22, 23, 26},       // temperature
        {-0.3, 0.3, -0.1, 0.2, -0.3},  // acceleration
				{200, 200, 200, 200, 200},  // gyroscope
        "variable",                  // previous_data
				false
		};
    SensorMetrics metrics = calculate_metrics(sensor_data);
    printf("%d\n", metrics.acceleration_size);

    int choice;
    printf("Please select the fault type:\n");
    printf("0: Normal\n");
    printf("1: Missing\n");
    printf("2: Minor\n");
    printf("3: Outlier\n");
    printf("4: Square\n");
    printf("5: Trend\n");
    printf("6: Drift\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    diagnose(sensor_data, metrics, choice);
		
		
//		SensorMetrics metrics; // state metrics

    // Run the diagnostic function
    diagnose(sensor_data, metrics, choice);

    // // Free allocated memory
    // free(sensor_data.battery_voltage);
    // free(sensor_data.temperature);
    // free(sensor_data.acceleration);

    return 0;
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		MPU6050_t MPU6050;
		MPU6050_Read_All(&hi2c2, &MPU6050);


		printf("Ax: %f, Ay: %f, Az: %f\n", MPU6050.Ax, MPU6050.Ay, MPU6050.Az);
		printf("Gx: %f, Gy: %f, Gz: %f\n", MPU6050.Gx, MPU6050.Gy, MPU6050.Gz);
		printf("Temperature: %f\n", MPU6050.Temperature);

    // MPU6050_Read_Show(&IMU_Calibration_Instance);
    // Triggering_Check(&LiftNode_TM);
    // HAL_Delay(10);
//    if(button_trigger == 1)
//    {
//      printf("Button Triggered!\n");
//      // LED_RGB(0,0,1);
//      Record_Sensing();
//      // LED_RGB(0,0,0);
//      printf("Triggering Processed!\n");
//    }
    /* USER CODE END WHILE */

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{

  /* Disables the MPU */
  HAL_MPU_Disable();
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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
