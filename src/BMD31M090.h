/*****************************************************************
File:             BMD31M090.h
Author:           BESTMODULES
Description:      Define classes and required variables
History:         
V1.0.2   --  2023-12-22
******************************************************************/

#ifndef _BMD31M090_H_
#define _BMD31M090_H_

#include "Arduino.h"
#include "utility/fontTable.h"  
#include <Wire.h>

#define BMD31M090_DEVICEADDR0  0x3C     // See Schematic "I2C Addr SEL" for address setting,
#define BMD31M090_DEVICEADDR1  0x3D     // The I2C address can configure: Addr0:0x3C ; Addr1:0x3D
#define BMD31M090_CLKFREQ      400000UL

#define TRUE                  1
#define FALSE                 0

#define displayROW0          0
#define displayROW1          1
#define displayROW2          2
#define displayROW3          3
#define displayROW4          4
#define displayROW5          5
#define displayROW6          6
#define displayROW7          7

/* Pixel Color                                                */
#define pixelColor_BLACK 0              // Pixel Color is 'OFF'
#define pixelColor_WHITE 1              // Pixel Color is 'ON'
#define pixelColor_INVERSE 2            // Pixel Color is last color flip

/* Datasheet Relation Setup                                   */
#define SET_LOWCOLUMN       0x00
#define SET_HIGHCOLUMN      0x10
#define SET_ADDRESSINGMODE  0x20
#define SET_COLUMNADDR      0x21
#define SET_PAGEADDR        0x22
#define SET_STARTLINE       0x40
#define SET_CONTRAST        0x81
#define SET_CHARGEPUMP      0x8D
#define SEGMENT_REMAP       0xA1
#define DISPLAYALLON_RESUME 0xA4
#define SET_NORMALDISPLAY   0xA6
#define SET_INVERTDISPLAY   0xA7
#define SET_MULTIPLEX       0xA8
#define SET_DISPLAYOFF      0xAE
#define SET_DISPLAYON       0xAF
#define SET_PAGEADDRESS     0xB0
#define COMSCAN_DIRECTION   0xC8
#define NORMAL_CONTRAST     0xCF
#define SET_DISPLAYOFFSET   0xD3
#define SET_DISPLAYCLOCKDIV 0xD5
#define SET_PRECHARGE       0xD9
#define SET_COMPINS         0xDA
#define SET_VCOMDETECT      0xDB

/* Display Scroll Relation Setup                              */
#define RIGHT_SCROLL         0x26
#define LEFT_SCROLL          0x27
#define RIGHTDIAGONAL_SCROLL 0x29
#define LEFTDIAGONAL_SCROLL  0x2A
#define DEACTIVATE_SCROLL    0x2E
#define ACTIVATE_SCROLL      0x2F
#define SET_SCROLLAREA       0xA3

#define SCROLL_2FRAMES       0x07
#define SCROLL_3FRAMES       0x04
#define SCROLL_4FRAMES       0x05
#define SCROLL_5FRAMES       0x00
#define SCROLL_25FRAMES      0x06
#define SCROLL_64FRAMES      0x01
#define SCROLL_128FRAMES     0x02
#define SCROLL_256FRAMES     0x03

#define SCROLLV_NONE         0x00       // Scroll Vertical Direction : N/A
#define SCROLLV_TOP          0x01       // Scroll Vertical Direction : Top
#define SCROLLV_BOTTOM       0x3F       // Scroll Vertical Direction : Bottom

class BMD31M090
{
  public:
    BMD31M090(uint8_t width, uint8_t height, TwoWire *theWire = &Wire);
    void begin(uint8_t  i2c_addr = BMD31M090_DEVICEADDR0, uint32_t clkFrequency = BMD31M090_CLKFREQ);
    void clearDisplay(void);
    void display(void);
    void drawPixel(uint8_t x, uint8_t y, uint8_t pixelColor);
    void drawLine(uint8_t x_Start, uint8_t y_Start, uint8_t x_End, uint8_t y_End, uint8_t pixelColor);
    void drawFastHLine(uint8_t x, uint8_t y, uint8_t width, uint8_t pixelColor);
    void drawFastVLine(uint8_t x, uint8_t y, uint8_t height, uint8_t pixelColor);
    void drawChar(uint8_t x, uint8_t row, uint8_t chr);
    void drawString(uint8_t x, uint8_t row, uint8_t *str);
    void drawNum(uint8_t x, uint8_t row, uint32_t num, uint8_t numLen);
    void drawBitmap(int8_t x, int8_t y, const uint8_t *Bitmap, uint8_t w, uint8_t h, uint8_t pixelColor);
    void startScrollRight(uint8_t startRow, uint8_t endRow, uint8_t scrollSpeed, uint8_t scrollVDirection = SCROLLV_NONE);
    void startScrollLeft(uint8_t startRow, uint8_t endRow, uint8_t scrollSpeed, uint8_t scrollVDirection = SCROLLV_NONE);
    void stopScroll(void);
    void setFont(const unsigned char* font);
    void setPixelRow(uint8_t x, uint8_t row);
    void dim(bool dim);
    void invertDisplay(bool i);
    
  private:
    void setCommand(uint8_t command);
    void setData(uint8_t data);
    uint32_t setPow(uint8_t baseValue, uint8_t indexValue);
    TwoWire *_wire = NULL;
    uint8_t _deviceAddress;             // device address initialized when begin method is called
    uint32_t _clkFrequency;
    uint8_t _displayWidth;
    uint8_t _displayHeight;

    const unsigned char* _font;
    uint8_t _fontOffset = 2;
    uint8_t _fontWidth;
    uint8_t _fontHeight;
    uint8_t buffer[1024];
};

#endif
