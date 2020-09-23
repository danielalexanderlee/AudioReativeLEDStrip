#define OCTAVE 1 //   // Group buckets into octaves  (use the log output function LOG_OUT 1)
#define OCT_NORM 0 // Don't normalise octave intensities by number of bins
#define SCALE 128
#include <FHT.h>

#include <FastLED.h>
#define LED_PIN     52
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define NUM_LEDS    112 //(12+56+56)
CRGB leds[NUM_LEDS];

#define WAVE 0
// 0: History
// 1: Flash

#define GLOW 0
// 0: Frequency
// 1: Rainbow
// 2: Pink

//variable to store incoming audio sample
byte incomingAudio;

//clipping indicator variables
boolean clipping = 0;

//ADJUSTING
int brightnessFloor = 0;
int brightnessScaler = 30;

int brightnessMax, brightnessMin;
int brightnessCount = 0;

int count = 0;
unsigned int maxIndex = 1;
int brightness = 0;
int colour = 0;

const int maLength = 20;
int maIndex = 0;
int Freq[maLength];
int aveFreq = 0;
int Bright[maLength];
int aveBright = 0;

//int noise[] = {94, 82, 19, 36, 39, 46, 53, 47};
//int noise[] = {99, 85, 36, 48, 49, 51, 56, 51};
//int noise[] = {111,  100, 53,  52,  50,  50,  55,  52};  
//int noise[] = {101,  88,  38,  51,  56,  53,  56,  52};
int noise[] = {93,  81,  27, 36,  42,  49,  52,  55};
//int noise[] = {0, 0, 0, 0, 0, 0, 0, 0};

int scaling[] = {1, 1, 1, 1, 1, 1, 1, 1};

int denoised[] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup(){
  Serial.begin(115200);
  // ADC SETUP
  cli();//disable interrupts
  
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  sei();//enable interrupts

  //LED SETUP
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(255);
}

//when new ADC value ready
ISR(ADC_vect) {
  incomingAudio = ADCH;//Fetch 8 bit value from analog pin 0
  //Serial.println(incomingAudio);
  
  if (incomingAudio == 0 || incomingAudio == 255){//if clipping
    digitalWrite(LED_BUILTIN, HIGH);//set pin 13 high
    clipping = 1;//currently clipping
  }
  // S
  fht_input[count] = incomingAudio;
  if(count >= 254) {
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_octave(); // take the output of the fht  fht_mag_log()

    for (int i = 0; i < 8; i++) {
      denoised[i] = (fht_oct_out[i] - noise[i])*scaling[i];
      if(denoised[i] < 0) denoised[i] = 0;
      //Serial.print(denoised[i]);
      //Serial.print("\t");
    }
    //Serial.println();
    maxIndex = 0;
    brightness = 0;
    for (int i = 2; i < 8; i++) {
      maxIndex = maxIndex + denoised[i]*i;
      brightness = brightness + denoised[i];
    }

    Freq[maIndex] = (maxIndex/brightness)-2;
    Bright[maIndex] = brightness;
    maIndex++;
    if(maIndex == maLength) maIndex = 0;
    
    int masum = 0;
    int masum2 = 0;
    for (int i = 0; i < maLength; i++) {
      masum = masum + Freq[i];
      masum2 = masum2 + Bright[i];
    }
    aveFreq = (3*32*masum)/maLength;
    aveBright = (((masum2)/maLength)-brightnessFloor);
    if(aveBright<0) aveBright=0;
    if(aveBright>254) aveBright=254;

    /*
    Serial.print(aveBright);
    Serial.print("\t");
    Serial.println(aveFreq);
    */ 
    count = 0;
  } else {
    //Serial.println(count);
    count++;
  }
}

void loop(){
  if (clipping){//if currently clipping
    clipping = 0;//
    digitalWrite(LED_BUILTIN, LOW);//turn off clipping led indicator (pin 13)
  }
  if(WAVE == 0) {
    for (int i = (NUM_LEDS-1); i > 0; i--) {
      leds[i] = leds[i-1];
    }
  } else if (WAVE == 1) {
    for (int i = 1; i < NUM_LEDS; i++) {
      leds[i] = leds[i-1];
    }
  }

  //Scaling Brightness
  
  if(aveBright < brightnessMin) brightnessMin = aveBright;
  if(aveBright > brightnessMax) brightnessMax = aveBright;
  brightnessCount++;

  if(brightnessCount == 500) {
    brightnessCount = 0;
    brightnessFloor = brightnessMin;
    brightnessScaler = 255/(brightnessMax-brightnessMin);

    
    Serial.print(brightnessFloor);
    Serial.print("\t");
    Serial.println(brightnessScaler);
    
    
    brightnessMin = 10;
    brightnessMax = 0;
  }
  
  colour++;
  if(colour == 224) colour = 0;

  if (GLOW == 0) {
    leds[0] = CHSV(aveFreq, 255, aveBright*brightnessScaler);
  } else if (GLOW == 1) {
    leds[0] = CHSV(colour, 255, aveBright*brightnessScaler);
  } else if (GLOW == 2) {
    fill_solid( leds, NUM_LEDS, CRGB::Blue);
  }
  //Update leds
  FastLED.show();
  //delay(10);
}
