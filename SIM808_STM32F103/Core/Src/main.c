/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
int __io_putchar(int ch) {
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char rxBuffer[1024];
char txBuffer[128];
char gpsLat[16] = "0.000000";
char gpsLon[16] = "0.000000";
uint8_t gpsFixed = 0;
volatile uint16_t rxIndex = 0;
volatile uint8_t rxComplete = 0;
uint8_t rxChar;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
void sendAT(char *cmd, uint32_t delayMs);
void getGPSData();
void submitHttpRequest();
void clearRxBuffer();
void startUartReceive();
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  printf("Starting SIM808 GPS Tracker...\r\n");

  // Start interrupt-based UART reception
  startUartReceive();

  // Basic AT command test
  HAL_UART_Transmit(&huart1, (uint8_t *)"AT\r\n", 4, 1000);
  HAL_Delay(1000);

  // Enable GPS power and wait longer for GPS to initialize
  printf("Enabling GPS...\r\n");
  sendAT("AT+CGNSPWR=1\r\n", 2000); // Enable GPS power
  HAL_Delay(5000); // Wait 5 seconds for GPS to power up

  // Configure GPRS
  printf("Configuring GPRS...\r\n");
  sendAT("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n", 1000);
  sendAT("AT+SAPBR=3,1,\"APN\",\"YourAPN\"\r\n", 1000);
  sendAT("AT+SAPBR=1,1\r\n", 3000);
  sendAT("AT+HTTPINIT\r\n", 500);

  printf("Initialization complete. Starting main loop...\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    getGPSData();

    if (gpsFixed) {
      printf("GPS Fixed! Latitude = %s, Longitude = %s\r\n", gpsLat, gpsLon);
      submitHttpRequest();
    } else {
      printf("GPS not fixed yet. Latitude = %s, Longitude = %s\r\n", gpsLat, gpsLon);
    }

    HAL_Delay(10000); // Wait 10 seconds before next reading
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
  * in the RCC_OscInitStruct structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void clearRxBuffer() {
  rxIndex = 0;
  rxComplete = 0;
  memset(rxBuffer, 0, sizeof(rxBuffer));
}

void startUartReceive() {
  HAL_UART_Receive_IT(&huart1, &rxChar, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    if (rxIndex < sizeof(rxBuffer) - 1) {
      rxBuffer[rxIndex++] = rxChar;
      rxBuffer[rxIndex] = '\0';

      // Check for end of response (OK or ERROR)
      if (strstr(rxBuffer, "OK\r\n") || strstr(rxBuffer, "ERROR\r\n") ||
          strstr(rxBuffer, "+CGNSINF:")) {
        rxComplete = 1;
      }
    }
    // Continue receiving
    HAL_UART_Receive_IT(&huart1, &rxChar, 1);
  }
}

void sendAT(char *cmd, uint32_t delayMs) {
  clearRxBuffer();
  printf("Sending: %s", cmd);

  HAL_UART_Transmit(&huart1, (uint8_t *)cmd, strlen(cmd), 1000);

  // Wait for response with timeout
  uint32_t startTime = HAL_GetTick();
  while (!rxComplete && (HAL_GetTick() - startTime) < delayMs) {
    HAL_Delay(10);
  }

  if (rxComplete) {
    printf("Response: %s\r\n", rxBuffer);
  } else {
    printf("Timeout waiting for response\r\n");
  }
}

void getGPSData() {
  printf("Getting GPS data...\r\n");

  clearRxBuffer();
  HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CGNSINF\r\n", 12, 1000);

  // Wait for complete response with longer timeout
  uint32_t startTime = HAL_GetTick();
  while (!rxComplete && (HAL_GetTick() - startTime) < 5000) {
    HAL_Delay(50);
  }

  if (rxComplete) {
    printf("GPS Raw Response: %s\r\n", rxBuffer);

    // Look for CGNSINF response
    char *cgnsinf_start = strstr(rxBuffer, "+CGNSINF:");
    if (cgnsinf_start) {
      printf("Found CGNSINF response\r\n");

      // Create a working copy to avoid modifying original buffer
      char workingBuffer[512];
      strcpy(workingBuffer, cgnsinf_start + 10); // Skip "+CGNSINF: "

      // Remove any trailing \r\n or other characters after the data
      char *endOfData = strstr(workingBuffer, "\r");
      if (endOfData) *endOfData = '\0';

      printf("Working with: %s\r\n", workingBuffer);

      // Parse the comma-separated values
      char *token = strtok(workingBuffer, ",");
      int index = 0;

      while (token != NULL && index < 10) {
        // Remove any whitespace
        while (*token == ' ') token++;

        printf("Field %d: '%s'\r\n", index, token);

        if (index == 0) {
          // GPS run status (should be 1)
          printf("GPS Run Status: %s\r\n", token);
        }
        if (index == 1) {
          // GPS fix status (should be 1)
          gpsFixed = (atoi(token) == 1) ? 1 : 0;
          printf("GPS Fix Status: %s (%s)\r\n", token, gpsFixed ? "FIXED" : "NOT FIXED");
        }
        if (index == 3 && strlen(token) > 1) {
          // Latitude
          strncpy(gpsLat, token, sizeof(gpsLat)-1);
          gpsLat[sizeof(gpsLat)-1] = '\0';
          printf("Parsed Latitude: '%s'\r\n", gpsLat);
        }
        if (index == 4 && strlen(token) > 1) {
          // Longitude
          strncpy(gpsLon, token, sizeof(gpsLon)-1);
          gpsLon[sizeof(gpsLon)-1] = '\0';
          printf("Parsed Longitude: '%s'\r\n", gpsLon);
        }

        token = strtok(NULL, ",");
        index++;
      }
    } else {
      printf("No CGNSINF found in response\r\n");
      gpsFixed = 0;

      // Let's also check what we actually received
      printf("Raw buffer content (hex): ");
      for(int i = 0; i < rxIndex && i < 100; i++) {
        printf("%02X ", (uint8_t)rxBuffer[i]);
      }
      printf("\r\n");
    }
  } else {
    printf("Timeout waiting for GPS data\r\n");
    gpsFixed = 0;
  }
}

void submitHttpRequest() {
  if (!gpsFixed) {
    printf("GPS not fixed, skipping HTTP request\r\n");
    return;
  }

  printf("Submitting HTTP request with GPS data...\r\n");

  char url[256];
  snprintf(url, sizeof(url),
    "AT+HTTPPARA=\"URL\",\"http://yourwebsite.com/getdata.php?lat=%s&lng=%s\"\r\n",
    gpsLat, gpsLon);

  printf("URL: %s", url);

  sendAT("AT+HTTPPARA=\"CID\",1\r\n", 500);
  sendAT(url, 500);
  sendAT("AT+HTTPACTION=0\r\n", 5000); // GET request, wait longer for response

  printf("HTTP request submitted\r\n");
}

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
