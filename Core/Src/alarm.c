/**
  ******************************************************************************
  * @file    alarm.c
  * @brief   报警输入非阻塞去抖模块 源文件
  * @note    状态机原理：
  *          STABLE  稳定态 —— 电平跳变后进入 WAITING 态并开始计时
  *          WAITING 等待态 —— 电平再次变化则重新计时；
  *                          电平保持稳定超过 ALARM_DEBOUNCE_MS 则确认有效
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "alarm.h"

/* Private variables ---------------------------------------------------------*/
AlarmDev_t alarm_A;
AlarmDev_t alarm_B;



/**
  * @brief  初始化一路报警输入
  * @note   以调用时刻的引脚电平作为初始稳定电平
  * @param  dev:  报警设备控制块指针
  * @param  port: GPIO 端口，如 GPIOA
  * @param  pin:  GPIO 引脚，如 GPIO_PIN_0
  * @retval None
  */
void Alarm_Init(AlarmDev_t *dev, eGpioPort port, eGpioPin pin, uint8_t defaultLevel)
{
    /* 初始化 GPIO 端口和引脚 */
    dev->gpio.port = port;
    dev->gpio.pin  = pin;

    /* 以上电初始电平为基准 */
    dev->lastSample  = defaultLevel ? GPIO_PIN_SET : GPIO_PIN_RESET;;
    dev->deaultLevel = dev->lastSample;
    dev->stableLevel = dev->lastSample;
    dev->changeTick  = HAL_GetTick();

    /* 状态机复位 */
    dev->state = DEBOUNCE_STABLE;
    dev->alarm = ALARM_IDLE;
}

/**
  * @brief  报警输入去抖扫描（非阻塞）
  * @note   需周期性调用，建议 5~10ms 一次；
  *         (nowTick - changeTick) 为无符号减法，
  *         HAL_GetTick() 溢出回绕后计算依然正确
  * @param  dev: 报警设备控制块指针
  * @retval None
  */
void Alarm_Scan(AlarmDev_t *dev)
{
    GPIO_PinState now     = gpioRead(&dev->gpio);
    uint32_t      nowTick = HAL_GetTick();

    switch (dev->state)
    {
        /* ---- 稳定态：检测电平是否跳变 ---- */
        case DEBOUNCE_STABLE:
            if (now != dev->stableLevel) {
                /* 电平跳变，开始去抖计时 */
                dev->lastSample = now;
                dev->changeTick = nowTick;
                dev->state      = DEBOUNCE_WAITING;
            } else {
                /* Nothing to do */
            }
            break;

        /* ---- 等待态：判断抖动 / 确认有效 ---- */
        case DEBOUNCE_WAITING:
            dev->lastSample = now;
            if (now != dev->lastSample) {
                /* 电平再次变化，视为抖动，重新计时 */
                dev->changeTick = nowTick;
            } else if ((nowTick - dev->changeTick) >= ALARM_DEBOUNCE_MS) {
                /* 电平稳定超时，确认本次电平变化有效 */
                GPIO_PinState oldLevel = dev->stableLevel;
                dev->stableLevel = now;
                dev->state       = DEBOUNCE_STABLE;
                if (now != oldLevel) {
                    if (now == GPIO_PIN_RESET) {
                        /* 低电平 = 报警触发（高触发动机请对调分支）*/
                        dev->alarm = ALARM_ACTIVE;
                    } else {
                        /* 高电平 = 报警解除 */
                        dev->alarm = ALARM_IDLE;
                    }
                }
            }
            break;
        default:
            /* 状态异常，恢复默认状态 */
            dev->state = DEBOUNCE_STABLE;
            break;
    }
}

