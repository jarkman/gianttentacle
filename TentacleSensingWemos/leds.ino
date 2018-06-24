#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN D6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60 * 5, PIN, NEO_BGR + NEO_KHZ400);

#define CHEAP_STRIP_LEVELS 16.0 // my cheap LED strip only uses the bottom 4 bits!

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.



float timeOffset = 0; // 0 to 1 over a cycle
float scrollSpeed = 0.05; // how fast does the pattern move, in seconds per phase
float ledsPerCycle = 60;

void setupLeds() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  colorWipe(strip.Color(255, 0, 0), 1); // r   
  colorWipe(strip.Color(0, 255, 0), 1); // g
  colorWipe(strip.Color(0, 0,255), 1); // b
  /*
  for( int i = 0; i < CHEAP_STRIP_LEVELS; i ++ )
  {
    colorWipe(strip.Color(0, 0,i), 1); // b
    yield();
  }
  */
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void loopLeds()
{
    
    yield();
    
    for(uint16_t i=0; i<strip.numPixels(); i++) 
    {
      
      int r = 0;

      float phase = (float) (i % (int) ledsPerCycle)/ledsPerCycle; // 0->1.0

      phase = fmod( phase + timeOffset, 1.0  ); // move it along
      
      float ig = (sin( 2.0 * 3.14 * phase ) + 1.0) / 2.0;
      //float ib = 1.0 - ig; //(cos( 2.0 * 3.14 * phase ) + 1.0) / 2.0;
      float ib = (cos( 2.0 * 3.14 * phase ) + 1.0) / 2.0;
      ig = 0.7 * ig + 0.3;
      ib = 0.7 * ib + 0.3;
      
      uint32_t c = strip.Color(r, (int) (CHEAP_STRIP_LEVELS * ig),(int) (CHEAP_STRIP_LEVELS * ib));
      strip.setPixelColor(i, c);
    }
    strip.show();
    
    timeOffset = fmod( timeOffset + loopSeconds * scrollSpeed , 1.0 );
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
