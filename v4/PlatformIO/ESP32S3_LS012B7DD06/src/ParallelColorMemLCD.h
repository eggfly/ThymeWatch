
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <vector>

#include <Adafruit_GFX.h>

/** @def
 * window system define
 */
#define LCD_DISP_WIDTH (240)
#define LCD_DISP_HEIGHT (240)


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


extern GFXcanvas8 *canvas;

