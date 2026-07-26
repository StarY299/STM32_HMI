#ifndef __UART_H__
#define __UART_H__
#include <stdint.h>

#define USART_REC_LEN   128

void Uart_StartReceive(void);
uint8_t Uart_GetKey(void);       /* 返回 1~4, 无按键返回 0 */

#endif
