/*
    filename:       tps55889.c
    date:           21-Mar-2024
    description:    Implementation of TPS55289 I2C programmable
                    power supply IC driver in Laser Pulse Supply 
                    project
*/

#include <tps55289.h>
#include "pins.h"
#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>
#include <Wire.h>
#include <tps55289.h>

#define REF_LSB_REG         0x00
#define REF_MSB_REG         0x01 
#define REF_MSB_REG_MASK    0x03 
#define IOUT_LIMIT_REG      0x02 
#define VOUT_FS_REG         0x04
#define VOUT_FS_REG_MASK    0x83 
#define MODE_REG            0x06
#define STATUS_REG          0x07
#define OE_BIT              7
#define DSCHG_BIT           4
#define VOUT_MIN_MV         800
#define VOUT_MAX_MV         20000
#define REF_MIN             0x0000  // Vout = 0.8V
#define REF_MAX             0x0780  // Vout = 20V

static uint16_t calculate_ref(uint32_t vout_mv);


void tps55289_initialize()
{
    Wire.begin();
    //tps55289_set_vout(0);
    //tps55289_disable_output(); // not working for some reason
}

bool tps55289_enable_output()
{
    Wire.beginTransmission(TPS55289_ADDR);
    Wire.write(MODE_REG);
    Wire.write(1 << OE_BIT);
    Wire.endTransmission();
    return true;
}

bool tps55289_disable_output()
{
    Wire.beginTransmission(TPS55289_ADDR);
    Wire.write(MODE_REG);
    Wire.write(1<<DSCHG_BIT);
    Wire.endTransmission();
    return true;
}

bool tps55289_set_vout(uint16_t vout_set_mv)
{
    uint16_t ref_value = calculate_ref(vout_set_mv);
    Wire.beginTransmission(TPS55289_ADDR);
    Wire.write(REF_LSB_REG);
    Wire.write(ref_value & 0x00ff);
    Wire.write(ref_value >> 8);
    Wire.endTransmission();
    return true;
}

static uint16_t calculate_ref(uint32_t vout_mv) {
    
    if (vout_mv < VOUT_MIN_MV) 
    {
        vout_mv = VOUT_MIN_MV; 
    } 
    else if (vout_mv > VOUT_MAX_MV) 
    {
        vout_mv = VOUT_MAX_MV;
    }

    uint32_t ref_value = ((vout_mv - VOUT_MIN_MV) * REF_MAX) / (VOUT_MAX_MV - VOUT_MIN_MV);
    
    Serial.print("ref_value: ");
    Serial.println(ref_value);

    return (uint16_t)ref_value;
}