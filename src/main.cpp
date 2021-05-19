#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN       6
#define COLOR_ORDER   GRB
#define CHIPSET       WS2811
#define NUM_LEDS_HIGH 8
#define NUM_LEDS_LENGHT 32
#define NUM_LEDS NUM_LEDS_HIGH*NUM_LEDS_LENGHT

#define BRIGHTNESS  100
#define FRAMES_PER_SECOND 10

bool gReverseDirection = false;

CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  60

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 80

void Fire2012WithPalette()
{
  static byte heat[NUM_LEDS_LENGHT][NUM_LEDS_HIGH];

  for( int i = 0; i < NUM_LEDS_LENGHT; i++) {
    // Step 1.  Cool down every cell a little
    for (size_t j = 0; j < NUM_LEDS_HIGH; j++){
      heat[i][j] = qsub8( heat[i][j],  random8(0, ((COOLING * 10) / NUM_LEDS_HIGH) + 2));
    }
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS_HIGH - 1; k >= 1; k--) {
      if (k<2){
        heat[i][k] = (heat[i][k - 1] * 2 + 0 )/ 3;
      } else {
        heat[i][k] = (heat[i][k - 1] + heat[i][k - 2] ) / 3;
      }
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int x = random8(NUM_LEDS_LENGHT);
      heat[x][0] = qadd8( heat[x][0], random8(160,255) );
    }
  }

  // Step 4.  Map from heat cells to LED colors
  for( int i = 0; i < NUM_LEDS_LENGHT; i++) {
    for( int j = 0; j < NUM_LEDS_HIGH; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[i][j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( i%2==1 ) {
        pixelnumber = i*NUM_LEDS_HIGH + (NUM_LEDS_HIGH-1) - j ;
      } else {
        pixelnumber = j + i*NUM_LEDS_HIGH;
      }
      leds[pixelnumber] = color;
    }
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

  gPal = HeatColors_p;
  
  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  
  // Third, here's a simpler, three-step gradient, from black to red to white
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

}

void loop()
{
  random16_add_entropy( random());
  Fire2012WithPalette(); // run simulation frame, using palette colors
  
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}



