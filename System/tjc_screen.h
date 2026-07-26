#ifndef __TJC_SCREEN_H__
#define __TJC_SCREEN_H__

#include <stdint.h>

/* ============================================================
 * 淘晶驰 T1 串口屏驱动
 *
 * 移植只需改 tjc_screen.c 里的 huart6 / USART6
 *
 * 屏端按钮事件: printh 01 ~ printh 04 (单字节, 无结束符)
 * 屏端指令格式: <命令> + \xFF\xFF\xFF
 * ============================================================ */

/* 初始化 (printf 重定向 + 启动 RX 中断) */
void TJC_Init(void);

/* 发送 */
void TJC_SetPage(const char *page);
void TJC_SetText(const char *name, const char *text);
void TJC_SetNum(const char *name, int num);
void TJC_WaveAdd(int wid, int ch, int val);
void TJC_WaveClear(int wid, int ch);

/* 读按键: 返回 1~4, 无按键返回 0 */
uint8_t TJC_GetKey(void);

#endif
