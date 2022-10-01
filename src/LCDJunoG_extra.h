/*
 * Copyright (c) 2022 Eddi De Pieri
 * Most code borrowed by Pico-DMX by Jostein Løwer 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 * Description: 
 * Roland Juno G LCD Emulator
 */

#include <TFT_eSPI.h> // Hardware-specific library
extern TFT_eSPI tft;

void showcmd(uint8_t cs, uint8_t val) {
    char sbuf[40];
	if ((val & 0xfe ) == 0xae) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (on/off)", cs, val);
        Serial.println(sbuf);
    } else if ((val & 0xfe ) == 0xa0) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (ADC select)", cs, val);
        Serial.println(sbuf);
	} else if ((val & 0xf0 ) == 0x40) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (Initial display line)", cs, val);
        Serial.println(sbuf);
	} else if ((val & 0xf0 ) == 0x20) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (Regulator resistor select)", cs, val);
        Serial.println(sbuf);
    } else if ((val & 0xf0 ) == 0xc0) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (SHL select)", cs, val);
        Serial.println(sbuf);
	} else if (val == 0xe2) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (reset)", cs, val);
        Serial.println(sbuf);
	} else if ((val & 0xf0 ) == 0x10 || (val & 0xf0 ) == 0x00) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (set column address)", cs, val);
//        Serial.println(sbuf);
    } else if ((val & 0xf0 ) == 0xb0) {
		sprintf(sbuf, "cs: %d, cmd 0x%02x (Set page address)", cs, val);
        Serial.println(sbuf);
    } else {
		sprintf(sbuf, "cs: %d, cmd 0x%02x", cs, val);
        Serial.println(sbuf);
    }
}

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
