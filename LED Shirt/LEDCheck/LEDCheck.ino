#include <FastLED.h>
#define LED_PIN     5
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define NUM_LEDS    240
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
}

int colorx = 0;
void loop() {
  // put your main code here, to run repeatedly:
  
  
  leds[0] = CRGB::Red;
  for(int i = 1; i < NUM_LEDS; i++) {
    leds[i] = leds[i-1];
  }

  colorx++;
  if(colorx > 100) colorx = 0;

  FastLED.show();
}
