/*
    filename:       pulser.h
    date:           08-Mar-2024
    description:    Header file for implementation of pulse generator
                    on ESP32
*/

#ifndef PULSER_H
#define PULSER_H

#include <stdint.h>
#include <stdbool.h>

#define MIN_PULSE_WIDTH 0 // 500 nanoseconds
#define MAX_PULSE_WIDTH (uint32_t)1000000
#define STEP_WIDTH 100U // 100 nanosecond steps
#define DEFAULT_PULSEWIDTH 7000U
#define DEFAULT_MAX_PULSEWIDTH 1000000U
#define DEFAULT_MIN_PULSEWIDTH 500U
#define DEFAULT_FREQ_HZ 1000U
#define DEFAULT_MAX_FREQ 10000U
#define DEFAULT_MIN_FREQ 100U

bool initialize_pulser();
bool enable_pulser();
bool disable_pulser();
bool update_pulse_width(uint32_t pulse_width);

#endif