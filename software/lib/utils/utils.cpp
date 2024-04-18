/*
    filename:       utils.c
    date:           9-Apr-2024
    description:    Various utility functions
*/

#include <stdint.h>
#include <utils.h>

void int_to_hex_str(uint8_t num_digits, uint32_t value, char * hex_string)
{
    const char * hex_chars = "0123456789ABCDEF";

    for(int8_t i = num_digits - 1; i >= 0; i--)
    {
        *(hex_string + i) = *(hex_chars + (value & 0xf));
        value >>= 4;
    } 
}
