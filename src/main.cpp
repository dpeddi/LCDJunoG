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

#define ZOOM_X 2
#define ZOOM_Y 3

#define START_CHANNEL 1
#define NUM_CHANNELS 8192

//volatile uint16_t buffer_cs1[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];
//volatile uint16_t buffer_cs2[DMXINPUT_BUFFER_SIZE(START_CHANNEL, NUM_CHANNELS)];
volatile uint16_t buffer_cs1[3*12*120*10];
volatile uint16_t buffer_cs2[3*12*120*10];

void setup()
{
	  tft.init();
	//tft.setRotation(2);
	tft.setRotation(1);
#ifdef DRAW_SPLASH
	tft.fillScreen(TFT_ORANGE);
	drawBitmapZoom(0, 0,  (const uint8_t *)junog, 240, 90, TFT_BLACK);
  delay(500);
#endif

#define DRAW_INFO
#ifdef DRAW_INFO
	tft.fillScreen(TFT_ORANGE);
	tft.setTextColor(TFT_BLACK);
	tft.setCursor(0, 280, 2);
	tft.println("Roland Juno G Lcd Emulator v0.1");
  tft.print("CPU_FREQ:" + String(rp2040.f_cpu()));
  delay(1000);
#endif

#ifdef DRAW_PINOUT  
	drawPinout(1000);
#endif
	
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0, 2);
	tft.setTextSize(1);
	tft.print("DMX:");
	
	
    // Setup our DMX Input to read on GPIO 0, from channel 1 to 3
    dmxInput[0].begin(2, START_CHANNEL, NUM_CHANNELS, pio0, 1);
    dmxInput[1].begin(2, START_CHANNEL, NUM_CHANNELS, pio1, 2);
    dmxInput[0].read_async(buffer_cs1);
    dmxInput[1].read_async(buffer_cs2);

    // Setup the onboard LED so that we can blink when we receives packets
    pinMode(LED_BUILTIN, OUTPUT);
}


int cnt = 0;

int start_index_cs1 = 0;
int start_index_cs2 = 0;

uint8_t page[2] = {0,0};
uint8_t xx[2] = {0,0};

void draw_juno_g(uint8_t val, uint8_t rs, uint8_t cs) {

    if (rs == 0) {
        if ((val >> 4) == 0xb) {
          page[cs] = val & 0xf;
          xx[cs] = 0;
          cnt = 0;
        } /*else
        if (tft.getCursorY() >320 ) {
            tft.fillScreen(TFT_ORANGE);
            tft.setCursor(0,0,2);
        } */
    } else if (rs == 1 ) {
        if (xx[cs] < 120) {  // avoid overlap the right area
          cnt++;
          for (int i = 0; i < 8; i++ ) {
              if (((val >> i) & 0x1) == 1) {
                tft.drawPixel(120 * cs * ZOOM_X + xx[cs] * ZOOM_X, (page[cs] * 8 + i ) * ZOOM_Y, TFT_BLACK);
              } else {
                tft.drawPixel(120 * cs * ZOOM_X + xx[cs] * ZOOM_X, (page[cs] * 8 + i ) * ZOOM_Y, TFT_ORANGE);
              }
          }
          xx[cs]++;
        } 
    }

}

void loop()
{

    //delay(30);

    if(millis() > 50 + dmxInput[0].latest_packet_timestamp()) {
        Serial.println("no data!");
        //return;
    } else {
      uint cur_index_cs1 = (uint) dmxInput[0].get_capture_index();  
      if (start_index_cs1 > cur_index_cs1)
        start_index_cs1 = 0;


      for (uint i = start_index_cs1; i < cur_index_cs1; i++)  
      {
          char sbuf[50];

          uint8_t val = buffer_cs1[i] & 0xff;
          uint8_t rs = (buffer_cs1[i] >> 9) & 1 ;
  
          uint8_t cs1 = 1;// (buffer_cs1[i] >> 10) & 1 ;
          uint8_t cs2 = 1;// (buffer_cs1[i] >> 11) & 1 ;
          
          draw_juno_g(val, rs, 0);

          //Serial.print(buffer_cs1[i]);
          //Serial.print(", ");
      }
      start_index_cs1 = cur_index_cs1;
    }

    if(millis() > 50 + dmxInput[1].latest_packet_timestamp()) {
        Serial.println("no data!");
        //return;
    } else {
      uint cur_index_cs2 = (uint) dmxInput[1].get_capture_index();  
      if (start_index_cs2 > cur_index_cs2)
        start_index_cs2 = 0;

      for (uint i = start_index_cs2; i < cur_index_cs2; i++)  
      {
          char sbuf[50];

          uint8_t val = buffer_cs2[i] & 0xff;
          uint8_t rs = (buffer_cs2[i] >> 9) & 1 ;
  
          uint8_t cs1 = 1; //(buffer_cs2[i] >> 10) & 1 ;
          uint8_t cs2 = 1; //(buffer_cs2[i] >> 11) & 1 ;
          
          /*if ( ((val >> 4) == 0xb) && rs == 0) {
          sprintf(sbuf,"%08x:%02x:%d:%d ", buffer_cs2[i], val, rs, cs1);
          tft.print(sbuf);
              if (tft.getCursorY() >320 ) {
                  tft.fillScreen(TFT_ORANGE);
                  tft.setCursor(0,0,2);
              }
          } */

          draw_juno_g(val, rs, 1);

          //Serial.print(buffer_cs2[i]);
          //Serial.print(", ");
      }
      start_index_cs2 = cur_index_cs2;
    }

    // Wait for next DMX packet
    //dmxInput[0].read(buffer);

    // Print the DMX channels
    //Serial.print("Received packet: ");
    //for (uint i = 0; i < sizeof(buffer) /2 ; i++)  


    //Serial.println("");


    // Blink the LED to indicate that a packet was received
    digitalWrite(LED_BUILTIN, HIGH);
    delay(10);
    digitalWrite(LED_BUILTIN, LOW);
}


//0xb0*3 - 0xbb*3  *120
//12 *3 *120 = 4320