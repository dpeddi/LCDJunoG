/*
 * Copyright (c) 2021 Jostein Løwer 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Description: 
 * Starts a DMX Input on GPIO pin 0 and read channel 1-3 repeatedly
 */

#include <Arduino.h>

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <junog.h>
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#include "DmxInput.h"
DmxInput dmxInput;

#define START_CHANNEL 1
#define NUM_CHANNELS 3

volatile uint8_t buffer[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

void drawBitmapZoom(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  //inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h*3; j++) {
    for (i = 0; i < w*2; i++ ) {
      if (pgm_read_byte(bitmap + j / 3 * byteWidth + i / 2 / 8) & (128 >> ((i/2) & 7))) {
        tft.drawPixel(x + i, y + j, color);
      }
    }
  }

  //inTransaction = lockTransaction;
  //end_tft_write();              // Does nothing if Sprite class uses this function
}

void drawPinout(uint16_t delayTime) {
    //tft.fillScreen(TFT_ORANGE);
    tft.fillRect(0, 0, 480, 270, TFT_ORANGE);
    tft.setCursor(200, 0, 2); 
    tft.setTextColor(TFT_BLACK); 
    tft.setTextSize(2);
    tft.println("Pinout:");
    
    tft.setCursor(0, tft.getCursorY(), 2); 

    tft.print("D0 " + String(JUNO_D0));
    tft.setCursor(240, tft.getCursorY(), 2); 
    tft.println("CS1 " + String(JUNO_CS1));

    tft.print("D1 " + String(JUNO_D1));
    tft.setCursor(240, tft.getCursorY(), 2); 
    tft.println("CS2 " + String(JUNO_CS2));

    tft.print("D2 " + String(JUNO_D2));
    tft.setCursor(240, tft.getCursorY(), 2); 
    tft.println("WE  " + String(JUNO_WE));

    tft.print("D3 " + String(JUNO_D3));
    tft.setCursor(240, tft.getCursorY(), 2); 
    tft.println("RS  " + String(JUNO_RS));

    tft.println("D5 " + String(JUNO_D5));
    tft.println("D4 " + String(JUNO_D4));
    tft.println("D6 " + String(JUNO_D6));
    tft.println("D7 " + String(JUNO_D7));

    delay(delayTime);
}

void setup()
{
	  tft.init();
	//tft.setRotation(2);
	tft.setRotation(1);
	tft.fillScreen(TFT_ORANGE);
	drawBitmapZoom(0, 0,  (const uint8_t *)junog, 240, 90, TFT_BLACK);
	tft.setTextColor(TFT_BLACK);
	tft.setCursor(0, 280, 2);
	tft.println("Roland Juno G Lcd Emulator v0.1");
	
	drawPinout(500);
	
	tft.fillScreen(TFT_ORANGE);
	tft.setCursor(0, 0, 2);
	tft.setTextSize(1);
	tft.print("DMX:");
	
	
    // Setup our DMX Input to read on GPIO 0, from channel 1 to 3
    dmxInput.begin(0, START_CHANNEL, NUM_CHANNELS);

    // Setup the onboard LED so that we can blink when we receives packets
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    // Wait for next DMX packet
    dmxInput.read(buffer);

    // Print the DMX channels
    //Serial.print("Received packet: ");
    for (uint i = 0; i < sizeof(buffer); i++)
    {
        tft.print(buffer[i]);
        tft.print(",");
        //Serial.print(buffer[i]);
        //Serial.print(", ");
    }
    //Serial.println("");

    // Blink the LED to indicate that a packet was received
    digitalWrite(LED_BUILTIN, HIGH);
    delay(10);
    digitalWrite(LED_BUILTIN, LOW);
}