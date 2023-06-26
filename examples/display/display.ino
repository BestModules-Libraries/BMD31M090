/*****************************************************************
File:         display.ino
Description:  1.Wire interface (Clock Frequency: 400K) is used to communicate with BMD31M090.
              2.Hardware Serial (BAUDRATE 115200) is used to communicate with Serial Port Monitor.
connection method: sclPin:SCL(D19) sdaPin:SDA(D18)
******************************************************************/
#include "BMD31M090.h"
#include "Bitmap.h"

#define BMD31M090_WIDTH   128        // BMD31M090 Module display width, in pixels
#define BMD31M090_HEIGHT  64         // BMD31M090 Module display height, in pixels
#define BMD31M090_ADDRESS 0x3C       // See Schematic "I2C Addr SEL" for address setting,
                                     // The I2C address can configure: Addr0:0x3C ; Addr1:0x3D

uint8_t t = ' ';

BMD31M090     BMD31(BMD31M090_WIDTH, BMD31M090_HEIGHT, &Wire); //Please uncomment out this line of code if you use HW Wire on BMduino
//BMD31M090     BMD31(BMD31M090_WIDTH, BMD31M090_HEIGHT, &Wire1); //Please uncomment out this line of code if you use HW Wire1 on BMduino
//BMD31M090     BMD31(BMD31M090_WIDTH, BMD31M090_HEIGHT, &Wire2); //Please uncomment out this line of code if you use HW Wire1 on BMduino

void setup()
{
  Serial.begin(115200);
  Serial.println("BMD31M090 0.96\" OLED Module Sketch");

  BMD31.begin(BMD31M090_ADDRESS);
  delay(100); // Recommended initial setting delay value.

  /* Test Function: drawString to draw font(6x8) Hello World! */
  test_drawString_6x8();

  /* Test Function: drawString to draw font(8x16) Hello World! */
  test_drawString_8x16();

  /* Test Function: drawString, drawChar and drawNum */
  test_drawString_drawChar_drawNum();

  /* Test Function: drawPixel */
  test_drawPixel();

  /* Test Function: drawFastHLine and drawFastVLine */
  test_drawFastHLine_drawFastVLine();

  /* Test Function: drawBitMap to show BestModule_LOGO and NameBestModule_LOGO */
  test_drawBitmap();

  /* Test Function: Scroll function to scroll various directions */
  test_variousScroll();

  /* Test Function: invertDisplay to invert and restore display */
  test_invertDisplay();

  /* Test Function: dim to display */
  test_dim();
}

void loop()
{
}

void test_drawString_6x8(void)
{
  BMD31.clearDisplay();
  BMD31.display();

  uint8_t col, row;

  BMD31.setFont(FontTable_6X8); // Set Font Type - 6X8 (Default Font Type : 8X16)

  col = (128 - (6 * sizeof("Hello World!"))) / 2;
  for (row=0; row<8; row++)
  {
    BMD31.drawString(col, row, (u8*)"Hello World!");
  }
  delay(500);
}

void test_drawString_8x16(void)
{
  BMD31.clearDisplay();
  BMD31.display();

  uint8_t col, row;

  BMD31.setFont(FontTable_8X16); // Set Font Type - 8X16

  col = (128 - (8 * sizeof("Hello World!"))) / 2;
  for (row=0; row<8; row+=2)
  {
    BMD31.drawString(col, row, (u8*)"Hello World!");
  }
  delay(500);
}

void test_drawString_drawChar_drawNum(void)
{
  BMD31.clearDisplay();
  BMD31.display();

  BMD31.drawString(16, displayROW0, (u8*)"Best Modules");
  BMD31.drawString(8, displayROW2, (u8*)"BMD31M090 TEST");
  /* Sequentially display ASCII ' ' (0x20/32) to ' ~' (0x7E/126) values for Font 6x8 & 8x16 */
  do
  {
    BMD31.setFont(FontTable_6X8);
    BMD31.drawString(0, displayROW5, (u8*)"ASCII:");
    BMD31.drawString(63, displayROW5, (u8*)"CODE:");
    BMD31.drawChar(48, displayROW5, t);
    BMD31.drawNum(103, displayROW5, t, 3);

    BMD31.setFont(FontTable_8X16);
    BMD31.drawString(0, displayROW6, (u8*)"ASCII:");
    BMD31.drawString(63, displayROW6, (u8*)"CODE:");
    BMD31.drawChar(48, displayROW6, t);
    BMD31.drawNum(103, displayROW6, t, 3);
    delay(10);
  } while(++t < '~');
  t=' ';
}

void test_drawPixel(void)
{
  BMD31.clearDisplay();
  for (uint8_t col=0; col<128; col++)
  {
    for(uint8_t row=0; row<64; row++)
    {
      BMD31.drawPixel(col, row, pixelColor_WHITE);
    }
  }
  BMD31.display();
  delay(500);

  for (uint8_t col=0; col<128; col++)
  {
    for(uint8_t row=0; row<32; row++)
    {
      BMD31.drawPixel(col, row, pixelColor_BLACK);
    }
  }
  BMD31.display();
  delay(500);

  for (uint8_t col=0; col<128; col++)
  {
    for(uint8_t row=0; row<64; row++)
    {
      BMD31.drawPixel(col, row, pixelColor_INVERSE);
    }
  }
  BMD31.display();
  delay(500);
}

void test_drawFastHLine_drawFastVLine(void)
{
  int8_t col, row;

  BMD31.clearDisplay();
  col = 112;
  for (row=0; row<64; row+=8)
  {
    BMD31.drawFastHLine(0, row, col, pixelColor_WHITE);
    BMD31.drawFastVLine(col, row, 64 - row, pixelColor_WHITE);
    col -= 14;
  }
  BMD31.display();
  delay(500);

  BMD31.clearDisplay();
  col = 112;
  for (row=56; row>=0; row-=8)
  {
    BMD31.drawFastHLine(col, row, 128 - col, pixelColor_WHITE);
    BMD31.drawFastVLine(col, row, 64 - row, pixelColor_WHITE);
    col -= 14;
  }
  BMD31.display();
  delay(500);
}

void test_drawBitmap(void)
{
  /*==========================================================
   |Using Image2LCD Software conversion, setting as bellow    |
   |1. Output file type: C array (*.c)                        |
   |2. Scan mode: Horizon Scan                                |
   |3. Bits Pixel: monochrome                                 |
   |4. Max Width and Height: 128 (Width) / 64 (Height)        |
   |5. Select "MSB First"                                     |
    ----------------------------------------------------------
   |Using LCD Assistant Software conversion, setting as bellow|
   |1. Byte orientation: Horizontal                           |
   |2. Size: According your image size                        |
   |3. Size endianness: Little                                |
   |4. Pixels/byte: 8                                         |
    ==========================================================*/

  BMD31.clearDisplay();
  BMD31.drawBitmap(0, 0, BestModule_LOGO, 128, 64, pixelColor_WHITE);
  BMD31.display();
  delay(300);
  BMD31.drawBitmap(0, 0, BestModule_LOGO, 128, 64, pixelColor_BLACK);
  BMD31.display();
  delay(300);
  BMD31.drawBitmap(0, 0, BestModule_LOGO, 128, 64, pixelColor_INVERSE);
  BMD31.display();
  delay(300);

  BMD31.clearDisplay();
  BMD31.drawBitmap(0, 0, BestModule_LOGOandName, 128, 64, pixelColor_WHITE);
  BMD31.display();
  delay(300);
  BMD31.drawBitmap(0, 0, BestModule_LOGOandName, 128, 64, pixelColor_BLACK);
  BMD31.display();
  delay(300);
  BMD31.drawBitmap(0, 0, BestModule_LOGOandName, 128, 64, pixelColor_INVERSE);
  BMD31.display();
  delay(300);
}


void test_variousScroll(void)
{
  /* Scroll Function Test */
  uint8_t startrow = displayROW0;
  uint8_t endrow = displayROW7;

  BMD31.startScrollRight(startrow, endrow, SCROLL_2FRAMES);
  delay(500);
  BMD31.startScrollRight(startrow, endrow, SCROLL_2FRAMES, SCROLLV_TOP);
  delay(500);
  BMD31.startScrollRight(startrow, endrow, SCROLL_2FRAMES, SCROLLV_BOTTOM);
  delay(500);

  BMD31.startScrollLeft(startrow, endrow, SCROLL_2FRAMES);
  delay(500);
  BMD31.startScrollLeft(startrow, endrow, SCROLL_2FRAMES, SCROLLV_TOP);
  delay(500);
  BMD31.startScrollLeft(startrow, endrow, SCROLL_2FRAMES, SCROLLV_BOTTOM);
  delay(500);

  BMD31.stopScroll();
}

void test_invertDisplay(void)
{
  BMD31.invertDisplay(TRUE); // invert Display Mode:black-on-white
  delay(500);
  BMD31.invertDisplay(FALSE); // normal Display Mode:white-on-black
  delay(500);
  BMD31.invertDisplay(TRUE); // invert Display Mode:black-on-white
  delay(500);
  BMD31.invertDisplay(FALSE); // normal Display Mode:white-on-black
  delay(500);
}

void test_dim(void)
{
  BMD31.dim(TRUE); // dim Mode:contrast-0x00
  delay(500);
  BMD31.dim(FALSE); // normal Brightness Mode:contrast-0xCF
  delay(500);
  BMD31.dim(TRUE); // dim Mode:contrast-0x00
  delay(500);
  BMD31.dim(FALSE); // normal Brightness Mode:contrast-0xCF
  delay(500);
}
