#include <Arduino.h>
#include <soc/ledc_reg.h>
#include <driver/ledc.h>
#include <stdint.h> // Include stdint.h for fixed-width data types
#include <Bounce2.h>

#define OUTPUT_PIN          21              
#define ADC_PIN             4              
#define MIN_PULSE_WIDTH     0  // 500 nanoseconds
#define MAX_PULSE_WIDTH     (uint32_t) 1000000 
#define STEP_WIDTH          100             // 100 nanosecond steps
#define NUM_READINGS        (uint8_t)50     // Number of ADC readings to average
#define BAUD_RATE           9600
#define ENABLE_PIN          34
#define DECREASE_PIN        25
#define INCREASE_PIN        26
#define DEBOUNCE_INTERVAL_MS  5
#define DEFAULT_PULSEWIDTH  500
#define DEFAULT_MAX_PULSEWIDTH  10000
#define DEFAULT_FREQ_HZ     1000

volatile bool ledcFallingEdgeDetected = false;
volatile uint16_t adcValues[NUM_READINGS] = {0};
volatile uint8_t adcIndex = 0;
volatile int32_t adcSum = 0;
volatile uint64_t isr_count = 0;
volatile bool isr_indicator_status = LOW;
volatile uint16_t pulseWidth = DEFAULT_PULSEWIDTH;
volatile bool ledcEnabled = false;
volatile bool transmitReading = false;
uint32_t averageMillivolts;

void updatePulseWidth();

void IRAM_ATTR onFallingEdge() {
  ledcFallingEdgeDetected = true;
  isr_count++;
  if(isr_count % 100 == 0)
    transmitReading = true;
}

Bounce enableButton = Bounce();
Bounce decreaseButton = Bounce();
Bounce increaseButton = Bounce();

void setup() {

  pinMode(OUTPUT_PIN, OUTPUT);
  Serial.begin(BAUD_RATE);

  // Configure LEDC timer
  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_16_BIT, // 16-bit duty resolution
    .timer_num = LEDC_TIMER_0,
    .freq_hz = DEFAULT_FREQ_HZ
  };
  ledc_timer_config(&ledc_timer);

  // Configure LEDC channel with a fixed duty cycle (e.g., 50%) for a 1ms period
  ledc_channel_config_t ledc_channel = {
    .gpio_num = OUTPUT_PIN,
    .speed_mode = LEDC_HIGH_SPEED_MODE, 
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0
  };
  ledc_channel_config(&ledc_channel);

  // Attach an interrupt to the falling edge of the LEDC signal
  attachInterrupt(OUTPUT_PIN, onFallingEdge, FALLING);

  // Configure Bounce objects (debounced push buttons)
  enableButton.attach(ENABLE_PIN, INPUT_PULLUP);
  enableButton.interval(DEBOUNCE_INTERVAL_MS);
  decreaseButton.attach(DECREASE_PIN, INPUT_PULLUP);
  decreaseButton.interval(DEBOUNCE_INTERVAL_MS);
  increaseButton.attach(INCREASE_PIN, INPUT_PULLUP);
  increaseButton.interval(DEBOUNCE_INTERVAL_MS);

  Serial.println("ESP32 Pulser setup passed!");
}


void loop() {

  enableButton.update();
  decreaseButton.update();
  increaseButton.update();

  static uint8_t enableButtonState = enableButton.read();
  static uint8_t decreaseButtonState = decreaseButton.read();
  static uint8_t increaseButtonState = increaseButton.read();

  if (ledcFallingEdgeDetected) {
  
    adcValues[adcIndex] = analogReadMilliVolts(ADC_PIN);

    // Update the sum with the new reading and subtract the oldest reading
    adcSum = adcSum + adcValues[adcIndex] - adcValues[(adcIndex + 1) % NUM_READINGS];

    // Calculate the average ADC value in millivolts
    averageMillivolts = adcSum / NUM_READINGS;

    // Increment the index with wrap-around
    adcIndex = (adcIndex + 1) % NUM_READINGS;
    
    ledcFallingEdgeDetected = false; // Reset the flag
  }
  
  // Do stuff based on button states
  if(increaseButton.fell() )
  {
    pulseWidth += STEP_WIDTH;
    if(pulseWidth > DEFAULT_MAX_PULSEWIDTH)
      pulseWidth = DEFAULT_MAX_PULSEWIDTH;
    updatePulseWidth();
    Serial.print("Pulse width: ");
    Serial.println(pulseWidth);
  }

  if(decreaseButton.fell())
  {
    pulseWidth -= STEP_WIDTH;
    if(pulseWidth < DEFAULT_PULSEWIDTH)
      pulseWidth = DEFAULT_PULSEWIDTH;
    updatePulseWidth();
    Serial.print("Pulse width: ");
    Serial.println(pulseWidth);
  }

  if(enableButton.fell())
  {
    ledcEnabled = !ledcEnabled;
    if(ledcEnabled)
    {
      ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, (pulseWidth * UINT16_MAX) / (MAX_PULSE_WIDTH));
      Serial.println("Pulse ENABLED");
    }
    else 
    { 
      ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
      Serial.println("Pulse DISABLED");
    }
  }
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

  if(transmitReading)
  {
    Serial.print(">ADC:");
    Serial.println(averageMillivolts);
    transmitReading = false;
  }
  
}

void updatePulseWidth()
{
  uint64_t duty = (pulseWidth * UINT16_MAX) / (MAX_PULSE_WIDTH);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
  if(ledcEnabled)
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}
