/*
 * Copyright (c) 2022 Eddi De Pieri
 * Most code borrowed by Pico-DMX by Jostein Løwer 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Description: 
 * Roland Juno G LCD Emulator
 */

#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <LCDJunoG_splash.h>
#include <LCDJunoG_extra.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#include "LCDJunoG.h"
LCDJunoG lcdJunoG[2];

#define ZOOM_X 2
#define ZOOM_Y 3

uint tft_xoffset = 0;
uint tft_yoffset = 0;

volatile uint16_t buffer_cs1[2][12*123*10];

uint32_t tft_bgcolor = TFT_BLACK;
uint32_t tft_bgcolor_prev = TFT_BLACK;

void intelaced_FillScreen(uint32_t bgcolor) {
  tft.fillScreen(TFT_BLACK);
  for (uint y = 0; y < tft.height(); y++) {
    for (uint x = 0; x < tft.width(); x++) {
      if (x % ZOOM_X  == 0 && y % ZOOM_Y == 0)
        tft.drawPixel(x, y, bgcolor);
    }
  }
}

bool force_refresh = false;

void tft_change_bgcolor() {
  uint32_t analog_read = analogRead(JUNO_BRGT);
  //Serial.println("ANALOG"+ String(analog_read));
  if ( analog_read <= 2850) {
    tft_bgcolor = TFT_ORANGE;
  } else if ( analog_read > 2650 && analog_read <= 2850) {
    tft_bgcolor = TFT_SKYBLUE;
  } else if ( analog_read > 2850 && analog_read <= 3000) {
    tft_bgcolor = TFT_GREENYELLOW;
  } else if ( analog_read > 3000 && analog_read <= 3150) {
    tft_bgcolor = TFT_YELLOW;
  } else if ( analog_read > 3150 && analog_read <= 3300) {
    tft_bgcolor = TFT_WHITE;
  } else if ( analog_read > 3300) {
    tft_bgcolor = TFT_CYAN;
  } 

  if (tft_bgcolor != tft_bgcolor_prev) {
    intelaced_FillScreen(tft_bgcolor);
    tft_bgcolor_prev = tft_bgcolor;
    force_refresh = true;
  }
}

void setup()
{
  analogReadResolution(12);
  uint32_t analog_read = analogRead(JUNO_BRGT);
  //Serial.begin(115200);
  // delay(2000);
  //Serial.setTimeout(50);
  Serial.println("juno g lcd emulator");
  tft.init();
  //tft.setRotation(2);
  tft.setRotation(1);
  tft_xoffset = (tft.width() - 240 * ZOOM_X) / 2 - ((tft.width() - 240 * ZOOM_X) / 2 % ZOOM_X);
  tft_yoffset = (tft.height() - 96 * ZOOM_Y) / 2 - ((tft.height() - 96 * ZOOM_Y) / 2 % ZOOM_Y);

#ifdef DRAW_SPLASH
  tft_change_bgcolor();
  drawBitmapZoom(0, 0, (const uint8_t *)junog, 240, 90, TFT_BLACK);
  delay(500);
#endif



#define DRAW_INFO
#ifdef DRAW_INFO
  tft_change_bgcolor();
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(0, tft.height()/2 -10  , 2);
  tft.setTextSize(2);
  
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Roland Juno G Lcd Emulator v0.1", tft.width() /2, tft.height() / 2 - 20 );
  tft.drawString("CPU_FREQ:" + String(rp2040.f_cpu()), tft.width() /2, tft.height() / 2 + 10 );
  delay(500);
#endif

#ifdef DRAW_PINOUT  
  drawPinout(1000);
#endif
   
  tft.setCursor(0, 0, 2);
  tft.setTextSize(1);
  
  // Setup our DMX Input to read on GPIO 0, from channel 1 to 3
  lcdJunoG[0].begin(2, pio0, 1);
  lcdJunoG[1].begin(2, pio1, 2);
  lcdJunoG[0].read_async(buffer_cs1[0]);
  lcdJunoG[1].read_async(buffer_cs1[1]);

  // Setup the onboard LED so that we can blink when we receives packets
  pinMode(LED_BUILTIN, OUTPUT);
}

volatile int start_index[2] = {0,0};
volatile uint8_t page[2] = {0,0};
volatile uint8_t xx[2] = {0,0};

void draw_juno_g(uint8_t val, uint8_t rs, uint8_t cs) {
  if (rs == 0) {
#ifdef SHOWCMD
    showcmd( cs, val );
#endif
    if ((val >> 4) == 0xb) {
      page[cs] = val & 0xf;
      xx[cs] = 0;
    } 
  } else if (rs == 1 ) {
    if (xx[cs] < 120) {  // avoid overlap the right area
      for (int i = 0; i < 8; i++ ) {
        if (((val >> i) & 0x1) == 1) {
          tft.drawPixel(tft_xoffset + 120 * cs * ZOOM_X + xx[cs] * ZOOM_X, tft_yoffset + (page[cs] * 8 + i ) * ZOOM_Y, TFT_BLACK);
        } else {
          tft.drawPixel(tft_xoffset + 120 * cs * ZOOM_X + xx[cs] * ZOOM_X, tft_yoffset + (page[cs] * 8 + i ) * ZOOM_Y, tft_bgcolor);
        }
      }
      xx[cs]++;
    }
  }
}

int period = 500;
unsigned long time_now = 0;
unsigned long my_millis = 0;



void loop()
{  
  my_millis = millis();

  if(my_millis >= time_now + period){
        time_now += period;
        tft_change_bgcolor();
  }

  if(! force_refresh  && my_millis > 50 + lcdJunoG[0].latest_packet_timestamp() && my_millis > 50 + lcdJunoG[1].latest_packet_timestamp()) {
    //Serial.println("no data!");
    return;
  } 
  if (force_refresh) {
    force_refresh = false;
    //Serial.println("Refresh forced!!");
  }
  for (uint i = 0; i < 123*12; i++)  
  {
    for (uint8_t j = 0; j <2; j++) {
      uint8_t val = buffer_cs1[j][i] & 0xff;
      uint8_t rs = (buffer_cs1[j][i] >> 9) & 1;
      draw_juno_g(val, rs, j);
    }
  }

  // Blink the LED to indicate that a packet was received
  digitalWrite(LED_BUILTIN, HIGH);
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
}
