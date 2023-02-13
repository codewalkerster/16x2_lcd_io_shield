#ifndef __LCD_H__
#define __LCD_H__

#define LCD_ROW             2   // 16 Char
#define LCD_COL             16  // 2 Line
#define LCD_BUS             4   // Interface 4 Bit mode

#define PORT_LCD_RS     7   // GPIOY.BIT3(#83)
#define PORT_LCD_E      0   // GPIOY.BIT8(#88)
#define PORT_LCD_D4     2   // GPIOX.BIT19(#116)
#define PORT_LCD_D5     3   // GPIOX.BIT18(#115)
#define PORT_LCD_D6     1   // GPIOY.BIT7(#87)
#define PORT_LCD_D7     4   // GPIOX.BIT7(#104)

#endif  //__LCD_H__
