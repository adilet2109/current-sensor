/*
    filename:       tps55889.h
    date:           21-Mar-2024
    description:    Header forTPS55289 I2C programmable
                    power supply IC driver in Laser Pulse Supply 
                    project
*/

#ifndef TPS55289_H
#define TPS55289_H

#define TPS55289_ADDR   0x74

#include <stdint.h>
#include <stdbool.h>

void tps55289_initialize();
bool tps55289_enable_output();
bool tps55289_disable_output();
bool tps55289_set_vout(uint16_t vout_set_mv);

#endif
