/* -------------------------------------------------------
                        glcd_spi_spiro.c

     Demoprogramm fuer grafische TFT-Displays mit Auf-
     loessungen von 128x128 oder 160x128 Pixeln.

     Einstellungen des verwendeten Displays in tftdisplay.h
     und tft_pindefs.h

     MCU   :  STM32F103
     Takt  :  72 MHz

     12.02.2020  R. Seelig
   ------------------------------------------------------ */

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <libopencm3.h>

#include "sysf103_init.h"
#include "my_printf.h"
#include "tftdisplay.h"

#define printf       my_printf


/* --------------------------------------------------------
   my_putchar

   wird von my-printf / printf aufgerufen und hier muss
   eine Zeichenausgabefunktion angegeben sein, auf das
   printf dann schreibt !
   -------------------------------------------------------- */
void my_putchar(char ch)
{
  lcd_putchar(ch);
}

/* --------------------------------------------------------
                      spiro_generate

       zeichnet einee Spirographen auf das Display
   -------------------------------------------------------- */
void spiro_generate(int inner, int outer, int evol, int resol, uint16_t col)
{
  const int c_width  = 128;
  const int c_height = 128;
  float     inner_xpos, inner_ypos;
  float     outer_xpos, outer_ypos;
  float     j, k;
  int       i;

  inner_xpos = (c_width / 2.0f);
  inner_ypos = (c_height / 2.0f) + inner;

  outer_xpos= inner_xpos;
  outer_ypos= inner_ypos + outer;
  turtle_moveto(outer_xpos, outer_ypos);

  for (i= 0; i< resol + 1; i++)
  {
    j= ((float)i / resol) * (2.0f * M_PI);
    inner_xpos = (c_width / 2.0f) + (inner * sin(j));
    inner_ypos = (c_height / 2.0f) + (inner * cos(j));

    k= j * ((float)evol / 10.0f);

    outer_xpos= inner_xpos + (outer * sin(k));
    outer_ypos= inner_ypos + (outer * cos(k));

    turtle_lineto(outer_xpos, outer_ypos, col);
    delay(5);
  }
}

/* --------------------------------------------------------
                           main
   -------------------------------------------------------- */
int main(void)
{
  uint16_t x;
  uint16_t coltmp;

  sys_init();

  lcd_init();

  printfkomma= 2;
  lcd_orientation(0);

  bkcolor= rgbfromega(0);
  textcolor= rgbfromvalue(0x00,0x80,0x80);
  clrscr();

  // ein paar Linien Zeichnen
  for (x= 0; x< 125; x+= 5)
  {
    line(0, 127, x, 0, rgbfromega(blue));
    delay(20);
  }
  for (x= 0; x< 125; x+= 5)
  {
    line(127, 127, x, 0, rgbfromega(grey));
    delay(20);
  }

  coltmp= rgbfromvalue(0x10, 0x10, 0x10);
  bkcolor= coltmp;
  gotoxy(1,1); printf("             ");
  gotoxy(1,2); printf("  Spirograph ");
  gotoxy(1,3); printf(" mit STM32F1 ");
  gotoxy(1,4); printf("             ");
  delay(2000);
  bkcolor= 0;
  clrscr();

  while(1)
  {
  spiro_generate(21, 21, 80, 220, rgbfromvalue(50, 255, 50));
  spiro_generate(32, 32, 140,220, rgbfromega(1));
  delay(5000);

  spiro_generate(32, 32, 140,220, rgbfromvalue(0x00,0x20,0x00));;
  spiro_generate(21, 21, 80, 220, rgbfromvalue(0x00,0xcf,0x00));
  delay(5000);

  spiro_generate(32, 32, 140,220, rgbfromega(4));
  spiro_generate(21, 21, 80, 220, rgbfromvalue(0xff,0x80,0x00));
  delay(5000);
  }
  gotoxy(0,0);
  printf("Spirograph");

}

