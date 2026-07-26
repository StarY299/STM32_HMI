#ifndef __TJC_SCREEN_H__
#define __TJC_SCREEN_H__

#include <stdint.h>

/* ============================================================
 * 淘晶驰 (TJC) 串口屏驱动
 * ============================================================ */

/* ── 基础发送 ── */
void HMI_SendString(char *name, char *showdata);
void HMI_SendNumber(char *name, int num);
void HMI_SendFloat(char *name, float num);
void HMI_SendPage(const char *page_name);

/* ── 波形操作 ── */

/* 向波形组件添加一个数据点
   wave_id: 波形组件序号 (屏上第一个波形=1)
   channel: 通道号 (0~3, 协议从0开始)
   value:   数据值 (0~255) */
#define HMI_WaveAdd(wave_id, channel, value) \
    printf("add %d,%d,%d\xff\xff\xff", (wave_id), (channel), (value))

/* 清空波形通道 (切波形时用) */
#define HMI_WaveClear(wave_id, channel) \
    printf("cle %d,%d\xff\xff\xff", (wave_id), (channel))

/* ── 触摸解析 ── */
#define HMI_TOUCH_NONE    0
#define HMI_TOUCH_PRESS   1
#define HMI_TOUCH_RELEASE 2

typedef struct {
    uint8_t type;
    char    id[16];
    uint8_t raw[3];
} HMI_TouchEvent;

int HMI_ParseTouch(HMI_TouchEvent *evt);

/* ── UART 接收 (来自 uart.h) ── */
extern uint8_t  USART_RX_BUF[];
extern uint16_t USART_RX_STA;

#endif /* __TJC_SCREEN_H__ */
