//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
// OlliW @ www.olliw.eu
//*******************************************************
// Buzzer
//********************************************************
#ifndef BUZZER_H
#define BUZZER_H
#pragma once


#ifndef DEVICE_HAS_BUZZER

class tBuzzer
{
  public:
    void Init(void) {}
    void BeepLP(void) {}
    void BeepLQ(uint8_t LQ) {}
};

#else


#include <stdlib.h>
#include <ctype.h>


class tBuzzer
{
  public:
    void Init(void);
    void BeepLP(void);
    void BeepLQ(uint8_t LQ);

    void beep_init(void);
    void beep_off(void);
    void beep_on(uint32_t freq_hz);
    void beep(uint32_t freq_hz, uint32_t duration_ms);
    bool is_beeping(void);
};


void tBuzzer::Init(void)
{
    beep_init();
}


void tBuzzer::BeepLP(void)
{
    if (is_beeping()) return; // previous beep still running

    beep(500, 50);
}


void tBuzzer::BeepLQ(uint8_t LQ)
{
    if (LQ == 0) { beep_on(500); return; }

    if (LQ < 25) {
        beep(500, 500);
    } else
    if (LQ < 75) {
        beep(500, 250);
    } else
    if (LQ < 100) {
        beep(500, 50);
    } else {
        beep_off();
    }
}


//-------------------------------------------------------
// Low-level beep implementation
//-------------------------------------------------------

IRQHANDLER(
void BUZZER_IRQHandler(void)
{
    LL_TIM_ClearFlag_UPDATE(BUZZER_TIMx);
    LL_TIM_DisableIT_UPDATE(BUZZER_TIMx);
    LL_TIM_DisableCounter(BUZZER_TIMx);
})


void tBuzzer::beep_init(void)
{
uint32_t ll_tim_channel_ch;

    gpio_init(BUZZER, IO_MODE_OUTPUT_ALTERNATE_PP, IO_SPEED_SLOW);

    tim_config_up(BUZZER_TIMx, 1000, TIMER_BASE_1US);

    // LL_TIM_OC_Init() and LL_TIM_OC_DisableFast() only allow
    // LL_TIM_CHANNEL_CHx, even if LL_TIM_CHANNEL_CHxN is used
    // so we need to cover up for this here
    switch (BUZZER_TIM_CHANNEL) {
    case LL_TIM_CHANNEL_CH1N: ll_tim_channel_ch = LL_TIM_CHANNEL_CH1; break;
    case LL_TIM_CHANNEL_CH2N: ll_tim_channel_ch = LL_TIM_CHANNEL_CH2; break;
    case LL_TIM_CHANNEL_CH3N: ll_tim_channel_ch = LL_TIM_CHANNEL_CH3; break;
    default:
        ll_tim_channel_ch = BUZZER_TIM_CHANNEL;
    }

    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.CompareValue = 0;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
    TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_LOW;
    TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_HIGH;
    LL_TIM_OC_Init(BUZZER_TIMx, ll_tim_channel_ch, &TIM_OC_InitStruct);

    LL_TIM_OC_DisableFast(BUZZER_TIMx, ll_tim_channel_ch);

    if (BUZZER_TIMx == TIM1) {
        LL_GPIO_AF_RemapPartial_TIM1();
        LL_TIM_EnableAllOutputs(BUZZER_TIMx);
    }

    //LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3N);
    //LL_TIM_EnableCounter(TIM1);

    nvic_irq_enable_w_priority(BUZZER_IRQn, BUZZER_TIM_IRQ_PRIORITY);
}


void tBuzzer::beep_off(void)
{
    LL_TIM_ClearFlag_UPDATE(BUZZER_TIMx);
    LL_TIM_DisableIT_UPDATE(BUZZER_TIMx);
    LL_TIM_DisableCounter(BUZZER_TIMx);
}


void tBuzzer::beep_on(uint32_t freq_hz)
{
    uint32_t buzzer_period_us = 1000000 / freq_hz;

    LL_TIM_SetAutoReload(BUZZER_TIMx, buzzer_period_us);
    LL_TIM_OC_SetCompareCH3(BUZZER_TIMx, buzzer_period_us / 2);

    LL_TIM_EnableCounter(BUZZER_TIMx);
    LL_TIM_CC_EnableChannel(BUZZER_TIMx, BUZZER_TIM_CHANNEL);
    // LL_TIM_EnableIT_UPDATE(BUZZER_TIMx);
}


void tBuzzer::beep(uint32_t freq_hz, uint32_t duration_ms)
{
    uint32_t buzzer_period_us = 1000000 / freq_hz;

    uint32_t buzzer_cnt = (duration_ms * freq_hz) / 1000;

    LL_TIM_SetAutoReload(BUZZER_TIMx, buzzer_period_us);
    LL_TIM_OC_SetCompareCH3(BUZZER_TIMx, buzzer_period_us / 2);
    LL_TIM_SetRepetitionCounter(BUZZER_TIMx, buzzer_cnt);

    LL_TIM_EnableCounter(BUZZER_TIMx);
    LL_TIM_CC_EnableChannel(BUZZER_TIMx, BUZZER_TIM_CHANNEL);
    LL_TIM_EnableIT_UPDATE(BUZZER_TIMx);
}


bool tBuzzer::is_beeping(void)
{
    return LL_TIM_IsEnabledCounter(BUZZER_TIMx);
}


#endif // DEVICE_HAS_BUZZER

#endif // BUZZER_H



