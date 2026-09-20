/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define UART_RX_BUF_SIZE        8
#define UART_TX_BUF_SIZE        8

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

typedef struct
{;
    unsigned char frameReady; 
    unsigned char txBuf[UART_TX_BUF_SIZE + 1];
    unsigned short rxCnt;
    unsigned char rxBuf[UART_RX_BUF_SIZE + 1];

    unsigned char ringBuf[UART_RX_BUF_SIZE +1];
} sUart;

extern UART_HandleTypeDef huart1;
extern sUart uart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void uart_Send(uint16_t cmd, unsigned char data1, unsigned char date2);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
