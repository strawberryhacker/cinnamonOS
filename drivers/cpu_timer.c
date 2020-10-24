/// Copyright (C) strawberryhacker

#include <cinnamon/cpu_timer.h>
#include <cinnamon/clock.h>
#include <regmap.h>

void cpu_timer_init(void)
{
    clk_pck_enable(36);

    // Using PCLK @ 83 MHz divided by 8 yielding 648 kHz
    TIMER1->channel[0].CMR = (1 << 14) | 1;
    TIMER1->channel[0].RC = 10375;
    TIMER1->channel[0].IER = (1 << 4);
}

void cpu_timer_clear_flags(void)
{
    (void)TIMER1->channel[0].SR;
}

void cpu_timer_reset(void)
{
    TIMER1->channel[0].CCR = (1 << 2);
}

u32 cpu_timer_get_value(void)
{
    return TIMER1->channel[0].CV;
}

void cpu_timer_start(void)
{
    TIMER1->channel[0].CCR = (1 << 0) | (1 << 2);
}