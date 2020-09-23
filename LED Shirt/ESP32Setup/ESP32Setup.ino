#include <arduinoFFT.h>      //https://github.com/kosme/arduinoFFT
//                           //https://github.com/G6EJD/ESP32-8266-Audio-Spectrum-Display

#define NUM_BANDS  8
#define READ_DELAY 50
#define USE_RANDOM_DATA false

//#include <WS2812FX.h>        //https://github.com/kitesurfer1404/WS2812FX
//#include "custom/VUMeter.h"  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

arduinoFFT FFT = arduinoFFT();

#define SAMPLES 512             //Must be a power of 2
#define SAMPLING_FREQUENCY 20000 //Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT.

// Use ADC1 so that WiFi stuff doesnt interfere with ADC measurements
#define ADC_PIN 34 // 36 = PIN VP on Lolin D32

int amplitude = 200;
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0, 0, 0, 0, 0, 0, 0, 0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;

// LED Settings
#include <FastLED.h>
#define LED_PIN     12
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define NUM_LEDS    112 //(12+56+56)
CRGB leds[NUM_LEDS];

//ADJUSTING
int brightnessFloor = 0;
int brightnessScaler = 1;

int brightnessMax, brightnessMin, aveBright, sumFreq;
int brightnessCount = 0;

int colour = 0;

#define WAVE 0
// 0: History
// 1: Flash
// 2: Pulse

#define GLOW 1
// 0: Frequency
// 1: Rainbow
// 2: Pink

int aveFreq = 0;

void displayBand(int band, int dsize)
{
  int dmax = amplitude;
  if (dsize > dmax)
    dsize = dmax;
  if (dsize > peak[band])
  {
    peak[band] = dsize;
  }
}

void setup()
{
  Serial.begin(115200);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  pinMode(ADC_PIN, INPUT);

  //ws2812fx.init();
  //ws2812fx.setBrightness(32);

  // setup the custom effect
  //uint32_t colors[] = {GREEN, YELLOW, RED};
  //uint8_t vuMeterMode = ws2812fx.setCustomMode(F("VU Meter"), vuMeter);
  //ws2812fx.setSegment(0, 0, LED_COUNT-1, vuMeterMode, colors, READ_DELAY, NO_OPTIONS);

  //LED SETUP
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(255);
}

void loop()
{
  for (int i = 0; i < SAMPLES; i++)
  {
    newTime = micros() - oldTime;
    oldTime = newTime;
    vReal[i] = analogRead(ADC_PIN); // A conversion takes about 1mS on an ESP8266
    vImag[i] = 0;
    while (micros() < (newTime + sampling_period_us))
    {
      delay(0);
    }
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  for (int i = 2; i < (SAMPLES / 2); i++)
  { // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
    if (vReal[i] > 2000)
    { // Add a crude noise filter, 10 x amplitude or more
      if (i <= 2)
        displayBand(0, (int)vReal[i] / amplitude); // 125Hz
      if (i > 3 && i <= 5)
        displayBand(1, (int)vReal[i] / amplitude); // 250Hz
      if (i > 5 && i <= 7)
        displayBand(2, (int)vReal[i] / amplitude); // 500Hz
      if (i > 7 && i <= 15)
        displayBand(3, (int)vReal[i] / amplitude); // 1000Hz
      if (i > 15 && i <= 30)
        displayBand(4, (int)vReal[i] / amplitude); // 2000Hz
      if (i > 30 && i <= 53)
        displayBand(5, (int)vReal[i] / amplitude); // 4000Hz
      if (i > 53 && i <= 200)
        displayBand(6, (int)vReal[i] / amplitude); // 8000Hz
      if (i > 200)
        displayBand(7, (int)vReal[i] / amplitude); // 16000Hz
    }
  }
  if (millis() % 4 == 0)
  {
    for (byte band = 0; band < NUM_BANDS; band++)
    {
      if (peak[band] > 0)
        peak[band] /= 2;
    }
  } // Decay the peak
  aveFreq = 0;
  sumFreq = 0;
  for (byte band = 0; band < (NUM_BANDS-2); band++)
  {
    uint16_t value = peak[band+1];
    sumFreq = sumFreq + value;
    aveFreq = aveFreq + (value*band);    

    Serial.print(value);
    Serial.print("\t");
  }
  
  if (sumFreq == 0) {
    aveFreq = 0;
  } else {
    aveFreq = (aveFreq*37)/sumFreq;
  }

  //aveBright = brightnessScaler*(sumFreq-brightnessFloor);
  //Scaling Brightness
  if (3*sumFreq > 255) {
    aveBright = 255;
  } else {
    aveBright = 3*sumFreq;
  }
  
  
  if(sumFreq < brightnessMin) brightnessMin = sumFreq;
  if(sumFreq > brightnessMax) brightnessMax = sumFreq;
  brightnessCount++;

  if(brightnessCount == 200) {
    brightnessCount = 0;
    brightnessFloor = brightnessMin;
    brightnessScaler = 255/(brightnessMax-brightnessMin);

    
    Serial.print(brightnessFloor);
    Serial.print("\t");
    Serial.print(brightnessScaler);
    
    
    brightnessMin = 100;
    brightnessMax = 0;
  }
  Serial.println();
  Serial.println(sumFreq);
  

  if(WAVE == 0) {
    for (int i = (NUM_LEDS-1); i > 0; i--) {
      leds[i] = leds[i-1];
    }
  } else if (WAVE == 1) {
    for (int i = 1; i < NUM_LEDS; i++) {
      leds[i] = leds[i-1];
    }
  } else if (WAVE == 2) {
    int len = (aveBright/256)*NUM_LEDS;
    if (len > NUM_LEDS) len = NUM_LEDS;
    for (int i = 1; i < len; i++) {
      leds[i] = leds[i-1];
    }
    for (int i = len; i < NUM_LEDS; i++) {
      leds[i] = CHSV(aveFreq, 255, 0);
    }
  }
 
  colour++;
  if(colour == 224) colour = 0;

  if (GLOW == 0) {
    leds[0] = CHSV(aveFreq, 255, aveBright);
  } else if (GLOW == 1) {
    leds[0] = CHSV(colour, 255, aveBright);
  } else if (GLOW == 2) {
    fill_solid( leds, NUM_LEDS, CRGB::Blue);
  }
  //Update leds
  FastLED.show();
}
