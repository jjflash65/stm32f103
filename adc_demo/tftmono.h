/* -----------------------------------------------------
                        tftmono.h

    Header fuer Softwaremodul fuer monochrome Displays.
    Momentan unterstuetzte Controller:

        - PCD8544
        - SSD1306  (SPI und I2C)

    Hardware  : STM32F103
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    Hinweis: Bei Verwendung eines I2C-Displays wird
    I2C2 des STM32F103 verwendet

    07.02.2020   R. Seelig
  ------------------------------------------------------ */

/* ----------------------------------------------------
    Pinbelegungen:

                     G   V           R
                     N   c   D   D   E   D   C
                     D   c   0   1   S   C   s
                 +-------------------------------+
                 |   o   o   o   o   o   o   o   |
                 |                               |
                 |   -------------------------   |
                 |  |                         |  |
                 |  |                         |  |
                 |  |     OLED1306 - SPI      |  |
                 |  |                         |  |
                 |  |                         |  |
                 |  |                         |  |
                 |   -----+-------------+-----   |
                 |        |             |        |
                 |        +-------------+        |
                 +-------------------------------+

       STM32F103            | Display OLED1306 - SPI
                              (128x64 Pixel)
    ------------------------------------------------
                              GND              (1)
                              Vcc              (2)
       PA5          ---       D0 (CLK)         (3)
       PA7          ---       D1 (DIN / MOSI)) (4)
       PB6          ---       RST              (5)
       PB5          ---       DC               (6)
       PB1          ---       CE               (7)



                          V   G   S   S
                          c   N   C   DC
                          c   D   L   As
                +-------------------------------+
                |         o   o   o   o         |
                |                               |
                |   -------------------------   |
                |  |                         |  |
                |  |                         |  |
                |  |     OLED1306 - I2C      |  |
                |  |                         |  |
                |  |                         |  |
                |  |                         |  |
                |   -----+-------------+-----   |
                |        |             |        |
                |        +-------------+        |
                +-------------------------------+

       STM32F103            | Display OLED1306 - I2C
                              (128x64 Pixel)
    ------------------------------------------------
                              GND              (1)
                              Vcc              (2)
       PB10         ---       SCL              (3)
       PB11         ---       SDA              (4)


                     R           D   C   V   L   G
                     S   C   D   I   L   C   E   N
                     T   E   C   N   K   C   D   D
                 +-----------------------------------+
                 |   o   o   o   o   o   o   o   o   |
                 |                                   |
                 |   ---------------------------     |
                 |  |                           |    |
                 |  |                           |    |
                 |  |        N5110 - SPI        |    |
                 |  |                           |    |
                 |  |                           |    |
                 |  |                           |    |
                 |   -------+-------------+-----     |
                 |          |             |          |
                 |          +-------------+          |
                 +-----------------------------------+



       STM32F103            | Display N5110 - SPI
                              (84x48 Pixel)
    ------------------------------------------------
       PB6          ---       RST              (1)
       PB1          ---       CE               (2)
       PB5          ---       DC               (3)
       PA7          ---       D1 (DIN / MOSI)) (4)
       PA5          ---       D0 (CLK)         (5)
                              Vcc (+3,3V)      (6)
                              LED (+3,3V)      (7)
                              GND              (8)




*/


#ifndef in_tftmono
  #define in_tftmono

  #include <stdlib.h>                             // fuer abs()

  #include "sysf103_init.h"
  #include "spi_f103.h"

  /* --------------------------------------------------
       verwendeter Displaycontroller, es darf nur ein
        Controller selektiert sein
     -------------------------------------------------- */
  #define pcd8544               0                 // Nokia Display 3310, 5110, 3410
  #define ssd1306               1                 // OLED Display mit SSD1306 Controller

  #define use_i2c               1                 // nur gueltig fuer SSD1306 Displays
                                                  // 0 : Ansteuerung ueber SPI
                                                  // 1 : Ansteuerung ueber I2C
  #if (ssd1306 == 0)
    #undef use_i2c
    #define use_i2c             0                 // wenn Display kein SSD1306, dann grundsaetzlich
                                                  // kein I2C
  #endif


  /* --------------------------------------------------
       Schriftstile
     -------------------------------------------------- */

  #define font5x7_enable        1                 // 1: Schriftstil verfuegbar, 0: nicht verfuegbar
  #define font8x8_enable        1                 // 1: Schriftstil verfuegbar, 0: nicht verfuegbar

  enum font { fnt5x7, fnt8x8 };

  /* --------------------------------------------------
                   Displayeigenschaften
     -------------------------------------------------- */
  #define _xres                 128
  #define _yres                 64


  #define  fb_enable            1                 //   Framebuffer enable
                                                  //   1 = es wird RAM fuer Framebuffer
                                                  //       reserviert und Grafikfunktionen werden eingebunden
                                                  //   0 = kein Displayram und keine Grafikfunktionen

  #define  fb_size              1038              // Framebuffergroesse in Bytes (wenn fb_enable)


  #define  fillrect_enable      1                 //   1 = Code mit einbinden, 0 = nicht einbinden
  #define  fillellipse_enable   1                 //   dto.
  #define  putcharxy_enable     1                 //   1 = Textausgabe im Framebuffer ermoeglichen
                                                  //   0 = nicht ermoeglichen

  #define  showimage_enable     1                 //   1 = Funktionen zur Anzeige von s/W Bitmaps einbinden
                                                  //   0 = nicht einbinden

  #if (use_i2c == 1)
    #define ssd1306_addr        0x78              //   I2C Adresse des SSD1306 Displaycontrollers
  #endif

  #if (pcd8544 == 1)
    #undef _xres
    #undef _yres
  #define _xres                 84
  #define _yres                 48
  #endif


  // --------------------------------------------
  //              globale Variable
  // --------------------------------------------

  extern uint8_t bkcolor;
  extern uint8_t invchar;

  extern uint8_t aktxp;                                // Beinhaltet die aktuelle Position des Textcursors in X-Achse
  extern uint8_t aktyp;                                // dto. fuer die Y-Achse

  extern uint8_t fontnr;                               //  0 : 5x7  Font
                                                       //  1 : 8x8  Font

  extern uint8_t fontsizex;
  extern uint8_t textsize;                             // Skalierung der Ausgabeschriftgroesse

  #if (use_i2c == 0)
    extern const uint8_t lcdinit_seq[];
  #endif

  #if (fb_enable == 1)
    extern uint8_t txoutmode;
  #endif


  /* ------------------------------------------------
                     Anschlusspins SPI
     ------------------------------------------------ */

  #define spi_mosi_init()    PA7_output_init()
  #define spi_sck_init()     PA5_output_init()

  #define spi_mosi_set()     PA7_set()
  #define spi_mosi_clr()     PA7_clr()

  #define spi_sck_set()      PA5_set()
  #define spi_sck_clr()      PA5_clr()

  #define lcd_rst_init()     PB6_output_init()
  #define lcd_dc_init()      PB5_output_init()
  #define lcd_ce_init()      PB1_output_init()

  #define lcd_dc_set()       PB5_set()
  #define lcd_dc_clr()       PB5_clr()

  #define lcd_ce_set()       PB1_set()
  #define lcd_ce_clr()       PB1_clr()

  #define lcd_rst_set()      PB6_set()
  #define lcd_rst_clr()      PB6_clr()

  #define lcd_enable()       lcd_ce_clr()                  // geloeschte CE Leitung bedeutet aktives Display
  #define lcd_disable()      lcd_ce_set()

  /* ------------------------------------------------
                 Ende Anschlusspins SPI
     ------------------------------------------------ */

  #if (use_i2c == 0)
    #define lcd_cmdmode()    lcd_dc_clr()                // D/C == 0 =>
    #define lcd_datamode()   lcd_dc_set()
  #endif

  #if (use_i2c == 1)
    #define lcd_cmdmode()    {   i2c_start(ssd1306_addr,0);   \
                                 i2c_write(0x00); }

    #define lcd_datamode()    {   i2c_start(ssd1306_addr,0);   \
                                  i2c_write(0x40); }

  #endif

  /* ------------------------------------------------
                       Prototypen
     ------------------------------------------------ */

  uint8_t reverse_byte(uint8_t b);

  #if (use_i2c == 1)
    void i2clcd_writecmd(uint8_t cmd);
    void i2clcd_writedata(uint8_t data);
  #endif

  #if (use_i2c == 0)
    void spi_delay(void);
    void spi_sw_init(void);
    void out_byte(uint8_t value);
  #endif

  void lcd_setxypos(uint8_t x, uint8_t y);
  void lcd_setxybyte(uint8_t x, uint8_t y, uint8_t value);
  void gotoxy(uint8_t x, uint8_t y);
  void clrscr(void);
  void lcd_init(void);

  void setfont(uint8_t fnr);
  uint8_t doublebits(uint8_t b, uint8_t nibble);
  void lcd_putchar(uint8_t ch);

  #if (fb_enable == 1)

    void fb_init(uint8_t x, uint8_t y);
    void fb_clear(void);
    void fb_show(uint8_t x, uint8_t y);
    void fb_putpixel(uint8_t x, uint8_t y, uint8_t col);
    void line(int x0, int y0, int x1, int y1, uint8_t col);
    void rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t col);
    void ellipse(int xm, int ym, int a, int b, uint8_t col );
    void circle(int x, int y, int r, uint8_t col );
    void fastxline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t col);

    #if (fillrect_enable == 1)
      void fillrect(int x1, int y1, int x2, int y2, uint8_t col);
    #endif

    #if (fillellipse_enable == 1)
      void fillellipse(int xm, int ym, int a, int b, uint8_t col );
      void fillcircle(int x, int y, int r, uint8_t col );
    #endif

    #if (putcharxy_enable == 1)
      void fb_putcharxy(uint8_t x, uint8_t y, uint8_t ch);
      void fb_outtextxy(uint8_t x, uint8_t y, uint8_t dir, char *p);
    #endif

  #endif                  // Framebuffer Funktionen

  #if (showimage_enable == 1)

    void showimage(uint8_t ox, uint8_t oy, const uint8_t* const image, char mode);

  #endif

#endif
