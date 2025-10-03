/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EN_DRUM_COM_Pin GPIO_PIN_2
#define EN_DRUM_COM_GPIO_Port GPIOC
#define EN_SPOOLER_COM_Pin GPIO_PIN_3
#define EN_SPOOLER_COM_GPIO_Port GPIOC
#define MODE_IN_Pin GPIO_PIN_0
#define MODE_IN_GPIO_Port GPIOA
#define SPEED_HIGH_IN_Pin GPIO_PIN_1
#define SPEED_HIGH_IN_GPIO_Port GPIOA
#define SPEED_LOW_IN_Pin GPIO_PIN_4
#define SPEED_LOW_IN_GPIO_Port GPIOA
#define CCW_IN_Pin GPIO_PIN_5
#define CCW_IN_GPIO_Port GPIOA
#define CW_IN_Pin GPIO_PIN_0
#define CW_IN_GPIO_Port GPIOB
#define AC_RELAY_Pin GPIO_PIN_14
#define AC_RELAY_GPIO_Port GPIOB
#define DC_RELAY_Pin GPIO_PIN_15
#define DC_RELAY_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
