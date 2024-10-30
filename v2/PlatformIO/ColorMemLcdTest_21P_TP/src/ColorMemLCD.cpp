/*********************************************************************
Library for the LPM013M126A 176×176 Japan Display Color memory LCD
inside the SMA-Q2 / SMA-TIME smartwatch.

The ColorMemLCD library is based on the mbed library for the JDI color memory LCD LPM013M126A
by Tadayuki Okamoto, originally released under the MIT License, and the Adafruit SHARP Memory display library.

Uses hardware SPI on the nrf52832 and an external signal for the EXTCOMIN (P0.06).
By default the EXTMODE pin of the display is set HIGH via a 10k resistor,
which means that the display expects toggling this pin from time to time (at least
once per minute, see datasheet). If EXTMODE is set LOW (use a soldering iron to move
the resistor, there is an unpopulated place for that), the "toggle" has to be done in
software, like in the Adaruit library.
A. Jordan 2018.
*********************************************************************/

#include "ColorMemLCD.h"

/**************************************************************************
    Sharp Memory Display Connector
    -----------------------------------------------------------------------
    Pin   Function        Notes
    ===   ==============  ===============================
      1   VIN             3.3-5.0V (into LDO supply)
      2   3V3             3.3V out
      3   GND
      4   SCLK            Serial Clock
      5   MOSI            Serial Data Input
      6   CS              Serial Chip Select
      9   EXTMODE         COM Inversion Select (Low = SW clock/serial)
      7   EXTCOMIN        External COM Inversion Signal
      8   DISP            Display On(High)/Off(Low)

 **************************************************************************/

// new stuff
/** @def
 * LCD_Color SPI commands
 */
#define LCD_COLOR_CMD_UPDATE (0x90)         //!< Update Mode (4bit Data Mode)
#define LCD_COLOR_CMD_UPDATE_3BIT (0x80)         //!< Update Mode (4bit Data Mode)
#define LCD_COLOR_CMD_ALL_CLEAR (0x20)      //!< All Clear Mode
#define LCD_COLOR_CMD_NO_UPDATE (0x00)      //!< No Update Mode
#define LCD_COLOR_CMD_BLINKING_WHITE (0x18) //!< Display Blinking Color Mode (White)
#define LCD_COLOR_CMD_BLINKING_BLACK (0x10) //!< Display Blinking Color Mode (Black)
#define LCD_COLOR_CMD_INVERSION (0x14)      //!< Display Inversion Mode

#define TOGGLE_VCOM       \
  do                      \
  {                       \
    polarity = !polarity; \
  } while (0);

bool pointInPolygon(int16_t x, int16_t y, const std::vector<Point> &polygon)
{
  int i, j, nvert = polygon.size();
  bool inside = false;
  for (i = 0, j = nvert - 1; i < nvert; j = i++)
  {
    if (((polygon[i].y > y) != (polygon[j].y > y)) &&
        (x < (polygon[j].x - polygon[i].x) * (y - polygon[i].y) / (double)(polygon[j].y - polygon[i].y) + polygon[i].x))
    {
      inside = !inside;
    }
  }
  return inside;
}

/* ************* */
/* CONSTRUCTORS  */
/* ************* */
ColorMemLCD::ColorMemLCD(SPIClass *theSPI, uint8_t clk, uint8_t mosi, uint8_t ss, uint8_t extcomin, uint32_t freq) : Adafruit_GFX(LCD_DEVICE_WIDTH, LCD_DEVICE_HEIGHT)
{
  spidev = new Adafruit_SPIDevice(ss, freq, SPI_BITORDER_MSBFIRST, SPI_MODE0,
                                  theSPI);
  _clk = clk;
  _mosi = mosi;
  _ss = ss;
  _extcomin = extcomin;
  // Set pin state before direction to make sure they start this way (no glitching)
  digitalWrite(_ss, HIGH);
  pinMode(_ss, OUTPUT);
  pinMode(_extcomin, OUTPUT);

  /* initialize variables */
  char_x = 0;
  char_y = 0;
  trans_mode = LCD_TRANSMODE_OPAQUE;
  window_x = 0;
  window_y = 0;
  window_w = LCD_DISP_WIDTH;
  window_h = LCD_DEVICE_HEIGHT; // LCD_DISP_HEIGHT_MAX_BUF;

  polarity = 0;
  blink_cmd = 0x00;
  // extcomin_stat = 0;
  // digitalWrite(_extcomin, extcomin_stat);
  /* set default color */
  // foreground( LCD_COLOR_WHITE );
  // background( LCD_COLOR_BLACK );

  /* initialize temporary buffer */
  memset(&cmd_buf[0], 0, sizeof(cmd_buf));
  memset(&disp_buf[0], (char)((_background & 0x0F) | ((_background & 0x0F) << 4)), sizeof(disp_buf));
  memset(&file_buf[0], 0, sizeof(file_buf));

  /* display turn ON */
  // command_AllClear();
  //_disp = 1;
  clearDisplay();
}

/** Set a pixel on the window memory */
void ColorMemLCD::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if (clipEnabled)
  {
    bool inside = pointInPolygon(x, y, clipPath);
    if ((clipInside && !inside) || (!clipInside && inside))
    {
      return; // 不绘制此像素
    }
  }
  // 上面是 clipPath 判断逻辑

  if ((window_x > x) ||
      ((window_x + window_w) <= x) ||
      (window_y > y) ||
      ((window_y + window_h) <= y))
  {
    /* out of display buffer */
    return;
  }

  if ((x % 2) == 0)
  {
    disp_buf[((window_w / 2) * (y - window_y)) + ((x - window_x) / 2)] &= 0x0F;
    disp_buf[((window_w / 2) * (y - window_y)) + ((x - window_x) / 2)] |= ((color & 0x0F) << 4);
  }
  else
  {
    disp_buf[((window_w / 2) * (y - window_y)) + ((x - window_x) / 2)] &= 0xF0;
    disp_buf[((window_w / 2) * (y - window_y)) + ((x - window_x) / 2)] |= ((color & 0x0F));
  }
}

/** Set a pixel - for transrucent mode */

// void ColorMemLCD::pixel_alpha( int x, int y, int color )
// {
// if( ( window_x > x )||
// ( ( window_x + window_w ) <= x )||
// ( window_y > y )||
// ( ( window_y + window_h ) <= y ) ) {
// /* out of display buffer */
// return;
// }

// if( ( x % 2 ) == 0 ) {
// disp_buf[ ( (window_w / 2) * ( y - window_y ) ) + ( ( x - window_x ) / 2 ) ] &= ( ( ( color & 0x0F ) << 4 ) | 0x0F );
// }
// else {
// disp_buf[ ( (window_w / 2) * ( y - window_y ) ) + ( ( x - window_x ) / 2 ) ] &= ( ( ( color & 0x0F )      ) | 0xF0 );
// }
// }

/** Fill the window memory with background color */
void ColorMemLCD::cls(void)
{
  memset(&disp_buf[0], (char)((_background & 0x0F) | ((_background & 0x0F) << 4)), sizeof(disp_buf));
}

void ColorMemLCD::begin()
{
  if (!spidev->begin()) {
    Serial.println("Failed to initialize SPI communication");
  }
}

/** set transpalent effect */
void ColorMemLCD::setTransMode(char mode)
{
  trans_mode = mode;
}

/** Transfer to the LCD from diaply buffer */
void ColorMemLCD::refreshBySingleLine()
{
  int32_t i;
  int copy_width;
  if (window_x + window_w < LCD_DISP_WIDTH)
  {
    copy_width = (window_w / 2);
  }
  else
  {
    copy_width = ((LCD_DISP_WIDTH - window_x) / 2);
  }

  for (i = 0; i < window_h; i++)
  {

    if (window_y + i > LCD_DISP_HEIGHT)
    {
      /* out of window system */
      break;
    }

    /* initialize command buffer */
    memset(&cmd_buf[0], (char)((_background & 0x0F) | ((_background & 0x0F) << 4)), sizeof(cmd_buf));

    /* copy to command buffer */
    memcpy(&cmd_buf[(window_x / 2)], &disp_buf[(window_w / 2) * i], copy_width);

    /* send cmaoond request */
    sendLineCommand(&cmd_buf[0], window_y + i);
  }
}


/** Transfer to the LCD from disply buffer */
void ColorMemLCD::refresh()
{
  int32_t i;
  int copy_width;
  if (window_x + window_w < LCD_DISP_WIDTH)
  {
    copy_width = (window_w / 2);
  }
  else
  {
    copy_width = ((LCD_DISP_WIDTH - window_x) / 2);
  }

  spidev->beginTransaction();
  digitalWrite(_ss, HIGH);
  spidev->transfer(LCD_COLOR_CMD_UPDATE | (polarity << 6));

  for (i = 0; i < window_h; i++)
  {

    if (window_y + i > LCD_DISP_HEIGHT)
    {
      /* out of window system */
      break;
    }

    /* initialize command buffer */
    memset(&cmd_buf[0], (char)((_background & 0x0F) | ((_background & 0x0F) << 4)), sizeof(cmd_buf));

    /* copy to command buffer */
    memcpy(&cmd_buf[(window_x / 2)], &disp_buf[(window_w / 2) * i], copy_width);

    /* send cmaoond request */
    // sendLineCommand(&cmd_buf[0], window_y + i);

    spidev->transfer(window_y + i + 1); // line 8 bits
    spidev->transfer((uint8_t *)&cmd_buf[0], (LCD_DISP_WIDTH / 2));
    // dummy 6+2 bits
    spidev->transfer(0x00);
  }

  digitalWrite(_ss, LOW);
  spidev->endTransaction();
}

/** Transfer to the LCD from display buffer */  
void ColorMemLCD::refresh_3bit()  
{  
    int32_t i;  
    int copy_bits;  

    // Calculate the total number of bits to copy per line  
    if (window_x + window_w <= LCD_DISP_WIDTH)  
    {  
        copy_bits = window_w * 3; // Each pixel is 3 bits in rgb111  
    }  
    else  
    {  
        copy_bits = (LCD_DISP_WIDTH - window_x) * 3;  
    }  

    // Calculate the number of bytes per line  
    int copy_bytes = (copy_bits + 7) / 8; // Round up to the nearest whole byte  

    for (i = 0; i < window_h; i++)  
    {  
        if (window_y + i >= LCD_DISP_HEIGHT)  
        {  
            /* Out of display range */  
            break;  
        }  

        /* Initialize command buffer */  
        memset(cmd_buf, 0, sizeof(cmd_buf));  

        /* Copy pixel data to command buffer without dummy bits */  
        int line_start_bit = (window_w * 3) * i; // Start bit for the current line in disp_buf  
        int byte_offset = line_start_bit / 8;  
        int bit_offset = line_start_bit % 8;  

        // Copy the bits from disp_buf to cmd_buf  
        for (int j = 0; j < copy_bits; j++)  
        {  
            // Calculate source bit position  
            int src_bit_pos = line_start_bit + j;  
            int src_byte = src_bit_pos / 8;  
            int src_bit = src_bit_pos % 8;  

            // Get the bit value  
            uint8_t bit_val = (disp_buf[src_byte] >> (7 - src_bit)) & 0x01;  

            // Calculate destination bit position  
            int dst_bit_pos = j;  
            int dst_byte = dst_bit_pos / 8;  
            int dst_bit = dst_bit_pos % 8;  

            // Set the bit in cmd_buf  
            cmd_buf[dst_byte] |= bit_val << (7 - dst_bit);  
        }  
        // memset(&cmd_buf[0], 0x80, 66);

        /* Send command request */  
        sendLineCommand(&cmd_buf[0], window_y + i);  
    }
}

/** send data packet */
void ColorMemLCD::sendLineCommand(char *line_cmd, int line)
{
  int32_t j;

  if ((line < 0) ||
      (line >= LCD_DEVICE_HEIGHT))
  {
    /* out of device size */
    return;
  }

  // delayMicroseconds(6);
  spidev->beginTransaction();
  digitalWrite(_ss, HIGH);
  // delayMicroseconds(6);
  sendbyte(LCD_COLOR_CMD_UPDATE | (polarity << 6)); // Command
  TOGGLE_VCOM;
  sendbyte(line + 1); // line
  sendBytes((uint8_t *)line_cmd, (LCD_DISP_WIDTH / 2));
  // SPI.endTransaction();
  // for( j = 0 ; j < (LCD_DISP_WIDTH/2) ; j++ ) {
  //     if( j >= (LCD_DEVICE_WIDTH/2) ) {
  //         /* out of device size */
  //         break;
  //     }
  //     sendbyte(line_cmd[j]);        // data
  // }
  // for( ; j < (LCD_DEVICE_WIDTH/2) ; j++ ) {
  //     /* padding to device size */
  //     sendbyteLSB( 0x00 );
  // }

  sendbyte(0x00);
  sendbyte(0x00);
  // delayMicroseconds(6);
  digitalWrite(_ss, LOW);
  spidev->endTransaction();
}


/** send data packet */
void ColorMemLCD::sendLineCommand3bit(char *line_cmd, int line)
{
  int32_t j;

  if ((line < 0) ||
      (line >= LCD_DEVICE_HEIGHT))
  {
    /* out of device size */
    return;
  }

  // delayMicroseconds(6);
  spidev->beginTransaction();
  digitalWrite(_ss, HIGH);
  // delayMicroseconds(6);
  sendbyte(LCD_COLOR_CMD_UPDATE_3BIT | (polarity << 6)); // Command
  TOGGLE_VCOM;
  sendbyte(line + 1); // line
  sendBytes((uint8_t *)line_cmd, (LCD_DISP_WIDTH *3/8));
  // SPI.endTransaction();
  // for( j = 0 ; j < (LCD_DISP_WIDTH/2) ; j++ ) {
  //     if( j >= (LCD_DEVICE_WIDTH/2) ) {
  //         /* out of device size */
  //         break;
  //     }
  //     sendbyte(line_cmd[j]);        // data
  // }
  // for( ; j < (LCD_DEVICE_WIDTH/2) ; j++ ) {
  //     /* padding to device size */
  //     sendbyteLSB( 0x00 );
  // }

  sendbyte(0x00);
  sendbyte(0x00);
  // delayMicroseconds(6);
  digitalWrite(_ss, LOW);
  spidev->endTransaction();
}

/** Command to blink */
void ColorMemLCD::setBlinkMode(char mode)
{
  switch (mode)
  {
  case LCD_BLINKMODE_NONE:
    /* Blinking None    */
    blink_cmd = LCD_COLOR_CMD_NO_UPDATE;
    break;
  case LCD_BLINKMODE_WHITE:
    /* Blinking White   */
    blink_cmd = LCD_COLOR_CMD_BLINKING_WHITE;
    break;
  case LCD_BLINKMODE_BLACK:
    /* Blinking Black   */
    blink_cmd = LCD_COLOR_CMD_BLINKING_BLACK;
    break;
  case LCD_BLINKMODE_INVERSE:
    /* Inversion Mode   */
    blink_cmd = LCD_COLOR_CMD_INVERSION;
    break;
  default:
    /* No Update */
    blink_cmd = LCD_COLOR_CMD_NO_UPDATE;
    break;
  }

  digitalWrite(_ss, HIGH);
  delayMicroseconds(6);
  sendbyte(blink_cmd | (polarity << 6));
  TOGGLE_VCOM;
  sendbyteLSB(0x00);
  delayMicroseconds(6);
  digitalWrite(_ss, LOW);
}
/* *************** */
/* PRIVATE METHODS */
/* *************** */

inline void ColorMemLCD::sendBytes(uint8_t *data, uint16_t len)
{
  spidev->transfer(data, len);
  // SPI.transfer(data, len);
}
/**************************************************************************/
/*!
    @brief  Sends a single byte in (pseudo)-SPI.
*/
/**************************************************************************/
inline void ColorMemLCD::sendbyte(uint8_t data)
{
  // spidev->setBitOrder(MSBFIRST);
  spidev->transfer(data);
  // SPI.setBitOrder(MSBFIRST);
  // SPI.transfer(data);
}

void ColorMemLCD::sendbyteLSB(uint8_t data)
{
  // spidev->setBitOrder(LSBFIRST);
  spidev->transfer(data);
  // SPI.setBitOrder(LSBFIRST);
  // SPI.transfer(data);
}

// 1<<n is a costly operation on AVR -- table usu. smaller & faster
// static const uint8_t PROGMEM
//   set[] = {  1,  2,  4,  8,  16,  32,  64,  128 },
//   clr[] = { ~1, ~2, ~4, ~8, ~16, ~32, ~64, ~128 };

/**************************************************************************/
/*!
    @brief Clears the screen
*/
/**************************************************************************/
void ColorMemLCD::clearDisplay()
{
  memset(&disp_buf[0], (char)((_background & 0x0F) | ((_background & 0x0F) << 4)), sizeof(disp_buf));

  // Send the clear screen command rather than doing a HW refresh (quicker)
  digitalWrite(_ss, HIGH);
  // sendbyte(_sharpmem_vcom | SHARPMEM_BIT_CLEAR);
  sendbyte(LCD_COLOR_CMD_ALL_CLEAR | (polarity << 6));
  TOGGLE_VCOM;
  sendbyteLSB(0x00);
  // extcomin_stat=!extcomin_stat;
  // digitalWrite(_extcomin, extcomin_stat); //toggle extcomin
  digitalWrite(_ss, LOW);
}

void ColorMemLCD::setClipPath(const std::vector<Point> &path)
{
  clipPath = path;
  clipEnabled = true;
  clipInside = true; // 默认裁剪内部
}

void ColorMemLCD::clipOutPath(bool enable)
{
  clipInside = !enable;
}

// 清除裁剪路径
void ColorMemLCD::clearClipPath()
{
  clipPath.clear();
  clipEnabled = false;
}