#include <Arduino.h>
#include <soc/ledc_reg.h>
#include <driver/ledc.h>
#include <stdint.h> // Include stdint.h for fixed-width data types
#include <Bounce2.h>

#include "pins.h"
#include "pulser.h"

#define NUM_READINGS (uint8_t)50 // Number of ADC readings to average
#define BAUD_RATE 115200
#define DEBOUNCE_INTERVAL_MS 5
#define INPUT_SIZE 30
#define DELAY 1 // supposedly uS

volatile bool ledcFallingEdgeDetected = false;
volatile uint16_t adcValues[NUM_READINGS] = {0};
volatile uint8_t adcIndex = 0;
volatile int32_t adcSum = 0;
volatile uint64_t isr_count = 0;
volatile bool isr_indicator_status = LOW;
volatile uint32_t pulseWidth = DEFAULT_PULSEWIDTH;
volatile bool ledcEnabled = false;
volatile bool transmitReading = false;
volatile bool flipbit = false;
uint32_t averageMillivolts;
hw_timer_t *timer = NULL;
volatile bool timerEnabled = false;
bool sampleinterruptdetected = false;
bool changed = false;
uint32_t freq = DEFAULT_FREQ_HZ;
char teststr[INPUT_SIZE + 1];
int i = -1;
int reg[4] = {1, 2, 3, 4};
int num = 666;
String outputstring;

void updatePulseWidth();
void set_ledc_timer();

// TODO: Faster ISR?
void IRAM_ATTR onFallingedge()
{ // on the rising edge of the LEDC pulse
    // on rising edge starts another timer alarm
    digitalWrite(TIMER_PIN, HIGH);
    sampleinterruptdetected = true;
    adcValues[adcIndex] = analogReadMilliVolts(PIN_ADC_1A);
    adcSum = adcSum + adcValues[adcIndex] - adcValues[(adcIndex + 1) % NUM_READINGS];
    averageMillivolts = adcSum / NUM_READINGS;
    adcIndex = (adcIndex + 1) % NUM_READINGS;
    digitalWrite(TIMER_PIN, LOW);
}

void setup()
{
    pinMode(PIN_PULSE_OUTPUT, OUTPUT);
    pinMode(TIMER_PIN, OUTPUT);
    Serial.begin(BAUD_RATE);

    initialize_pulser();
    // Attach an interrupt to the falling edge of the LEDC signal
    attachInterrupt(PIN_PULSE_OUTPUT, &onFallingedge, FALLING);

    Serial.println("ESP32 Pulser setup passed!");
}

void loop()
{
    // TODO: test time to run 5 ADC conversions back to back
    
    if (sampleinterruptdetected)
    {
        sampleinterruptdetected = false;
    }
    else
    {
        // TODO: Main menu FSM 
        byte size = Serial.readBytes(teststr, INPUT_SIZE);
        // Add the final 0 to end the C string
        teststr[size] = 0;
        
        outputstring = teststr;
        String tokens = "";
        if (size != 0)
        {
            // Serial.println(outputstring);
            //    remove any \r \n whitespace at the end of the String
            char *token = strtok(teststr, "     ");
            // Serial.println(token);
            tokens = token;
            tokens.trim();
            // Serial.println(tokens);
            i = 99;
            if (tokens.equals("freq"))
            {
                i = 1;
            }
            else if (tokens.equals("width"))
            {
                i = 2;
            }
            else if (tokens.equals("curr"))
            {
                i = 3;
            }
            else if (tokens.equals("onoff"))
            {
                i = 4;
            }
            else
                Serial.println("Error!");
            // Serial.print(i);
            token = strtok(NULL, "     ");
            // Serial.println(token);
            tokens = token;
            tokens.trim();
            if (tokens.length() > 0)
            {
                if (tokens.equals("?"))
                    Serial.println(reg[i - 1]);
                else
                {
                    num = tokens.toInt();
                    reg[i - 1] = num;
                    Serial.println("OK");
                    changed = true;
                }
            }
        }

        if (changed)
        {

            // here something has changed so lets reset everything....
            Serial.println("Somethings happening here...");
            freq = reg[0];
            if (freq < DEFAULT_MIN_FREQ)
                freq = DEFAULT_MIN_FREQ;
            if (freq > DEFAULT_MAX_FREQ)
                freq = DEFAULT_MAX_FREQ;
            reg[0] = freq;
            // Do stuff based on button states
            changed = false;
            pulseWidth = reg[1];
            if (pulseWidth < DEFAULT_MIN_PULSEWIDTH)
                pulseWidth = DEFAULT_MIN_PULSEWIDTH;
            if (pulseWidth > DEFAULT_MAX_PULSEWIDTH)
                pulseWidth = DEFAULT_MAX_PULSEWIDTH;
            reg[1] = pulseWidth;
            if (!reg[3])
                pulseWidth = 0;

            set_ledc_timer();     // set new ledc frequence
            updatePulseWidth(); // set new pulsewidth
        }
    }
}

void updatePulseWidth()
{
    uint64_t duty = (pulseWidth * UINT16_MAX) / (MAX_PULSE_WIDTH);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    if (ledcEnabled)
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}
