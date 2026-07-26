#ifndef __DDS_H__
#define __DDS_H__

#include <stdint.h>

/* ============================================================
 * DDS 波形发生器
 *
 * 256 点查找表, 32 位相位累加器
 * 支持: 正弦 / 方波 / 三角 / 锯齿
 *
 * 用法:
 *   DDS_Init(1000);               // 初始化, 目标频率 1kHz
 *   DDS_SetWaveform(DDS_SINE);    // 选正弦波
 *   val = DDS_GetNextSample();    // 每次调用取一个点 (0~255)
 * ============================================================ */

/* 波形类型 (同时也是按键索引) */
typedef enum {
    DDS_SINE     = 0,
    DDS_SQUARE   = 1,
    DDS_TRIANGLE = 2,
    DDS_SAWTOOTH = 3,
    DDS_NUM_WAVES
} DDS_WaveType;

/* 初始化: 填入期望频率 (Hz) 和采样率 (Hz), 自动算 tuning word */
void DDS_Init(uint32_t target_freq_hz, uint32_t sample_rate_hz);

/* 切换波形, 同时自动通知屏清空波形通道 */
void DDS_SetWaveform(DDS_WaveType type);

/* 获取当前波形类型 */
DDS_WaveType DDS_GetWaveform(void);

/* 获取当前类型名称 */
const char* DDS_GetWaveformName(void);

/* 取下一个采样点 (0~255), 并推进相位 */
uint8_t DDS_GetNextSample(void);

#endif /* __DDS_H__ */
