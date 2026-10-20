#include "parser.h"
#include "usart.h"
#include "gpio.h"
#include "flash.h"
#include "alarm.h"

static void (*boot_entry)(void);

sCommRqMsg peMsg;

int parse_uart_data(const sUartMsg *msg, sCommRqMsg *rqMsg)
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
    memmove(rqMsg->rqData, msg->data, PELCO_LENGTH);
    
    rqMsg->length = PELCO_LENGTH;
    rqMsg->index = 0;
    rqMsg->header = rqMsg->rqData[rqMsg->index++];
    rqMsg->index++;
    rqMsg->cmd = rqMsg->rqData[rqMsg->index++];
    rqMsg->data1 = rqMsg->rqData[rqMsg->index++];
    rqMsg->data2 = rqMsg->rqData[rqMsg->index++];
    rqMsg->checksum = rqMsg->rqData[rqMsg->index++];
    
    if (rqMsg->header == 0x99 && rqMsg->rqData[3] == 0xC3) {
        IAP_SetState(0x01 );
        Jump_To_Bootloader();
    }
    
    if (rqMsg->length < PELCO_LENGTH) {
        return PARSE_ERR_LEN;
    }
    
    if (rqMsg->header != PELCO_HEADER) {
        uart1.frameReady = 0;
        return PARSE_ERR_HEAD;
    }
    
    for(int i = 0; i < PELCO_LENGTH - 1; i++) {
        checksum += rqMsg->rqData[i];

    }
    
    if (rqMsg->checksum != (checksum & 0xff)) {
        return PARSE_ERR_CHK;
    }
    
    switch(rqMsg->cmd) {
        case SET_ALARM_CONFIG:
//            FlashStorage_Write(rqMsg->data1, rqMsg->data2);
            alarmB_type = (rqMsg->data1 >> 4) & 0x01u;
            alarmA_type = rqMsg->data1 & 0x01u;
            alarmB_enable = (rqMsg->data2 >> 4) & 0x01u;
            alarmA_enable = rqMsg->data2 & 0x01u;
        
            FlashStorage_Write(alarmA_type, alarmB_type, alarmA_enable, alarmB_enable);
        
            alarm_A.deaultLevel = alarmA_type ? GPIO_HIGH : GPIO_LOW;
            alarm_B.deaultLevel = alarmB_type ? GPIO_HIGH : GPIO_LOW;

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
//            gpioWrite(&ENA);
//            gpioWrite(&ENB);
            break;
        default:
            break;
        
    }
    
    uart1.frameReady = 0;
}

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
//    __HAL_RCC_DMA1_FORCE_RESET();
//    __HAL_RCC_DMA1_RELEASE_RESET();
    HAL_RCC_DeInit();

    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;
    
//    for (int i = 0; i < 48U; i++)
//        *(volatile uint32_t *)(0x20000000U + 4U * i) =
//        *(volatile uint32_t *)(IAP_BOOT_ADDR + 4U * i);
    
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_SYSCFG_REMAPMEMORY_FLASH();
        
//    __HAL_SYSCFG_REMAPMEMORY_SRAM();
//    SYSCFG->CFGR1 &= ~SYSCFG_CFGR1_MEM_MODE;   /* 00 = Flash 映射到 0x0 */

//    __enable_irq();

//    boot_entry = (void (*)(void))boot_pc;
    __set_MSP(boot_sp);
    __DSB();
    __ISB();
    __enable_irq(); 
    ((void (*)(void))boot_pc)();

//    while (1) { }   /* 不会到达 */
}
 