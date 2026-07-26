#include "delay.h"
#include "math.h"
#include "string.h"
#include "sys.h"
#include "usart.h"

// ALIENTEK Mini STM32开发板范例代码3
// 串口实验
// 技术支持：www.openedv.com
// 广州市星翼电子科技有限公司
int test_num = 0;
float test_float = 0;
void HMI_send_string(char* name, char* showdata)
{
    // printf("t0.txt=\"%d\"\xff\xff\xff", num);
    printf("%s=\"%s\"\xff\xff\xff", name, showdata);
}
void HMI_send_number(char* name, int num)
{
    // printf("t0.txt=\"%d\"\xff\xff\xff", num);
    printf("%s=%d\xff\xff\xff", name, num);
}
void HMI_send_float(char* name, float num)
{
    // printf("t0.txt=\"%d\"\xff\xff\xff", num);
    printf("%s=%d\xff\xff\xff", name, (int)(num * 100));
}
void HMI_Wave(char* name, int ch, int val)
{
    printf("add %s,%d,%d\xff\xff\xff", name, ch, val);
}
void HMI_Wave_Fast(char* name, int ch, int count, int* show_data)
{
    int i;
    printf("addt %s,%d,%d\xff\xff\xff", name, ch, count);
    delay_ms(100);
    for (i = 0; i < count; i++)
        printf("%c", show_data[i]);
    printf("\xff\xff\xff");
}
void HMI_Wave_Clear(char* name, int ch)
{
    printf("cle %s,%d\xff\xff\xff", name, ch);
}
int main(void)
{
    u8 t;
    u8 len;
    u16 times = 0;
    int i;
    delay_init(); // 延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
    uart_init(9600); // 串口初始化为9600
    while (1) {
        if (USART_RX_STA & 0x8000) {
            len = USART_RX_STA & 0x3fff; // 得到此次接收到的数据长度
            if (USART_RX_BUF[0] == 0xfd && USART_RX_BUF[1] == 0xff
                && USART_RX_BUF[2] == 0xff && USART_RX_BUF[3] == 0xff) {
                memset(USART_RX_BUF, 0, 200); // 清空缓存区
                printf("OKK");
            }
            if (USART_RX_BUF[0] == '1') // 0x31
            {
                HMI_send_string("t0.txt", "test_num");
                // printf("t0.txt=\"%d\"\xff\xff\xff", test_num);
            }
            if (USART_RX_BUF[0] == '2') // 0x32
            {
                test_float = test_float + 0.3;
                HMI_send_float("x0.val", test_float);
                // printf("t0.txt=\"%d\"\xff\xff\xff", test_num);
            }
            if (USART_RX_BUF[0] == '3') // 0x33
            {
                test_num++;
                HMI_send_number("n0.val", test_num);
                // printf("t0.txt=\"%d\"\xff\xff\xff", test_num);
            }
            if (USART_RX_BUF[0] == '4') // 0x34
            {
                int sin_data[255];
                for (i = 0; i < 255; i++) {
                    sin_data[i] = (int)((sin((i + 1) * 3.14 / 50) + 1) * 90);
                    HMI_Wave("s0.id", 0, sin_data[i]);
                }
            }
            if (USART_RX_BUF[0] == '5') // 0x35
            {
                int sin_data[255];
                for (i = 0; i < 255; i++)
                    sin_data[i] = (int)((sin((i + 1) * 3.14 / 50) + 1) * 90);
                HMI_Wave_Fast("s0.id", 0, 255, sin_data);
            }
            if (USART_RX_BUF[0] == '6') // 0x36
            {
                HMI_Wave_Clear("s0.id", 0);
            }
            USART_RX_STA = 0;
        } else { /*
             times++;
             if (times % 10000 == 0)
                 printf("demo\r\n");*/
        }
    }
}
