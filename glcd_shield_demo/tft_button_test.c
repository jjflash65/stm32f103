/* -------------------------------------------------------
                         tft_button_test.c

     Demoprogramm fuer GLCD mit Aufloesung  128 x 128
     oder 128x160 Pixel

     MCU   :  STM32F103C8T6
     Takt  :  interner Takt 64 MHz

     16.10.2016  R. Seelig
   ------------------------------------------------------ */

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <libopencm3.h>

#include "sysf103_init.h"
#include "./tftdisplay.h"
#include "my_printf.h"

#define keyright_init() PA1_input_init()      // Taste rechts
#define keyup_init()    PA4_input_init()      // Taste oben
#define keyleft_init()  PB1_input_init()      // Taste links
#define keydwn_init()   PB3_input_init()      // Taste runter

#define is_keyright()   ( !(is_PB4()) )
#define is_keyup()      ( !(is_PB5()) )
#define is_keyleft()    ( !(is_PB1()) )
#define is_keydwn()     ( !(is_PB3()) )


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

void key_init(void)
{
  keyright_init();
  keyup_init();
  keyleft_init();
  keydwn_init();
}

/* -------------------------------------------------------
                           printxyf

     gibt einen String an den angegebenen Koordinaten
     in der Farbe f aus
   ------------------------------------------------------- */
void printxyf(uint8_t x, uint8_t y, uint8_t f, char *s)
{
  gotoxy(x,y);
  textcolor= rgbfromega(f);
  printf("%s",s);
}


/* -------------------------------------------------------
                             main
   ------------------------------------------------------- */
int main(void)
{

  sys_init();
  lcd_init();

  key_init();
  outmode= 1;

  printfkomma= 2;

  bkcolor= rgbfromega(15);
  textcolor= rgbfromega(0);
  clrscr();
  fillrect(0,0,159,26, rgbfromega(1));
  textcolor= rgbfromega(14);
  outtextxy(8,4,0,"TFT 160x128");
  outtextxy(8,14,0,"Four Buttons");
  textcolor= rgbfromega(1);

  circle(50,64,7, rgbfromega(1));    // Kreis fuer linken Button
  circle(110,64,7, rgbfromega(1));   // dto. rechts
  circle(80,40,7, rgbfromega(1));    // dto. oben
  circle(80,88,7, rgbfromega(1));    // dto. unten

  while(1)
  {
    if (is_keyright()) fillcircle(110,64,4, rgbfromega(9)); else fillcircle(110,64,4, rgbfromega(7));
    if (is_keyleft()) fillcircle(50,64,4, rgbfromega(12)); else fillcircle(50,64,4, rgbfromega(7));

    if (is_keyup()) fillcircle(80,40,4, rgbfromega(2)); else fillcircle(80,40,4, rgbfromega(7));
    if (is_keydwn()) fillcircle(80,88,4, rgbfromega(13)); else fillcircle(80,88,4, rgbfromega(7));
    delay(50);
  }

}
