#include "tjc_screen.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>

/* ============================================================
 * 移植到其他工程只需改:
 *   1. huart6 → 你的串口句柄
 *   2. USART6  → 你的串口实例 (__io_putchar 里的寄存器)
 *   3. USART6  → 回调中断判断
 * ============================================================ */

#define RX_BUF_SIZE  256

static volatile uint8_t key_val = 0;
static uint8_t  rx_byte;
static uint8_t  rx_buf[RX_BUF_SIZE];
static uint16_t rx_sta = 0;

/* ── printf 重定向 ── */

int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart6, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

int __io_putchar(int ch)
{
    while (!(USART6->SR & USART_SR_TXE));
    USART6->DR = (uint8_t)ch;
    return ch;
}

/* ── 初始化 ── */
void TJC_Init(void)
{
    HAL_UART_Receive_IT(&huart6, &rx_byte, 1);
}

/* ── 读按键 ── */
uint8_t TJC_GetKey(void)
{
    uint8_t k;
    __disable_irq();
    k = key_val;
    key_val = 0;
    __enable_irq();
    return k;
}

/* ── 发送 ── */
void TJC_SetPage(const char *page)         { printf("page %s\xff\xff\xff", page); }
void TJC_SetText(const char *n, const char *t) { printf("%s=\"%s\"\xff\xff\xff", n, t); }
void TJC_SetNum(const char *n, int v)      { printf("%s=%d\xff\xff\xff", n, v); }
void TJC_WaveAdd(int w, int c, int v)      { printf("add %d,%d,%d\xff\xff\xff", w, c, v); }
void TJC_WaveClear(int w, int c)           { printf("cle %d,%d\xff\xff\xff", w, c); }

/* ── RX 中断回调 ── */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART6) {
        HAL_UART_Receive_IT(huart, &rx_byte, 1);
        return;
    }

    uint8_t data = rx_byte;

    /* printh 单字节按键: 0x01~0x04, 直接设 key_val */
    if (data >= 0x01 && data <= 0x04) {
        key_val = data;
    }
    /* 其他字节走 FFFFFF 帧缓冲 */
    else {
        uint16_t idx = rx_sta & 0x3FFF;
        if (rx_sta & 0x8000) { rx_sta = 0; idx = 0; }

        rx_buf[idx] = data;
        rx_sta = (rx_sta & 0xC000) | ((idx + 1) & 0x3FFF);

        if (idx >= 2 && rx_buf[idx-2] == 0xFF &&
            rx_buf[idx-1] == 0xFF && rx_buf[idx] == 0xFF)
            rx_sta |= 0x8000;

        if ((rx_sta & 0x3FFF) >= RX_BUF_SIZE - 1) rx_sta = 0;
    }

    HAL_UART_Receive_IT(&huart6, &rx_byte, 1);
}
