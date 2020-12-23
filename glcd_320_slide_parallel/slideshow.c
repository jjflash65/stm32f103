/* -------------------------------------------------------
                         slideshow.c

     Grafik-Demoprogramm

     Anzeigen von Bildern die im BMP- und im PCX-Format
     eingebettet sind.

     Zusaetzlich wird eine Mandelbrot Fraktalgrafik be-
     rechnet und angezeigt

     320 x 240 Pixel

     MCU   :  STM32F103cb     (128 kByte Flash)
     Takt  :  externer Takt 72 MHz

     HINWEIS: wird ein paralleles Display mit Anschluss-
     belegung nach Boardversion 1 verwendet, so
     ist in der MAIN nach

                         sys_init();

     die folgende Anweisung einzufuegen:

                         jtag_disable();

     ACHTUNG: Das hat zur Folge, dass der Controller
     nur noch mittels Bootloader geflasht werden kann.
     Ein loeschen (und neubeschreiben des Controllers) ist
     dann nur noch mit einer Verbindung "Connect under
     reset" moeglich

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
#include "gfx_pictures.h"

#include "./include_bmp/popstar.h"
#include "./include_bmp/popstarpal.h"
#include "./include_bmp/naturpic.h"
#include "./include_bmp/naturpicpal.h"

#define printf    my_printf

// #define MY_PI         3.14159265359f

// Aufloesung fuer Mandelbrot
#define graphwidth     320
#define graphheight    240

struct colvalue
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};


/* --------------------------------------------------------
                         mandelbrot

     Mandelbrotgenerator zum Erzeugen des "Apfelmaennchens"
   -------------------------------------------------------- */
void mandelbrot(void)
{
  uint16_t k, kt, x, y;
  float    dx, dy, xmin, xmax, ymin, ymax;
  float    jx, jy, wx, wy, tx, ty, r, m;
  struct   colvalue mrgb;

  kt= 100; m= 4.0;

  xmin= 1.7; xmax= -0.8; ymin= -1.0; ymax= 1.0;

//  alternative Zahlenwerte

//  xmin= -0.5328; xmax= -0.2078; ymin= 0.3742; ymax= 0.892;

  dx= (float)(xmax-xmin) / graphwidth;
  dy= (float) (ymax-ymin) / graphheight;

  for (x= 0; x< graphwidth; x++)
  {
    jx= xmin + ((float)x*dx);

    for (y= 0; y< graphheight; y++)
    {
      jy= ymin+((float)y*dy);

      k= 0; wx= 0.0; wy= 0.0;
      do
      {
        tx= wx*wx-(wy*wy+jx);
        ty= 2.0*wx*wy+jy;
        wx= tx;
        wy= ty;
        r= wx*wx+wy+wy;

        k++;
      } while ((r < m) & (k < kt));

      if (k< 3) {mrgb.b= k * 20; }
      if ((k> 2) && (k< 35)) { mrgb.b= k * 9; mrgb.g = k; mrgb.r= k*2 ; }
      if (k> 34) { mrgb.r= ( 128 ); mrgb.g= (k * 3 ); mrgb.b= (k); }
      if (k> 90) { mrgb.r= 0, mrgb.g= 0; mrgb.b= 0; }

      putpixel(x,y,rgbfromvalue(mrgb.r, mrgb.g, mrgb.b));

    }
  }
}


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
                              main
   -------------------------------------------------------- */
int main(void)
{
  sys_init();
  delay(1000);
//  jtag_disable();                    // nur bei parallelen Displays in Verbindung
                                     // mit Boardversion 1 notwendig.
                                     // Achtung: Flashen ist danach nur noch mittels
                                     // Bootloader moeglich

  lcd_init();
  lcd_orientation(1);

  bkcolor= rgbfromega(15);
  clrscr();

  while(1)
  {
    pcx256_show(0, 0, &naturpic[0],&naturpicpal[0]);
    bkcolor= rgbfromega(4);
    setfont(1); fntfilled= 0;
    gotoxy(12,3);
    textcolor= rgbfromvalue(255,128,40);
    outtextxy(410,40,1,"PCX-Grafik");
    delay(10000);
    pcx256_show(0, 0, &popstar[0],&popstarpal[0]);
    gotoxy(1,1);
    textcolor= rgbfromega(0);
    printf("Normale Textausgabe");
    delay(10000);
    bkcolor= rgbfromega(1);
    textcolor= rgbfromega(15);
    clrscr();
    gotoxy(1,0); printf("Apfelmaennchen");
    mandelbrot();
    delay(10000);
    bkcolor= rgbfromega(15);
    clrscr();
  }
}
