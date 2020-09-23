#include "driver/adc.h"
#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t *adc_chars;
static adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

#define NO_OF_SAMPLES   4          //Multisampling


int newTime, oldTime;
uint32_t adc_reading;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  oldTime = micros();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t adc_reading = 0;
  //Multisampling
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw((adc1_channel_t)channel);
  }
  adc_reading /= NO_OF_SAMPLES;


  //adc_reading = adc1_get_raw((adc1_channel_t)channel);
  //Convert adc_reading to voltage in mV
  //uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
  Serial.println(adc_reading);
}
