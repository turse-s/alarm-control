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

#define UART_RX_BUF_SIZE        54
#define UART_RX_MSG_SIZE        7
#define UART_RX_MSG_NUM         8
#define UART_TX_BUF_SIZE        8

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

typedef struct {
    uint8_t data[UART_RX_MSG_SIZE];
    uint8_t len;
} sUartMsg;

typedef struct {
    /* RX: DMA直达缓冲 */
    uint8_t          ringBuf[UART_RX_BUF_SIZE];
    /* RX: 帧环形队列 */
    sUartMsg         rxMsg[UART_RX_MSG_NUM];
    volatile uint8_t rxHead;      /* 仅ISR写 */
    volatile uint8_t rxTail;      /* 仅主循环写 */
    volatile uint8_t frameReady;  /* 队列非空提示 */
    uint16_t         rxOvf;       /* 队列满丢弃计数 */
    uint16_t         rxErr;       /* 半帧/噪声计数 */
    /* TX */
    uint8_t          txBuf[UART_TX_BUF_SIZE];
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
