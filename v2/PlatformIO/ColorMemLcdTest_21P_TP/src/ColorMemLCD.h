
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

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <vector>

#include <Adafruit_GFX.h>

#include <Adafruit_SPIDevice.h>

#include <SPI.h>

// new stuff
/** @def
 * Device define
 */
#define LCD_DEVICE_WIDTH (176)
#define LCD_DEVICE_HEIGHT (176)

/** @def
 * window system define
 */
#define LCD_DISP_WIDTH (176)
#define LCD_DISP_HEIGHT (176)
#define LCD_DISP_HEIGHT_MAX_BUF (44)

/** @def
 * some RGB color definitions
 */
/*                                        R, G, B     */
#define LCD_COLOR_BLACK (0x00)   /*  0  0  0  0  */
#define LCD_COLOR_BLUE (0x02)    /*  0  0  1  0  */
#define LCD_COLOR_GREEN (0x04)   /*  0  1  0  0  */
#define LCD_COLOR_CYAN (0x06)    /*  0  1  1  0  */
#define LCD_COLOR_RED (0x08)     /*  1  0  0  0  */
#define LCD_COLOR_MAGENTA (0x0a) /*  1  0  1  0  */
#define LCD_COLOR_YELLOW (0x0c)  /*  1  1  0  0  */
#define LCD_COLOR_WHITE (0x0e)   /*  1  1  1  0  */

/** @def
 * ID for setTransMode
 */
#define LCD_TRANSMODE_OPAQUE (0x00)      //!< BackGroud is Opaque
#define LCD_TRANSMODE_TRANSPARENT (0x01) //!< BackGroud is Transparent
#define LCD_TRANSMODE_TRANSLUCENT (0x02) //!< BackGroud is Translucent

/** @def
 *ID for setBlinkMode
 */
#define LCD_BLINKMODE_NONE (0x00)    //!< Blinking None
#define LCD_BLINKMODE_WHITE (0x01)   //!< Blinking White
#define LCD_BLINKMODE_BLACK (0x02)   //!< Blinking Black
#define LCD_BLINKMODE_INVERSE (0x03) //!< Inversion Mode

/** A class for Color Memory LCD Library
 */


  struct Point {
    int16_t x;  
    int16_t y; 
  };
  
class ColorMemLCD : public Adafruit_GFX
{
public:
  ColorMemLCD(SPIClass *theSPI, uint8_t clk, uint8_t mosi, uint8_t ss, uint8_t extcomin, uint32_t freq);
  void begin(void);
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  void clearDisplay();
  void refreshBySingleLine(void);
  void refresh(void);
  void refresh_3bit(void);
  void setBlinkMode(char mode);
  void setTransMode(char mode);
  void cls(void);
  void setClipPath(const std::vector<Point>& path);
  void clipOutPath(bool enable);
  void clearClipPath();


private:
  Adafruit_SPIDevice *spidev = NULL;
  std::vector<Point> clipPath; // 裁剪多边形的点集合
  bool clipEnabled = false;    // 是否启用裁剪
  bool clipInside = true;      // true 表示仅绘制多边形内部，false 表示仅绘制多边形外部

  uint8_t _ss, _clk, _mosi, _extcomin;
  volatile uint8_t *dataport, *clkport;
  uint8_t _sharpmem_vcom, datapinmask, clkpinmask;

  /* polarity variable */
  char polarity;
  char blink_cmd;
  bool extcomin_stat;

  /* trans mode variable */
  char trans_mode;

  /* data for character variable */
  unsigned char *font;
  int char_x;
  int char_y;

  /* window  variable */
  int window_x;
  int window_y;
  int window_w;
  int window_h;
  uint16_t _foreground;
  uint16_t _background = LCD_COLOR_WHITE;

  /* temporary buffer */
  char cmd_buf[LCD_DISP_WIDTH / 2]; /* for sending command */
                                    // char    disp_buf[(LCD_DISP_WIDTH/2)*LCD_DISP_HEIGHT_MAX_BUF];   /* display buffer */
  char disp_buf[(LCD_DISP_WIDTH / 2) * LCD_DISP_HEIGHT];
  char file_buf[118];

  void sendbyte(uint8_t data);
  void sendBytes(uint8_t *data, uint16_t len);

  void sendbyteLSB(uint8_t data);
  void sendLineCommand(char *line_cmd, int line);
  void sendLineCommand3bit(char *line_cmd, int line);
};
