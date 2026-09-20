/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

typedef enum
{
    GPIO_A = (unsigned int)GPIOA,
    GPIO_B = (unsigned int)GPIOB,
    GPIO_C = (unsigned int)GPIOC,
    GPIO_F = (unsigned int)GPIOF,
} eGpioPort;

// gpio pin
typedef enum
{
    GPIO_0 = GPIO_PIN_0,
    GPIO_1 = GPIO_PIN_1,
    GPIO_2 = GPIO_PIN_2,
    GPIO_3 = GPIO_PIN_3,
    GPIO_4 = GPIO_PIN_4,
    GPIO_5 = GPIO_PIN_5,
    GPIO_6 = GPIO_PIN_6,
    GPIO_7 = GPIO_PIN_7,
    GPIO_8 = GPIO_PIN_8,
    GPIO_9 = GPIO_PIN_9,
    GPIO_10 = GPIO_PIN_10,
    GPIO_11 = GPIO_PIN_11,
    GPIO_12 = GPIO_PIN_12,
    GPIO_13 = GPIO_PIN_13,
    GPIO_14 = GPIO_PIN_14,
    GPIO_15 = GPIO_PIN_15,
} eGpioPin;

// gpio status
typedef enum
{
    GPIO_LOW = GPIO_PIN_RESET,
    GPIO_HIGH = GPIO_PIN_SET,
} eGpioSta;


typedef struct
{
    eGpioPort port;
    eGpioPin pin;
    eGpioSta sta;
    uint8_t status;
} sGpio;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

extern sGpio ENA;
extern sGpio ENB;


int alarmStaGpioInit(void);

int gpioRead(sGpio *gpio);
int gpioWrite(sGpio *gpio);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
