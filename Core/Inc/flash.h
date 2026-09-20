/* flash.h */
#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f0xx_hal.h"

#define FLASH_OK      0u   /* 读到有效配置 / 写入成功 */
#define FLASH_ERR     1u   /* 操作失败 */
#define FLASH_EMPTY   2u   /* Flash 从未写过(出厂态), 调用方用默认值 */

/* F042F6P6: 32KB Flash, 1KB/页, 最后一页首地址 */
#define FLASH_CFG_PAGE_ADDR   ((uint32_t)0x08007C00)
#define FLASH_CFG_ADDR_MAGIC  ((uint32_t)0x08007C00)  /* 魔法字 */
#define FLASH_CFG_ADDR_DATA   ((uint32_t)0x08007C02)  /* 高4位=A路电平, 低4位=B路电平 */

uint8_t FlashStorage_Read (uint8_t *levelA, uint8_t *levelB);
uint8_t FlashStorage_Write(uint8_t levelA, uint8_t levelB);

#endif

