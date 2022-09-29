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
DmxInput dmxInput[2];

#define START_CHANNEL 1
#define NUM_CHANNELS 8192

volatile uint16_t buffer_cs1[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];
volatile uint16_t buffer_cs2[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

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
    dmxInput[0].begin(2, START_CHANNEL, NUM_CHANNELS, pio0, 1);
    dmxInput[1].begin(2, START_CHANNEL, NUM_CHANNELS, pio0, 2);
    dmxInput[0].read_async(buffer_cs1);
    dmxInput[1].read_async(buffer_cs2);

    // Setup the onboard LED so that we can blink when we receives packets
    pinMode(LED_BUILTIN, OUTPUT);
}

int page1 = 0;
int page2 = 0;
int xx1 = 0;
int xx2 = 0;

int cnt = 0;

int start_index = 0;
void loop()
{

    //delay(30);

    if(millis() > 100 + dmxInput[0].latest_packet_timestamp()) {
        Serial.println("no data!");
        return;
    }

    if(millis() > 100 + dmxInput[1].latest_packet_timestamp()) {
        Serial.println("no data!");
        return;
    }

    // Wait for next DMX packet
    //dmxInput[0].read(buffer);

    // Print the DMX channels
    //Serial.print("Received packet: ");
    //for (uint i = 0; i < sizeof(buffer) /2 ; i++)  
    uint cur_index = (uint) dmxInput[0].get_capture_index();  
    if (start_index > cur_index)
      start_index = 0;

    for (uint i = start_index; i < cur_index; i++)  
    {
        char sbuf[50];

        uint8_t val = buffer_cs1[i] & 0xff;
        uint8_t rs = (buffer_cs1[i] >> 9) & 1 ;
 
        uint8_t cs1 = (buffer_cs1[i] >> 10) & 1 ;
        uint8_t cs2 = (buffer_cs1[i] >> 11) & 1 ;
        
        /*if ( ((val >> 4) == 0xb) && rs == 0) {
        sprintf(sbuf,"%08x:%02x:%d:%d ", buffer_cs1[i], val, rs, cs1);
        tft.print(sbuf);
             if (tft.getCursorY() >320 ) {
                tft.fillScreen(TFT_ORANGE);
                tft.setCursor(0,0,2);
            }
        } */

        if (rs == 0) {
            //sprintf(sbuf,"%02x:%d ", val, cs1);
            //tft.print(sbuf);
            if ((cs1 == 1) & ((val >> 4) == 0xb)) {
              page1 = val & 0xf;
              xx1 = 0;
             // sprintf(sbuf,"%02x, ", val);
              //tft.print(sbuf);
                /*sprintf(sbuf,"cnt %d", cnt);
                tft.print(sbuf);*/
                cnt = 0;
            } else
            if ((cs2 == 2) & ((val >> 4) == 0xb)) {
              page2 = val & 0xf;
              xx2 = 0;
              /*sprintf(sbuf,"val: %02x page:%03d\n", val, page1);
              tft.print(sbuf);*/
            }
            if (tft.getCursorY() >320 ) {
                tft.fillScreen(TFT_ORANGE);
                tft.setCursor(0,0,2);
            }
        } else if (rs == 1 ) {
            if ((cs1 == 1) && (xx1 < 120)) {  // avoid overlap the right area
              cnt++;
              for (int i = 0; i < 8; i++ ) {
                  if (((val >> i) & 0x1) == 1) {
                    tft.drawPixel(xx1*2, (page1 * 8 + i ) * 3, TFT_BLACK);
                  } else {
                    tft.drawPixel(xx1*2, (page1 * 8 + i ) * 3, TFT_WHITE);
                  }
              }
              xx1++;
            } else
            if (cs2 == 1 && (xx2 < 120)) {  // avoid overlap the right area
              for (int i = 0; i < 8; i++ ) {
                  if (((val >> i) & 0x1) == 1) {
                    tft.drawPixel((120 * 2 + xx2 * 2), (page2 * 8 + i) * 3, TFT_BLACK);
                  } else {
                    tft.drawPixel((120 * 2 + xx2 * 2), (page2 * 8 + i) * 3, TFT_LIGHTGREY);
                  }
              }
              xx2++;
            }
        }


        //Serial.print(buffer_cs1[i]);
        //Serial.print(", ");
    }
    //Serial.println("");
    start_index = cur_index;

    // Blink the LED to indicate that a packet was received
    digitalWrite(LED_BUILTIN, HIGH);
    delay(10);
    digitalWrite(LED_BUILTIN, LOW);
}