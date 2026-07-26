#include "dds.h"
#include "uart.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ============================================================
 * DDS 波形发生器实现
 * ============================================================ */

#define LUT_SIZE  256                     /* 查找表点数 */
#define PHASE_MAX 0xFFFFFFFF              /* 32 位相位累加器最大值 */

static const char* WAVE_NAMES[] = {
    "Sine", "Square", "Triangle", "Sawtooth"
};

static uint8_t  wave_lut[DDS_NUM_WAVES][LUT_SIZE];   /* 4 张波形表 */
static DDS_WaveType current_wave = DDS_SINE;
static uint32_t phase_acc  = 0;            /* 相位累加器 */
static uint32_t tune_word  = 0;            /* 频率调谐字 */

/* ── 生成查找表 ── */
static void gen_sine(uint8_t *buf)
{
    for (int i = 0; i < LUT_SIZE; i++) {
        double rad = 2.0 * M_PI * i / LUT_SIZE;
        /* sin 输出 [-1,1] → 映射到 [0,255], 中间值 = 128 */
        buf[i] = (uint8_t)(127.5 * sin(rad) + 127.5);
    }
}

static void gen_square(uint8_t *buf)
{
    for (int i = 0; i < LUT_SIZE; i++) {
        buf[i] = (i < LUT_SIZE / 2) ? 255 : 0;
    }
}

static void gen_triangle(uint8_t *buf)
{
    for (int i = 0; i < LUT_SIZE; i++) {
        /* 0→127: 0→255, 128→255: 255→0 */
        if (i < LUT_SIZE / 2)
            buf[i] = (uint8_t)(i * 510 / LUT_SIZE);           /* 0→255 */
        else
            buf[i] = (uint8_t)(510 - (i * 510 / LUT_SIZE));   /* 255→0 */
    }
}

static void gen_sawtooth(uint8_t *buf)
{
    for (int i = 0; i < LUT_SIZE; i++) {
        buf[i] = (uint8_t)(i * 255 / (LUT_SIZE - 1));
    }
}

/* ── 初始化 ── */
void DDS_Init(uint32_t target_freq_hz, uint32_t sample_rate_hz)
{
    /* 生成 4 张波形表 */
    gen_sine(wave_lut[DDS_SINE]);
    gen_square(wave_lut[DDS_SQUARE]);
    gen_triangle(wave_lut[DDS_TRIANGLE]);
    gen_sawtooth(wave_lut[DDS_SAWTOOTH]);

    /* 计算频率调谐字: tune_word = (freq / sample_rate) * PHASE_MAX */
    tune_word = (uint32_t)(((double)target_freq_hz / sample_rate_hz) * PHASE_MAX);
    phase_acc = 0;
    current_wave = DDS_SINE;
}

/* ── 切换波形 ── */
void DDS_SetWaveform(DDS_WaveType type)
{
    if (type >= DDS_NUM_WAVES) return;

    current_wave = type;
    phase_acc = 0;                      /* 切波形时相位归零, 画面清爽 */

    /* 通知屏清空波形显示通道 */
    printf("cle 1,0\xff\xff\xff");
}

DDS_WaveType DDS_GetWaveform(void)
{
    return current_wave;
}

const char* DDS_GetWaveformName(void)
{
    return WAVE_NAMES[current_wave];
}

/* ── 取下一个采样点 ── */
uint8_t DDS_GetNextSample(void)
{
    /* 查表: 取相位累加器的高 8 位作为索引 (0~255) */
    uint8_t  idx  = (uint8_t)(phase_acc >> 24);
    uint8_t  raw  = wave_lut[current_wave][idx];

    /* 推进相位 */
    phase_acc += tune_word;

    /* 波形组件高 200 像素, 缩放到 0~200 防止超出 */
    return (uint8_t)(raw * 200 / 255);
}
