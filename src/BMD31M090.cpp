/*****************************************************************
File:        BMD31M090.cpp
Author:      BESTMODULES
Description: BMD31M090.cpp is the library for controlling the BMD31M090 OLED Module
History:
V1.0.2   --  2023-12-22
******************************************************************/
#include "BMD31M090.h"

#ifndef _swapValue_uint8_t
#define _swapValue_uint8_t(a, b)                                               \
  {                                                                            \
    uint8_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

#define ssd1306_swap(a, b)                                                     \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

/**********************************************************
Description: Constructor
Parameters: width: Display width in pixels
            height: Display height in pixels
            theWire: Wire object if your board has more than one Wire interface
Return:
Others:
**********************************************************/
BMD31M090::BMD31M090(uint8_t width, uint8_t height, TwoWire *theWire)
{
  _displayWidth = width;
  _displayHeight = height;
  _wire = theWire;
}

/**********************************************************
Description: Module Initial
Parameters: deviceAddress: Device Address Of Corresponding BMD31M090 Display.
            clkFrequency: Speed (in Hz) for Wire transmissions in BMD31M090 library calls.
                          (Defaults to 400000 Hz (400 KHz))
Return:
Others:
**********************************************************/
void BMD31M090::begin(uint8_t  i2c_addr, uint32_t clkFrequency)
{
  _deviceAddress =  i2c_addr;
  _clkFrequency = clkFrequency;

  _wire->begin();

  _wire->setClock(_clkFrequency);                   // IIC rate (default rate = 40kHz)

  _wire->setWireTimeout(25000, true);

  setCommand(SET_DISPLAYOFF);                       // 0xAE

  setCommand(SET_DISPLAYCLOCKDIV);                  // 0xD5
  setCommand(0x80);                                 // The Suggested Ratio 0x80

  setCommand(SET_MULTIPLEX);                        // 0xA8
  setCommand(_displayHeight - 1);                   // 0x3F - (64 -1)dec -> (0x3F)hex

  setCommand(SET_DISPLAYOFFSET);                    // 0xD3
  setCommand(0x00);                                 // No Offset
  setCommand(SET_STARTLINE | 0x0);                  // 0x40
  setCommand(SET_CHARGEPUMP);                       // 0x8D
  setCommand(0x14);                                 // 0x14, Enable Charge Pump

  setCommand(SET_ADDRESSINGMODE);                   // 0x20
  setCommand(0x00);                                 // Horizontal Address Mode
  setCommand(SEGMENT_REMAP);                        // 0xA1
  setCommand(COMSCAN_DIRECTION);                    // 0xC8

  setCommand(SET_COMPINS);                          // 0xDA
  setCommand(0x12);                                 // COM Pins Hardware Configuration
  setCommand(SET_CONTRAST);                         // 0x81
  setCommand(NORMAL_CONTRAST);                      // 0xCF

  setCommand(SET_VCOMDETECT);                       // 0xDB
  setCommand(0x40);

  setCommand(SET_PRECHARGE);                        // 0xD9
  setCommand(0xF1);
  setCommand(DISPLAYALLON_RESUME);                  // 0xA4
  setCommand(SET_NORMALDISPLAY);                    // 0xA6
  setCommand(DEACTIVATE_SCROLL);                    // 0x2E

  setCommand(SET_DISPLAYON);                        // 0xAF

  clearDisplay();
  display();

  setFont(FontTable_8X16);                          // Default Font Type : 8 * 16
}

/**********************************************************
Description: Clear contents of display buffer
Parameters:
Return:
Others: set all pixels to off.
**********************************************************/
void BMD31M090::clearDisplay(void)
{
  memset(buffer, 0x00, _displayWidth * ((_displayHeight + 7) / 8));
}

/**********************************************************
Description: Push data currently in RAM to Display.
Parameters:
Return:
Others: Drawing operations are not visible until this function is called. 
        Call after each graphics command, or after a whole set of graphics commands, as best needed by one's own application.
**********************************************************/
void BMD31M090::display(void)
{
  uint8_t *ptr = buffer;

  setCommand(SET_PAGEADDRESS);                    // Set Page Address
  setCommand(SET_LOWCOLUMN);                      // Set Column Lower Address
  setCommand(SET_HIGHCOLUMN);                     // Set Column Higher Address

  uint16_t count = _displayWidth * ((_displayHeight + 7) / 8);
  while (count--)
  {
    setData(*ptr++);
  }
}

/**********************************************************
Description: Set/Clear/Invert a single pixel.
Parameters: x: Column of display, 0 at left to (_displayWidth - 1) at right.
            y: Row of display, 0 at left to (_displayHeight - 1) at right.
            pixelColor: pixelColor_BLACK,
                        pixelColor_WHITE,
                        pixelColor_INVERSE,
Return:
Others:
**********************************************************/
void BMD31M090::drawPixel(uint8_t x, uint8_t y, uint8_t pixelColor)
{
  switch (pixelColor)
  {
    case pixelColor_BLACK:
      buffer[x + (y / 8) * _displayWidth] &= ~(1 << (y & 7));
      break;

    case pixelColor_WHITE:
      buffer[x + (y / 8) * _displayWidth] |= (1 << (y & 7));
      break;

    case pixelColor_INVERSE:
      buffer[x + (y / 8) * _displayWidth] ^= (1 << (y & 7));
      break;

    default:
      /* BMD31M090 doesn't support colors other than black and white. */
      break;
  }
}

/**********************************************************
Description: draw Line.
Parameters: x_Start : Start point x coordinate.
            y_Start : Start point y coordinate.
            x_End : End point x coordinate.
            y_End : End point y coordinate.
            pixelColor: pixelColor_BLACK,
                        pixelColor_WHITE,
                        pixelColor_INVERSE,
Return:
Others:
**********************************************************/
void BMD31M090::drawLine(uint8_t x_Start, uint8_t y_Start, uint8_t x_End, uint8_t y_End, uint8_t pixelColor)
{
  uint8_t Steep = abs(y_End - y_Start) > abs(x_End - x_Start);

  if (Steep)
  {
    _swapValue_uint8_t(x_Start, y_Start);
    _swapValue_uint8_t(x_End, y_End);
  }

  if (x_Start > x_End)
  {
    _swapValue_uint8_t(x_Start, x_End);
    _swapValue_uint8_t(y_Start, y_End);
  }

  uint8_t dx, dy;
  dx = abs(x_End - x_Start);
  dy = abs(y_End - y_Start);
  
  int16_t errValue = dx / 2;
  int8_t yStep;

  if (y_Start < y_End)
  {
    yStep = 1;
  }
  else
  {
    yStep = -1;
  }

  for (uint8_t i=x_Start; i <= x_End; i++)
  {
    if (Steep) 
    {
      drawPixel(y_Start, i, pixelColor);
    }
    else
    {
      drawPixel(i, y_Start, pixelColor);
    }
    errValue -= dy;
    if (errValue < 0)
    {
      y_Start += yStep;
      errValue += dx;
    }
  }
}

/**********************************************************
Description: draw a Horizontal Line
Parameters: x: Column of display, 0 at left to (_display_width - 1) at right.
            y: Row of display, 0 at left to (_displayHeight - 1) at right.
            width : Width of Line, in pixels.
            pixelColor: pixelColor_BLACK,
                        pixelColor_WHITE,
                        pixelColor_INVERSE,
Return:
Others:
**********************************************************/
void BMD31M090::drawFastHLine(uint8_t x, uint8_t y, uint8_t width, uint8_t pixelColor)
{
  drawLine(x, y, (x + width - 1), y, pixelColor);
}

/**********************************************************
Description: draw a Vertical Line
Parameters: x: Column of display, 0 at left to (_display_width - 1) at right.
            y: Row of display, 0 at left to (_displayHeight - 1) at right.
            height : Height of Line, in pixels.
            pixelColor: pixelColor_BLACK,
                        pixelColor_WHITE,
                        pixelColor_INVERSE,
Return:
Others:
**********************************************************/
void BMD31M090::drawFastVLine(uint8_t x, uint8_t y, uint8_t height, uint8_t pixelColor)
{
  drawLine(x, y, x, (y + height - 1), pixelColor);
}

/**********************************************************
Description: draw Char
Parameters: x: Column of display, 0 at left to (_display_width - 1) at right.
            page: Row page of display, 0 at top page to 7 at bottom page.
            chr : The char on FontTable.
Return:
Others:
**********************************************************/
void BMD31M090::drawChar(uint8_t x,uint8_t row,uint8_t chr)
{
  uint8_t  i, charPageNum;
  uint8_t pageAddr = 0;

  if(x > (_displayWidth-1))
  {
    x = 0;
    row = row + (_fontHeight/8);
  }

  if(chr < 32 || chr > 127)
  {
    chr = ' ';
  }

  /* Calculate how many pages a char need. The height is not a multiple of 8 (1 page), set page + 1. */
  charPageNum = (_fontHeight % 8) ? ((_fontHeight / 8) + 1) : (_fontHeight / 8);

  for (uint8_t j = charPageNum; j>0; j--)
  {
    setPixelRow(x, row + pageAddr);
    for(i=0; i<_fontWidth; i++)
    {
      setData(pgm_read_byte(&_font[(chr - 32) * (_fontWidth * ((_fontHeight % 8) ? ((_fontHeight / 8) + 1) : (_fontHeight / 8))) +
                                    _fontOffset + i + (_fontWidth * pageAddr)]));
    }
    pageAddr++;
  }
}

/**********************************************************
Description: draw String
Parameters: x: Column of display, 0 at left to (_display_width - 1) at right.
            page: Row page of display, 0 at top page to 7 at bottom page.
            str : The string on FontTable.
Return:
Others:
**********************************************************/
void BMD31M090::drawString(uint8_t x,uint8_t row,uint8_t *str)
{
  unsigned char j=0;
  while (str[j] != '\0')
  {
    drawChar(x, row, str[j]);

    /* If it is judged that it exceeds the width of the display, set x = 0 and skip to the next display page */
    x += _fontWidth;
    if(x > (128 - _fontWidth))
    {
      x = 0;
      row += (_fontHeight % 8) ? ((_fontHeight / 8) + 1) : (_fontHeight / 8);
    }
    j++;
  }
}

/**********************************************************
Description: draw Num
Parameters: x: Column of display, 0 at left to (_display_width - 1) at right.
            page: Row page of display, 0 at top page to 7 at bottom page.
            num : The num on FontTable.
            numLen : The num Length.
Return:
Others:
**********************************************************/
void BMD31M090::drawNum(uint8_t x, uint8_t row,uint32_t num, uint8_t numLen)
{
  uint8_t t,temp;
  uint8_t enShow=0;
  for(t=0; t<numLen; t++)
  {
    /* One-digit, one-digit display                                                            */
    temp=(num / setPow(10, (numLen - t - 1))) % 10;

    if(enShow == 0 && t < (numLen - 1))
    {
      /* According the value of 'numLen',
         If the Highest digit number is 0 (or Highest and the second of topest, etc.), then show "space". */
      if(temp == 0)
      {
        drawChar(x + (_fontWidth * t), row, ' ');
        continue;
      }
      else
      {
        enShow = 1; 
      }
    }
    /* Show the num                                                                            */
    drawChar(x + (_fontWidth * t), row, (temp + '0'));
  }
}

/**********************************************************
Description: draw BMP
Parameters: x: Column of display, 0 at left to (_display_width - 1) at right.
            y: Row of display, 0 at left to (_displayHeight - 1) at right.
            Bitmap[]: Bitmap Name.
            w: Width of Bitmap in pixels
            h: Height of Bitmap in pixels
            pixelColor: pixelColor_BLACK,
                        pixelColor_WHITE,
                        pixelColor_INVERSE,
Return:
Others:
**********************************************************/
void BMD31M090::drawBitmap(int8_t x, int8_t y, const uint8_t *Bitmap, uint8_t w, uint8_t h, uint8_t pixelColor)
{
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t b = 0;

  for (int16_t j = 0; j < h; j++, y++)
  {
    for (int16_t i = 0; i < w; i++)
    {
      if (i & 7)
      {
        b <<= 1;
      }
      else
      {
        #if 1
        /* RAM-resident 1-bit image at the specified */
        b = Bitmap[(j * byteWidth) + (i / 8)];
        #else
        /* PROGMEM-resident 1-bit image at the specified */
        b = pgm_read_byte(&Bitmap[(j * byteWidth) + (i / 8)]);
        #endif
      }
      if (b & 0x80)
      {
        if (((x + i) >= 0) && ((x + i) < _displayWidth) && (y >= 0) && (y < _displayHeight))
        {
          drawPixel(x + i, y, (pixelColor == pixelColor_WHITE) ? pixelColor_WHITE : 
                                                                (pixelColor == pixelColor_BLACK) ? pixelColor_BLACK : pixelColor_INVERSE);
        }
      }
      else
      {
        if (((x + i) >= 0) && ((x + i) < _displayWidth) && (y >= 0) && (y < _displayHeight))
        {
          drawPixel(x + i, y, (pixelColor == pixelColor_WHITE) ? pixelColor_BLACK : 
                                                                (pixelColor == pixelColor_BLACK) ? pixelColor_WHITE : pixelColor_INVERSE);
        }
      }
    }
  }
}

/**********************************************************
Description: start Right-handed Scroll
Parameters: startPage: start scroll Page, 0 at top page to 7 at bottom page.
            endPage: end scroll Page, 0 at top page to 7 at bottom page.
            scrollSpeed: SCROLL_2FRAMES/SCROLL_3FRAMES/SCROLL_4FRAMES/SCROLL_5FRAMES
                         SCROLL_25FRAMES/SCROLL_64FRAMES/SCROLL_128FRAMES/SCROLL_256FRAMES
            scrollVDirection: SCROLLV_NONE,
                              SCROLLV_TOP,
                              SCROLLV_BOTTOM
Return:
Others:
**********************************************************/
void BMD31M090::startScrollRight(uint8_t startRow, uint8_t endRow, uint8_t scrollSpeed, uint8_t scrollVDirection)
{
  setCommand(DEACTIVATE_SCROLL);                        // Must be deactivated before issuing this command (0x2E).

  switch (scrollVDirection)
  {
    case SCROLLV_NONE:
      setCommand(RIGHT_SCROLL);                       // 0x26, Right Horizontal Scroll
      setCommand(0x00);                               // A[7:0] Dummy Byte (Set as 00h)

      setCommand(startRow);                          // B[2:0] Define start page address
      setCommand(scrollSpeed);                        // C[2:0] Set time interval between each scroll step in terms of the frame frequency
      setCommand(endRow);                            // D[2:0] Define end page address

      setCommand(0x00);                               // E[7:0] Dummy Byte (Set as 00h)
      setCommand(0xFF);                               // F[7:0] Dummy Byte (Set as FFh)
      setCommand(ACTIVATE_SCROLL);                    // 0x2F
      break;

    case SCROLLV_TOP:
    case SCROLLV_BOTTOM:
      setCommand(SET_SCROLLAREA);                     // 0xA3, Set Vertical Scroll Area
      setCommand(0x00);                               // Set No. of rows in top fixed area
      setCommand(_displayHeight);                     // Set No. of rows in scroll area (0-63)

      setCommand(RIGHTDIAGONAL_SCROLL);               // 0x29, Vertical and Right Horizontal Scroll
      setCommand(0x00);                               // A[7:0] Dummy Byte (Set as 00h)

      setCommand(startRow);                          // B[2:0] Define start page address
      setCommand(scrollSpeed);                        // C[2:0] Set time interval between each scroll step in terms of the frame frequency
      setCommand(endRow);                            // D[2:0] Define end page address

      setCommand(scrollVDirection);                   // Vertical Scrolling offset
      setCommand(ACTIVATE_SCROLL);                    // 0x2F
      break;

    default:
      break;
  }
}

/**********************************************************
Description: start Left-handed Scroll
Parameters: startPage: start scroll Page, 0 at top page to 7 at bottom page.
            endPage: end scroll Page, 0 at top page to 7 at bottom page.
            scrollSpeed: SCROLL_2FRAMES/SCROLL_3FRAMES/SCROLL_4FRAMES/SCROLL_5FRAMES
                         SCROLL_25FRAMES/SCROLL_64FRAMES/SCROLL_128FRAMES/SCROLL_256FRAMES
            scrollVDirection: SCROLLV_NONE,
                              SCROLLV_TOP,
                              SCROLLV_BOTTOMs
Return:
Others:
**********************************************************/
void BMD31M090::startScrollLeft(uint8_t startRow, uint8_t endRow, uint8_t scrollSpeed, uint8_t scrollVDirection)
{
  setCommand(DEACTIVATE_SCROLL);                        // Must be deactivated before issuing this command (0x2E).

  switch (scrollVDirection)
  {
    case SCROLLV_NONE:
      setCommand(LEFT_SCROLL);                        // 0x27, Left Horizontal Scroll
      setCommand(0x00);                               // A[7:0] Dummy Byte (Set as 00h)

      setCommand(startRow);                          // B[2:0] Define start page address
      setCommand(scrollSpeed);                        // C[2:0] Set time interval between each scroll step in terms of the frame frequency
      setCommand(endRow);                            // D[2:0] Define end page address

      setCommand(0x00);                               // E[7:0] Dummy Byte (Set as 00h)
      setCommand(0xFF);                               // F[7:0] Dummy Byte (Set as FFh)
      setCommand(ACTIVATE_SCROLL);                    // 0x2F
      break;

    case SCROLLV_TOP:
    case SCROLLV_BOTTOM:
      setCommand(SET_SCROLLAREA);                     // 0xA3, Set Vertical Scroll Area
      setCommand(0x00);                               // Set No. of rows in top fixed area
      setCommand(_displayHeight);                     // Set No. of rows in scroll area (0-63)

      setCommand(LEFTDIAGONAL_SCROLL);                // 0x2A, Vertical and Left Horizontal Scroll
      setCommand(0x00);                               // A[7:0] Dummy Byte (Set as 00h)

      setCommand(startRow);                          // B[2:0] Define start page address
      setCommand(scrollSpeed);                        // C[2:0] Set time interval between each scroll step in terms of the frame frequency
      setCommand(endRow);                            // D[2:0] Define end page address

      setCommand(scrollVDirection);                   // Vertical Scrolling offset
      setCommand(ACTIVATE_SCROLL);                    // 0x2F
      break;

    default:
      break;
  }
}

/**********************************************************
Description: stop a previously-begun scrolling action.
Parameters:
Return:
Others:
**********************************************************/
void BMD31M090::stopScroll(void)
{
  setCommand(DEACTIVATE_SCROLL);                    // 0x2E
}

/**********************************************************
Description: set Font
Parameters: font: fontTable's font name.
Return:
Others: Currently support FontTable_8X16 and FontTable_6X8.
        After begin() is executed, FontTable_8X16 is used by default.
        (set __fontWidth and _fontHeight by _font[0]&[1])
**********************************************************/
void BMD31M090::setFont(const unsigned char* font)
{
  _font = font;
  _fontWidth = pgm_read_byte(&_font[0]);
  _fontHeight = pgm_read_byte(&_font[1]);
}

/**********************************************************
Description: set Pixel Page
Parameters: x: Column of display, 0 at left to (_display_width - 1) at right.
            page: Row page of display, 0 at top page to 7 at bottom page.
Return:
Others:
**********************************************************/
void BMD31M090::setPixelRow(uint8_t x, uint8_t row)
{
  setCommand(SET_PAGEADDRESS + row);               // Set Page Address
  setCommand(((x & 0xF0) >> 4) | SET_HIGHCOLUMN);   // Set Column Higher Address
  setCommand((x & 0x0F));                           // Set Column Lower Address
}

/**********************************************************
Description: set Dim to display
Parameters: dim : True for enable dim mode (contrast value : 0x00),
                  False for Normal brightness mode (contrast value : 0xCF).
Return:
Others: Contrast ranges from 00h to FFh, the range is too small to be really useful.
        This function is only set for the lowest contrast and normal contrast.
**********************************************************/
void BMD31M090::dim(bool dim)
{
  setCommand(SET_CONTRAST);
  setCommand(dim ? 0 : NORMAL_CONTRAST);            // if True - set contrast 0 , False - set normal contrast 0xCF
}

/**********************************************************
Description: invert Display mode
Parameters: i: True to set invert Display mode (black-on-white).
               False to set normal Display mode (white-on-black).
Return:
Others:
**********************************************************/
void BMD31M090::invertDisplay(bool i)
{
  setCommand(i ? SET_INVERTDISPLAY : SET_NORMALDISPLAY);
}

/**********************************************************
Description: set Command to display
Parameters: Commmand: The Command character to send to the display.
Return:
Others:
**********************************************************/
void BMD31M090::setCommand(uint8_t command)
{
  uint8_t ret;
  do
  {
    _wire->beginTransmission(_deviceAddress);
    _wire->write(0x00);
    _wire->write(command);
    ret = _wire->endTransmission();
  }while(ret!=0);
}

/**********************************************************
Description: set Data to display
Parameters: Data: The Data character to send to the display.
Return:
Others:
**********************************************************/
void BMD31M090::setData(uint8_t data)
{
  uint8_t ret;
  do
  {
    _wire->beginTransmission(_deviceAddress);
    _wire->write(0x40);
    _wire->write(data);
    ret = _wire->endTransmission();
  }while(ret!=0);
}

/**********************************************************
Description: set Pow
Parameters: baseValue : Base Value.
            indexValue : Index Value.
Return: The value of baseValue^indexValue.
Others:
**********************************************************/
uint32_t BMD31M090::setPow(uint8_t baseValue, uint8_t indexValue)
{
  uint32_t result = 1;
  while(indexValue--)
  {
    result *= baseValue;                            // result = baseValue^indexValue
  }

  return result;
}
