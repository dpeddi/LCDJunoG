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
#define NUM_CHANNELS 8192

volatile uint16_t buffer[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];

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
    dmxInput.begin(2, START_CHANNEL, NUM_CHANNELS);

    // Setup the onboard LED so that we can blink when we receives packets
    pinMode(LED_BUILTIN, OUTPUT);
}

// Function to reverse bits of num
unsigned int reverseBits(unsigned int num)
{
    unsigned int NO_OF_BITS = sizeof(num) * 8;
    unsigned int reverse_num = 0;
    int i;
    for (i = 0; i < NO_OF_BITS; i++) {
        if ((num & (1 << i)))
            reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
}


int page1 = 0;
int page2 = 0;
int yy1 = 0;
int yy2 = 0;

int cnt = 0;

int start_index = 0;
void loop()
{
    // Wait for next DMX packet
    dmxInput.read(buffer);

    // Print the DMX channels
    //Serial.print("Received packet: ");
    //for (uint i = 0; i < sizeof(buffer) /2 ; i++)  
    uint cur_index = (uint) dmxInput.get_capture_index();  
    if (start_index > cur_index)
      start_index = 0;

    for (uint i = start_index; i < cur_index; i++)  
    {
//5b7 01 0110110111
//5b4 01 0110110100
//5b5 01 0110110101
//7b5 01 1110110101
//7b7 01 1110110111
        char sbuf[50];
        //sprintf(sbuf,"%08x", buffer[i]);
        //Serial.println(sbuf);
        //unsigned int val = reverseBits((buffer[i] >> 2 ) & 0xff ) >> 0x18;
        //uint8_t we = (buffer[i] >> 0) & 1 ;
        //uint8_t rs = (buffer[i] >> 1) & 1 ;
 
        uint8_t val = buffer[i] & 0xff;
        uint8_t rs = (buffer[i] >> 9) & 1 ;
 
        uint8_t cs1 = (buffer[i] >> 10) & 1 ;
        uint8_t cs2 = (buffer[i] >> 11) & 1 ;
        
        if ( ((val >> 4) == 0xb) && rs == 0) {
        //sprintf(sbuf,"%08x:%02x:%d:%d ", buffer[i], val, rs, cs1);
        //tft.print(sbuf);
             if (tft.getCursorY() >320 ) {
                tft.fillScreen(TFT_ORANGE);
                tft.setCursor(0,0,2);
            }
        }

        if (rs == 0) {
            //sprintf(sbuf,"%02x:%d ", val, cs1);
            //tft.print(sbuf);
            if ((cs1 == 1) & ((val >> 4) == 0xb)) {
              page1 = val & 0xf;
              yy1 = 0;
             // sprintf(sbuf,"%02x, ", val);
              //tft.print(sbuf);
                /*sprintf(sbuf,"cnt %d", cnt);
                tft.print(sbuf);*/
                cnt = 0;
            } else
            if ((cs2 == 188) & ((val >> 4) == 0xb)) {
              page2 = val & 0xf;
              yy2 = 0;
              sprintf(sbuf,"val: %02x page:%03d\n", val, page1);
              tft.print(sbuf);
            }
            if (tft.getCursorY() >320 ) {
                tft.fillScreen(TFT_ORANGE);
                tft.setCursor(0,0,2);
            }
        } else if (rs == 1 ) {
            if ((cs1 == 1) && (yy1 < 120)) {  // avoid overlap the right area
              cnt++;
              for (int i = 0; i < 8; i++ ) {
                  if (((val >> i) & 0x1) == 1) {
                    tft.drawPixel(yy1*2, (page1 * 8 + i ) * 3, TFT_BLACK);
                  } else {
                    tft.drawPixel(yy1*2, (page1 * 8 + i ) * 3, TFT_WHITE);
                  }
              }
              yy1++;
            } else
            if (cs2 == 1) {
              for (int i = 0; i < 8; i++ ) {
                  if (((val >> i) & 0x1) == 1) {
                    tft.drawPixel(120 + yy2, page2 * 8 + i, TFT_BLACK);
                  } else {
                    tft.drawPixel(120 + yy2, page2 * 8 + i, TFT_LIGHTGREY);
                  }
              }
              yy2++;
            }
        }


        //Serial.print(buffer[i]);
        //Serial.print(", ");
    }
    //Serial.println("");
    start_index = cur_index;

    // Blink the LED to indicate that a packet was received
    digitalWrite(LED_BUILTIN, HIGH);
    delay(10);
    digitalWrite(LED_BUILTIN, LOW);
}