/**
  ******************************************************************************
  * @file    alarm.h
  * @brief   报警输入非阻塞去抖模块 头文件
  * @note    基于 HAL_GetTick() 实现，需周期性调用 Alarm_Scan()
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ALARM_H
#define __ALARM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "gpio.h"

/* Exported constants --------------------------------------------------------*/
#define ALARM_DEBOUNCE_MS    100u     /* 去抖确认时间 20ms */
#define DEBOUNCE_STABLE      0x00    /* 电平稳定 */
#define DEBOUNCE_WAITING     0x01    /* 电平跳变，等待确认 */
#define ALARM_IDLE           0x00    /* 无报警 */
#define ALARM_ACTIVE         0x01    /* 报警中 */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief 单路报警输入控制块结构体定义
  */
typedef struct
{
    sGpio             gpio;
    GPIO_PinState     lastSample;   /* 上次采样电平 */
    GPIO_PinState     stableLevel;  /* 已确认的稳定电平 */
    uint32_t          changeTick;   /* 电平变化时刻 */
    unsigned char     state;        /* 去抖状态机 */
    unsigned char     alarm;        /* 报警状态 */
    unsigned char     deaultLevel;
} AlarmDev_t;



extern AlarmDev_t alarm_A;
extern AlarmDev_t alarm_B;


void Alarm_Scan(AlarmDev_t *dev);
void Alarm_Init(AlarmDev_t *dev, eGpioPort port, eGpioPin pin, uint8_t defaultLevel);

#endif
