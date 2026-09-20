#include "parser.h"
#include "usart.h"
#include "gpio.h"
#include "flash.h"

static void (*boot_entry)(void);

sCommRqMsg peMsg;

int parse_uart_data(sUart *bus, sCommRqMsg *rqMsg)
{
    int err = 0;
    unsigned long checksum = 0;
    uint8_t levelA = 0, levelB = 0;
    
    memset(rqMsg->rqData, 0, sizeof(rqMsg->rqData));
    memmove(rqMsg->rqData, bus->rxBuf, bus->rxCnt);
    
    rqMsg->length = bus->rxCnt;
    rqMsg->index = 0;
    rqMsg->header = rqMsg->rqData[rqMsg->index++];
    rqMsg->index++;
    rqMsg->cmd = (rqMsg->rqData[rqMsg->index]<<8) | rqMsg->rqData[rqMsg->index+1];
    rqMsg->index += 2;
    rqMsg->data1 = rqMsg->rqData[rqMsg->index++];
    rqMsg->data2 = rqMsg->rqData[rqMsg->index++];
    rqMsg->checksum = rqMsg->rqData[rqMsg->index++];
    
   if (rqMsg->header == 0x99) {
        Jump_To_Bootloader();
    }
    
    if (rqMsg->length < PELCO_LENGTH) {
        return PARSE_ERR_LEN;
    }
    
    if (rqMsg->header != PELCO_HEADER) {
        uart1.frameReady = 0;
        return PARSE_ERR_HEAD;
    }
    
    for(int i = 1; i < PELCO_LENGTH - 1; i++) {
        checksum += rqMsg->rqData[i];

    }
    
    if (rqMsg->checksum != (checksum & 0xff)) {
        return PARSE_ERR_CHK;
    }
    
    switch(rqMsg->cmd) {
        case SET_ALARM_CONFIG:
            FlashStorage_Write(rqMsg->data1, rqMsg->data2);
            levelA = (rqMsg->data1 >> 4) & 0x01u;
            levelB = rqMsg->data1 & 0x01u;
            ENA.sta = levelA ? GPIO_HIGH : GPIO_LOW;
            ENB.sta = levelA ? GPIO_HIGH : GPIO_LOW;

            ENA.status = (rqMsg->data2 >> 4) & 0x01u;
            ENB.status = rqMsg->data2 & 0x01u;
            break;
        case ALARM_INPUT:
            levelA = rqMsg->data1;
            levelB = rqMsg->data2;
            ENA.sta = levelA ? GPIO_HIGH : GPIO_LOW;
            ENB.sta = levelB ? GPIO_HIGH : GPIO_LOW;
            gpioWrite(&ENA);
            gpioWrite(&ENB);
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
 