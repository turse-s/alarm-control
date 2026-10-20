/**
  ******************************************************************************
  * @file    flash.c
  * @brief   通用Flash读写模块（HAL版，无全局状态）
  ******************************************************************************
  */
#include "flash.h"

#define FLASH_CFG_MAGIC   ((uint16_t)0xA55A)

/* ---------------- 内部工具 ---------------- */

static uint16_t PackLevel(uint8_t a, uint8_t b)
{
    return (uint16_t)(((a & 0x01u) << 4) | (b & 0x01u));
}

/**
 * @brief  写入两路报警默认电平 (解锁->擦页->写魔法字->写数据->上锁->回读校验)
 */
uint8_t FlashStorage_Write(uint8_t type_A, uint8_t type_B, uint8_t enable_A, uint8_t enable_B)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError = 0;
    uint16_t magic = FLASH_CFG_MAGIC;
    uint16_t alarm_type  = PackLevel(type_A, type_B);
    uint16_t alarm_enable  = PackLevel(enable_A, enable_B);

    if (HAL_FLASH_Unlock() != HAL_OK)
        return FLASH_ERR;

    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_CFG_PAGE_ADDR;
    erase.NbPages     = 1;
    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return FLASH_ERR;
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_CFG_ADDR_ALARM, alarm_type) != HAL_OK ||
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_CFG_ADDR_ENABLE, alarm_enable) != HAL_OK ||
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_CFG_ADDR_MAGIC,  magic)   != HAL_OK) {
        HAL_FLASH_Lock();
        return FLASH_ERR;
    }

    HAL_FLASH_Lock();

    /* 回读校验 */
    if (*(__IO uint16_t *)FLASH_CFG_ADDR_MAGIC != magic ||
        *(__IO uint16_t *)FLASH_CFG_ADDR_ALARM  != alarm_type ||
        *(__IO uint16_t *)FLASH_CFG_ADDR_ENABLE  != alarm_enable)
        return FLASH_ERR;

    return FLASH_OK;
}

/**
 * @brief  读取两路报警默认电平
 * @retval FLASH_OK=读到有效配置; FLASH_EMPTY=出厂态(输出默认低); FLASH_ERR=参数错误
 */
uint8_t FlashStorage_Read(uint8_t *type_A, uint8_t *type_B, uint8_t *enable_A, uint8_t *enable_B)
{
    uint16_t alarm_type;
    uint16_t alarm_enable;

    if (type_A == 0 || type_B == 0 || enable_A == 0 || enable_B == 0)
        return FLASH_ERR;

    *type_A = 0u;   /* 默认: 两路均为低电平 */
    *type_B = 0u;
    *enable_A = 0u;   /* 默认: 两路均为低电平 */
    *enable_B = 0u;


    /* 魔法字不匹配 = 从未写过, 保持默认低电平 */
    if (*(__IO uint16_t *)FLASH_CFG_ADDR_MAGIC != FLASH_CFG_MAGIC) {
        *type_A =  0x01;
        *type_B =  0x01;

        *enable_A = 0x01;
        *enable_B = 0x01;
        return FLASH_EMPTY;
    }

    alarm_type    = *(__IO uint16_t *)FLASH_CFG_ADDR_ALARM;
    *type_A = (alarm_type >> 4) & 0x01u;
    *type_B =  alarm_type       & 0x01u;
    
    alarm_enable    = *(__IO uint16_t *)FLASH_CFG_ADDR_ENABLE;
    *enable_A = (alarm_enable >> 4) & 0x01u;
    *enable_B =  alarm_enable       & 0x01u;
    return FLASH_OK;
}

/**
 * @brief  写 IAP 状态标志（解锁->擦页->半字编程->上锁->回读校验）
 * @param  state: 状态字，如 0x04 = 跳转 APP
 * @retval 0=成功, 1=失败
 * @note   单个半字是原子写入，无多字段掉电窗口；
 *         写完如需立即生效，接 NVIC_SystemReset() 复位后走 IAP 判断
 */
uint8_t IAP_SetState(uint16_t state)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError = 0;

    if (HAL_FLASH_Unlock() != HAL_OK)
        return 1;

    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_IAP_SATE_PAGE;
    erase.NbPages     = 1;
    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return 1;
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_IAP_SATE_ADDR, state) != HAL_OK) {
        HAL_FLASH_Lock();
        return 1;
    }

    HAL_FLASH_Lock();

    /* 回读校验 */
    if (*(__IO uint16_t *)FLASH_IAP_SATE_ADDR != state)
        return 1;

    return 0;
}
