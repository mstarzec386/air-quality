#include <Arduino.h>
#include <Wire.h>

#include "OLEDDisplayUi.h"
#include "SSD1306Wire.h"
#include <SDS011.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D6
#define BUTTON D4

// Include custom images
#include "images.h"

int battery;
float p10, p25;
int error;

sensors_event_t tempSensor;
sensors_event_t humiSensor;

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, D3, D5);
OLEDDisplayUi ui(&display);

SDS011 my_sds;
DHT_Unified dht(DHTPIN, DHT22);

void batteryOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
  float batteryVoltage = battery / 100.0;
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(127, 53, String(batteryVoltage) + "V");
}

void airQualityPM10(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setFont(ArialMT_Plain_10);

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 0 + y, "PM 10");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 15 + y, String(p10));
}

void airQualityPM25(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setFont(ArialMT_Plain_10);

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 0 + y, "PM 2.5");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 15 + y, String(p25));
}

void temp(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 0 + y, "Temperature");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 15 + y, String(tempSensor.temperature));
}

void humi(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 0 + y, "Humidity");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 15 + y, String(humiSensor.relative_humidity));
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {airQualityPM10, airQualityPM25, temp, humi};

// how many frames are there?
int frameCount = 4;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = {batteryOverlay};
int overlaysCount = 1;

void setup()
{
  Serial.begin(115200);
  my_sds.begin(D1, D2); //RX, TX
  dht.begin();
  Serial.println();
  Serial.println();

  pinMode(BUTTON, INPUT_PULLUP);

  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(30);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  ui.setTimePerFrame(5000);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();
}

void loop()
{
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0)
  {
    int rawValue = analogRead(A0);
    battery = map(rawValue, 0, 1023, 0, 439);

    dht.temperature().getEvent(&tempSensor);
    dht.humidity().getEvent(&humiSensor);

    if (!digitalRead(BUTTON))
    {
      ui.nextFrame();
    }

    error = my_sds.read(&p25, &p10);
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(1);
  }
}