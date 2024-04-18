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
#include <utils.h>

#define REF_LSB_REG 0x00
#define REF_MSB_REG 0x01
#define REF_MSB_REG_MASK 0x03
#define IOUT_LIMIT_REG 0x02
#define VOUT_FS_REG 0x04
#define VOUT_FS_REG_MASK 0x83
#define MODE_REG 0x06
#define STATUS_REG 0x07
#define VOUT_SR_REG 0x03
#define STATUS_BIT_MASK 0x03
#define OVP_BIT_MASK 0x20
#define OCP_BIT_MASK 0x40
#define SCP_BIT_MASK 0x80
#define OCP_DELAY_BIT 0x04
#define OCP_DELAY_NONE 0x00
#define OCP_DELAY_3MS 0x11
#define OE_BIT 7
#define DSCHG_BIT 4
#define VOUT_MIN_MV 800
#define VOUT_MAX_MV 20000
#define REF_MIN 0x0000 // Vout = 0.8V
#define REF_MAX 0x0780 // Vout = 20V

static uint16_t calculate_ref(uint32_t vout_mv);

void tps55289_initialize()
{
    Wire.begin();
    delay(500);
    Wire.beginTransmission(TPS55289_ADDR);

    // Set over current protection delay and keep other default
    // bit values in VOUT_SR_REG (see data sheet 7.6.3)
    Wire.write(VOUT_SR_REG);
    Wire.write(OCP_DELAY_3MS);
    Wire.endTransmission();

    tps55289_set_vout(0);
    tps55289_disable_output(); // not working for some reason
}

bool tps55289_enable_output()
{
    Wire.beginTransmission(TPS55289_ADDR);
    Wire.write(MODE_REG);
    
    // Sets output enable but also disables (writes 0) HICCUP mode.
    // See datasheet 7.6.6
    Wire.write(1 << OE_BIT);
    Wire.endTransmission();
    return true;
}

bool tps55289_disable_output()
{
    Wire.beginTransmission(TPS55289_ADDR);
    Wire.write(MODE_REG);
    Wire.write( ~(1 << OE_BIT) | (1 << DSCHG_BIT) );
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

// Reads that STATUS register and returns it
uint8_t tps55289_get_status()
{
    Wire.beginTransmission(TPS55289_ADDR);
    Wire.write(STATUS_REG);
    Wire.endTransmission();

    Wire.requestFrom(TPS55289_ADDR, 1);
    uint8_t status = Wire.read();
    return status;
}

// Reads the STATUS register and prints results
void tps55289_status_report()
{
    uint8_t status = tps55289_get_status();
    uint8_t status_bits = status & STATUS_BIT_MASK;
    uint8_t ovp_bit = (status & OVP_BIT_MASK) >> 5;
    uint8_t ocp_bit = (status & OCP_BIT_MASK) >> 6;
    uint8_t scp_bit = (status & SCP_BIT_MASK) >> 7;

    char status_str[4];
    int_to_hex_str(4, status, status_str);
    Serial.print("TPS55289 status: 0x");
    Serial.println(status_str);

    if(status_bits == 0)
        Serial.println("\tBoost mode");
    else if(status_bits == 0x1)
        Serial.println("\tBuck mode");
    else if(status_bits == 0x2)
        Serial.println("\tBuck-boost mode");

    if(ovp_bit)
        Serial.println("\t OVP ON");
    
    if(ocp_bit)
        Serial.println("\tOCP ON");

    if(scp_bit)
        Serial.println("\tSCP ON");

}

static uint16_t calculate_ref(uint32_t vout_mv)
{
    if (vout_mv < VOUT_MIN_MV)
    {
        vout_mv = VOUT_MIN_MV;
    }
    else if (vout_mv > VOUT_MAX_MV)
    {
        vout_mv = VOUT_MAX_MV;
    }
    uint32_t ref_value = ((vout_mv - VOUT_MIN_MV) * REF_MAX) / (VOUT_MAX_MV - VOUT_MIN_MV);
    char ref_str[4];
    int_to_hex_str(4, ref_value, ref_str);
    Serial.print("ref_value: 0x");
    Serial.print(ref_str);

    return (uint16_t)ref_value;
}