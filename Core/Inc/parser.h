#ifndef __PARSER_H__
#define __PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "usart.h"
#include <string.h>


#define IAP_BOOT_ADDR   0x08000000U

#define PELCO_HEADER           0xfa
#define PELCO_LENGTH           6

#define PARSE_ERR_LEN   1   // 数据长度不足
#define PARSE_ERR_HEAD  2   // 帧头错误
#define PARSE_ERR_CHK   3   // 校验和错误

#define COMM_MSG_LENGTH_MAX                     8


#define SET_ALARM_CONFIG                        0x71
#define ALARM_INPUT                             0x72
#define BOOT_HEADER                             0x99
typedef struct
{
    unsigned char header;
    unsigned char data1;
    unsigned char data2;
    unsigned char rqAddr;
    unsigned char rqSeq;
    uint16_t cmd;
    unsigned char checksum;
    unsigned char itemCnt;
    unsigned short netFnBase;
    unsigned short index;
    unsigned short length;
    unsigned short frmLen;
    unsigned char tempVal;
    unsigned char rqData[COMM_MSG_LENGTH_MAX];
} sCommRqMsg;

extern sCommRqMsg peMsg;

int parse_uart_data(const sUartMsg *msg, sCommRqMsg *rqMsg);

void Jump_To_Bootloader(void);

#endif
