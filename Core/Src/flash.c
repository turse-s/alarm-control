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
uint8_t FlashStorage_Write(uint8_t levelA, uint8_t levelB)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError = 0;
    uint16_t magic = FLASH_CFG_MAGIC;
    uint16_t data  = PackLevel(levelA, levelB);

    if (HAL_FLASH_Unlock() != HAL_OK)
        return FLASH_ERR;

    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_CFG_PAGE_ADDR;
    erase.NbPages     = 1;
    if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return FLASH_ERR;
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_CFG_ADDR_MAGIC, magic) != HAL_OK ||
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_CFG_ADDR_DATA,  data)   != HAL_OK) {
        HAL_FLASH_Lock();
        return FLASH_ERR;
    }

    HAL_FLASH_Lock();

    /* 回读校验 */
    if (*(__IO uint16_t *)FLASH_CFG_ADDR_MAGIC != magic ||
        *(__IO uint16_t *)FLASH_CFG_ADDR_DATA  != data)
        return FLASH_ERR;

    return FLASH_OK;
}

/**
 * @brief  读取两路报警默认电平
 * @retval FLASH_OK=读到有效配置; FLASH_EMPTY=出厂态(输出默认低); FLASH_ERR=参数错误
 */
uint8_t FlashStorage_Read(uint8_t *levelA, uint8_t *levelB)
{
    uint16_t data;

    if (levelA == 0 || levelB == 0)
        return FLASH_ERR;

    *levelA = 0u;   /* 默认: 两路均为低电平 */
    *levelB = 0u;

    /* 魔法字不匹配 = 从未写过, 保持默认低电平 */
    if (*(__IO uint16_t *)FLASH_CFG_ADDR_MAGIC != FLASH_CFG_MAGIC)
        return FLASH_EMPTY;

    data    = *(__IO uint16_t *)FLASH_CFG_ADDR_DATA;
    *levelA = (data >> 4) & 0x01u;
    *levelB =  data       & 0x01u;
    return FLASH_OK;
}