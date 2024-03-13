/*
    filename:       pulser.c
    date:           08-Mar-2024
    description:    Implementation of pulse generator
                    on ESP32
*/

#include <Arduino.h>
#include <stdint.h>
#include <soc/ledc_reg.h>
#include <driver/ledc.h>
#include "pins.h"
#include "pulser.h"

static uint32_t pulse_width = 0;

bool initialize_pulser()
{
    // Configure LEDC timer0
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_16_BIT, // 16-bit duty resolution
        .timer_num = LEDC_TIMER_0,
        .freq_hz = DEFAULT_FREQ_HZ
    };
    ledc_timer_config(&ledc_timer);

    // Configure LEDC channel with initial 0% duty cycle
    ledc_channel_config_t ledc_channel = {
        .gpio_num = PIN_PULSE_OUTPUT,
        .speed_mode = LEDC_HIGH_SPEED_MODE, 
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&ledc_channel);

    return true;
}

bool enable_pulser()
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, (pulse_width * UINT16_MAX) / (MAX_PULSE_WIDTH));
}

// TODO: Better way do disable other than duty = 0?
bool disable_pulser()
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    return true;
}

bool update_pulse_width(uint32_t new_pulse_width)
{
    pulse_width = new_pulse_width;
    uint64_t duty = (pulse_width * UINT16_MAX) / (MAX_PULSE_WIDTH);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    return true;
}