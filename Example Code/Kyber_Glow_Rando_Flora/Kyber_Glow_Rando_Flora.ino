#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip[2];
uint8_t pins[2] = {6, 9};

void setup() {

 for (int x = 0; x <= 1; x++) {
    strip[x].updateLength(3);
    strip[x].updateType(NEO_GRB + NEO_KHZ800);
    strip[x].setPin(pins[x]);
  }
  
  for (int x = 0; x <= 1 ; x++) {
    strip[x].begin();
    strip[x].show();
    for (int p = 0; p <= 2; p++) {
      strip[x].setPixelColor(p, strip[x].Color(255,255,255));
    }
    strip[x].setBrightness(25);
    strip[x].show();
  }
  randomSeed(analogRead(0));
}

void loop() {
    // Pulse all Pixels
  colorPulse();
}

void colorPulse() {
  int stripNumber = random(0,2);
  int pixelNumber = random(0,3);
  int rColor = random(5,21);
  int bColor = random(100,256);
  int iDelay = random(50,201);
  int iBright = random(75,256);

  strip[stripNumber].setPixelColor(pixelNumber, strip[stripNumber].Color(rColor, 255, bColor));
  delay(iDelay);
  strip[stripNumber].setBrightness(iBright);
  strip[stripNumber].show();
}
