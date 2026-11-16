#include "parser.h"
#include "usart.h"
#include "gpio.h"
#include "flash.h"
#include "alarm.h"

static void (*boot_entry)(void);

sCommRqMsg peMsg;


/**
  * @brief  串口数据协议解析
  * @param  dev:            报警设备控制块指针
  * @param  port:           GPIO 端口，如 GPIOA
  * @param  pin:            GPIO 引脚，如 GPIO_PIN_0
  * @param  defaultLevel:   上电默认电平-非报警状态电平
  * @param  enable:         是否使能报警

  * @retval None
  */
  
  
int parse_uart_data(const sUartMsg *msg, uint8_t len, sCommRqMsg *rqMsg)
{
    int err = 0;
    unsigned long checksum = 0;
    uint8_t levelA = 0, levelB = 0;
    uint8_t alarmA_type = 0, alarmB_type = 0;
    uint8_t alarmA_enable = 0, alarmB_enable = 0;
    
    if ((msg == NULL) || (rqMsg == NULL)) {
    return PARSE_ERR_HEAD;
    }
    
    memset(rqMsg->rqData, 0, sizeof(rqMsg->rqData));
    memmove(rqMsg->rqData, msg->data, len);
    
    rqMsg->length = len;
    rqMsg->index = 0;
    rqMsg->header = rqMsg->rqData[rqMsg->index++];
    rqMsg->index++;
    rqMsg->cmd = rqMsg->rqData[rqMsg->index++];
    rqMsg->data1 = rqMsg->rqData[rqMsg->index++];
    rqMsg->data2 = rqMsg->rqData[rqMsg->index++];
    rqMsg->checksum = rqMsg->rqData[len - 1];
    
    if (rqMsg->header == 0x99 && rqMsg->rqData[3] == 0xC3) {
        IAP_SetState(0x01);
        Jump_To_Bootloader();
    }
    
    if (rqMsg->length < 6) {
        return PARSE_ERR_LEN;
    }
    
    if (rqMsg->header != ALARM_HEADER && rqMsg->header != PELCO_HEADER) {
        uart1.frameReady = 0;
        return PARSE_ERR_HEAD;
    }
    if (len == ALARM_LENGTH) {
        for(int i = 0; i < ALARM_LENGTH - 1; i++) {
            checksum += rqMsg->rqData[i];
        }
    } else {
        for(int i = 1; i < PELCO_LENGTH - 1; i++) {
            checksum += rqMsg->rqData[i];
        }
    }
    
    
    if (rqMsg->checksum != (checksum & 0xff)) {
        return PARSE_ERR_CHK;
    }
    switch (rqMsg->header) {
        case PELCO_HEADER :
            switch(rqMsg->cmd) {
                case POWER_CYCLE_REBOOT:
                    Reset.sta = GPIO_LOW;
                    gpioWrite(&Reset);
                    HAL_Delay(3000);
                    Reset.sta = GPIO_HIGH;
                    gpioWrite(&Reset);
                    break;
                default:
                    break;
            }
            break;
        case ALARM_HEADER:
            switch(rqMsg->cmd) {
                case SET_ALARM_CONFIG:
                    alarmB_type = (rqMsg->data1 >> 4) & 0x01u;
                    alarmA_type = rqMsg->data1 & 0x01u;
                    alarmB_enable = (rqMsg->data2 >> 4) & 0x01u;
                    alarmA_enable = rqMsg->data2 & 0x01u;
                
                    FlashStorage_Write(alarmA_type, alarmB_type, alarmA_enable, alarmB_enable);
                
                    alarm_A.deaultLevel = alarmA_type ? GPIO_LOW : GPIO_HIGH;
                    alarm_B.deaultLevel = alarmB_type ? GPIO_LOW : GPIO_HIGH;

                    alarm_A.stableLevel = alarmA_type ? GPIO_HIGH : GPIO_LOW;
                    alarm_B.stableLevel = alarmB_type ? GPIO_HIGH : GPIO_LOW;
                
                    alarm_A.enable = alarmA_enable ? GPIO_HIGH : GPIO_LOW;
                    alarm_B.enable = alarmB_enable ? GPIO_HIGH : GPIO_LOW;
                    alarm_A.alarm = ALARM_IDLE;
                    alarm_B.alarm = ALARM_IDLE;
                    break;
                case ALARM_INPUT:
                    levelA = rqMsg->data1;
                    levelB = rqMsg->data2;
                    alarm_A.alarm = levelA ? GPIO_LOW : GPIO_HIGH;
                    alarm_B.alarm = levelB ? GPIO_LOW : GPIO_HIGH;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    
    }
    
    uart1.frameReady = 0;
    return 0;
}

/**
  * @brief  程序跳转至IAP地址
  * @retval None
  */
void Jump_To_Bootloader(void)
{
    uint32_t boot_sp;
    uint32_t boot_pc;

    boot_sp = *(volatile uint32_t *)(IAP_BOOT_ADDR + 0U);   /* 初始 MSP  */
    boot_pc = *(volatile uint32_t *)(IAP_BOOT_ADDR + 4U);   /* Reset_Handler */

    if ((boot_sp & 0x2FFE0000U) != 0x20000000U)
        return;

    while (huart1.Instance->ISR & USART_ISR_RXNE)
    {
        (void)huart1.Instance->RDR;
    }

    huart1.Instance->ICR = 0xFFFFFFFFU;
    
    __disable_irq();

    NVIC->ICER[0] = 0xFFFFFFFFU;
    NVIC->ICPR[0] = 0xFFFFFFFFU;

    __HAL_RCC_USART1_FORCE_RESET();     /* 硬复位：所有寄存器/标志归零 */
    __HAL_RCC_USART1_RELEASE_RESET();

    HAL_RCC_DeInit();

    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;
    

    
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_FLASH();
        

    __set_MSP(boot_sp);
    __DSB();
    __ISB();
    __enable_irq(); 
    ((void (*)(void))boot_pc)();

}
 