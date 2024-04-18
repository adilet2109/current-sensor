#include <Arduino.h>
#include <soc/ledc_reg.h>
#include <driver/ledc.h>
#include <stdint.h> // Include stdint.h for fixed-width data types
#include <Bounce2.h>

#define OUTPUT_PIN 33
#define TIMER_PIN 32
#define ADC_PIN 4
#define MIN_PULSE_WIDTH 0 // 500 nanoseconds
#define MAX_PULSE_WIDTH (uint32_t)1000000
#define STEP_WIDTH 100           // 100 nanosecond steps
#define NUM_READINGS (uint8_t)50 // Number of ADC readings to average
#define BAUD_RATE 115200
#define ENABLE_PIN 34
#define DECREASE_PIN 25
#define INCREASE_PIN 26
#define DEBOUNCE_INTERVAL_MS 5
#define DEFAULT_PULSEWIDTH 7000
#define DEFAULT_MAX_PULSEWIDTH 1000000
#define DEFAULT_MIN_PULSEWIDTH 500
#define DEFAULT_FREQ_HZ 1000
#define DEFAULT_MAX_FREQ 10000
#define DEFAULT_MIN_FREQ 100
#define INPUT_SIZE 30
#define DELAY 1 // supposedly uS
volatile bool ledcFallingEdgeDetected = false;
volatile uint16_t adcValues[NUM_READINGS] = {0};
volatile uint8_t adcIndex = 0;
volatile int32_t adcSum = 0;
volatile uint64_t isr_count = 0;
volatile bool isr_indicator_status = LOW;
volatile uint16_t pulseWidth = DEFAULT_PULSEWIDTH;
volatile bool ledcEnabled = false;
volatile bool transmitReading = false;
volatile bool flipbit = false;
uint32_t averageMillivolts;
hw_timer_t *timer = NULL;
volatile bool timerEnabled = false;
bool sampleinterruptdetected = false;
bool changed = false;
int32_t freq = DEFAULT_FREQ_HZ;
char teststr[INPUT_SIZE + 1];
int i = -1;
int reg[4] = {1, 2, 3, 4};
int num = 666;
String outputstring;

void IRAM_ATTR onFallingedge()
{ // on the rising edge of the LEDC pulse
  // on rising edge starts another timer alarm
  digitalWrite(TIMER_PIN, HIGH);
  sampleinterruptdetected = true;
  adcValues[adcIndex] = analogReadMilliVolts(ADC_PIN);
  adcSum = adcSum + adcValues[adcIndex] - adcValues[(adcIndex + 1) % NUM_READINGS];
  averageMillivolts = adcSum / NUM_READINGS;
  adcIndex = (adcIndex + 1) % NUM_READINGS;
  digitalWrite(TIMER_PIN, LOW);
}

void setup()
{

  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(TIMER_PIN, OUTPUT);

  Serial.begin(BAUD_RATE);
  // analogSetCycles(1);

  set_ledc_timer();
  Serial.println("ESP32 Pulser setup passed!");
}

void set_ledc_timer()
{
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .duty_resolution = LEDC_TIMER_16_BIT, // 16-bit duty resolution
      .timer_num = LEDC_TIMER_0,
      .freq_hz = freq};
  ledc_timer_config(&ledc_timer);

  // Configure LEDC channel with a fixed duty cycle (e.g., 50%) for a 1ms period
  ledc_channel_config_t ledc_channel = {
      .gpio_num = OUTPUT_PIN,
      .speed_mode = LEDC_HIGH_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0};
  ledc_channel_config(&ledc_channel);
  updatePulseWidth();

  // Attach an interrupt to the rising edge of the LEDC signal
  attachInterrupt(OUTPUT_PIN, &onFallingedge, FALLING);
}

void loop()
{

  // Serial.println("Loopy");

  //

  if (sampleinterruptdetected)
  {
    sampleinterruptdetected = false;
  }
  else
  {
    byte size = Serial.readBytes(teststr, INPUT_SIZE);
    // Add the final 0 to end the C string
    teststr[size] = 0;
    outputstring = teststr;
    String tokens = "";
    if (size != 0)
    {
      // Serial.println(outputstring);
      //  remove any \r \n whitespace at the end of the String
      char *token = strtok(teststr, "   ");
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
      token = strtok(NULL, "   ");
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

      set_ledc_timer();   // set new ledc frequence
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
