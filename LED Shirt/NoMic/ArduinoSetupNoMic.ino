// LED Settings
#include <FastLED.h>
#define LED_PIN     12
#define LED_TYPE    WS2812
#define COLOR_ORDER RGB

#define NUM_LEDS    112 //(12+56+56)
CRGB leds[NUM_LEDS];

int colour = 0;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  
  //LED SETUP
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  fill_solid(leds, NUM_LEDS, CRGB(0,0,0));         // fill all black
  FastLED.show();  
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  colour++;
  if(colour == 224) colour = 0;
  Serial.println(colour);

  //leds[0] = CHSV(colour, 255, 255);
  leds[0] = CRGB::Blue;
  
  for (int i = (NUM_LEDS-1); i > 0; i--) {
      leds[i] = leds[i-1];
    }

  delay(100);
  //Update leds
  FastLED.show();
}
