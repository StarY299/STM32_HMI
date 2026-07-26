#include "uart.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>

uint8_t  USART_RX_BUF[USART_REC_LEN];
uint16_t USART_RX_STA = 0;
static volatile uint8_t key_val = 0;    /* 按键 1~4 */
static uint8_t rx_byte;

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

/* ── 启动接收 ── */
void Uart_StartReceive(void)
{
    HAL_UART_Receive_IT(&huart6, &rx_byte, 1);
}

/* ── 读按键 ── */
uint8_t Uart_GetKey(void)
{
    uint8_t k;
    __disable_irq();
    k = key_val;
    key_val = 0;
    __enable_irq();
    return k;
}

/* ── 接收中断回调 ── */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART6) {
        HAL_UART_Receive_IT(huart, &rx_byte, 1);
        return;
    }

    uint8_t data = rx_byte;

    /* printh 单字节按键: 直接设 key_val */
    if (data >= 0x01 && data <= 0x04) {
        key_val = data;
    }

    /* 其他字节走缓冲 (兼容 FF FF FF 帧) */
    else {
        uint16_t idx = USART_RX_STA & 0x3FFF;
        if (USART_RX_STA & 0x8000) { USART_RX_STA = 0; idx = 0; }

        USART_RX_BUF[idx] = data;
        USART_RX_STA = (USART_RX_STA & 0xC000) | ((idx + 1) & 0x3FFF);

        if (idx >= 2 &&
            USART_RX_BUF[idx-2] == 0xFF &&
            USART_RX_BUF[idx-1] == 0xFF &&
            USART_RX_BUF[idx]   == 0xFF) {
            USART_RX_STA |= 0x8000;
        }

        if ((USART_RX_STA & 0x3FFF) >= USART_REC_LEN - 1)
            USART_RX_STA = 0;
    }

    HAL_UART_Receive_IT(&huart6, &rx_byte, 1);
}
