#include "tjc_screen.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * 淘晶驰 (TJC) 串口屏驱动
 * ============================================================ */

/* ── 发送 ── */
void HMI_SendString(char *name, char *showdata)
{
    printf("%s=\"%s\"\xff\xff\xff", name, showdata);
}

void HMI_SendNumber(char *name, int num)
{
    printf("%s=%d\xff\xff\xff", name, num);
}

void HMI_SendFloat(char *name, float num)
{
    printf("%s=%d\xff\xff\xff", name, (int)(num * 100));
}

void HMI_SendPage(const char *page_name)
{
    printf("page %s\xff\xff\xff", page_name);
}

/* ── 触摸解析 ── */

/*
 *  prints 方式: 帧内容是纯文本, 以 \xFF\xFF\xFF 结尾
 *  例: "btn0_press" + FF FF FF
 *  约定: 文本格式为 "<控件名>_<事件>"
 *        press / release / click
 */
static int parse_prints(HMI_TouchEvent *evt)
{
    uint16_t len = USART_RX_STA & 0x3FFF;
    if (len < 4) return 0;   /* 太少忽略 */

    /* 截掉 FF FF FF, 得到纯文本 */
    char str[256];
    uint16_t text_len = len - 3;
    if (text_len >= sizeof(str)) text_len = sizeof(str) - 1;
    memcpy(str, USART_RX_BUF, text_len);
    str[text_len] = '\0';

    /* 查找 _press / _release 后缀 */
    char *suffix = NULL;
    uint8_t type = HMI_TOUCH_NONE;

    if ((suffix = strstr(str, "_press")) != NULL) {
        type = HMI_TOUCH_PRESS;
        *suffix = '\0';                          /* 截断得到控件名 */
    } else if ((suffix = strstr(str, "_release")) != NULL) {
        type = HMI_TOUCH_RELEASE;
        *suffix = '\0';
    } else if ((suffix = strstr(str, "_click")) != NULL) {
        type = HMI_TOUCH_PRESS;                  /* click 等同于 press */
        *suffix = '\0';
    }

    if (type == HMI_TOUCH_NONE) return 0;

    strncpy(evt->id, str, sizeof(evt->id) - 1);
    evt->type = type;
    return 1;
}

/*
 *  printh 方式: 帧内容是十六进制, 以 \xFF\xFF\xFF 结尾
 *  例: 0xAA 0x01 + FF FF FF
 *  协议自行约定, 这里示例: [0] = 控件ID, [1] = 事件 (01=press, 00=release)
 */
static int parse_printh(HMI_TouchEvent *evt)
{
    uint16_t len = USART_RX_STA & 0x3FFF;
    if (len < 5) return 0;   /* 至少 2 字节数据 + 3 字节结束符 */

    evt->raw[0] = USART_RX_BUF[0];
    evt->raw[1] = USART_RX_BUF[1];
    evt->raw[2] = USART_RX_BUF[2];

    if (evt->raw[1] == 0x01) {
        evt->type = HMI_TOUCH_PRESS;
    } else if (evt->raw[1] == 0x00) {
        evt->type = HMI_TOUCH_RELEASE;
    } else {
        return 0;
    }

    snprintf(evt->id, sizeof(evt->id), "btn_%02X", evt->raw[0]);
    return 1;
}

/*
 *  统一入口: 先试 prints 方式, 再试 printh 方式
 */
int HMI_ParseTouch(HMI_TouchEvent *evt)
{
    if (!(USART_RX_STA & 0x8000)) return 0;   /* 没有新帧 */

    memset(evt, 0, sizeof(*evt));

    if (parse_prints(evt)) return 1;
    if (parse_printh(evt)) return 1;

    return 0;   /* 不是触摸帧 (可能是 get 的回复等其他数据) */
}
