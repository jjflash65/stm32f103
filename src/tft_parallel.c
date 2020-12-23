/* ------------------------------------------------------------
                            tft_parallel.c

     Softwaremodul fuer farbige TFT-Displays mit 8-Bit
     Parallelbus

     unterstuetzte Displaycontroller:

         LCD 480x320
           - ili9481
           - ili9486

         LCD 320x240
           - SPFD5408
           - ili9341
           - ili9335

     MCU   :  STM32103
     Takt  :  intern oder extern

     13.03.2018  R. Seelig
 ------------------------------------------------------------ */

#include "tft_parallel.h"


int      t_lastx, t_lasty;                                       // x,y - Positionen der letzten Zeichenaktion von moveto

// ------------------------------------
//          Variable Farben
// ------------------------------------

// RGB565 Farbpallete der "EGA"-Farben
uint16_t egapalette [] =
    { 0x0000, 0x0015, 0x0540, 0x0555,
      0xa800, 0xa815, 0xaaa0, 0xad55,
      0x52aa, 0x52bf, 0x57ea, 0x57ff,
      0xfaaa, 0xfabf, 0xffea, 0xffff };

uint16_t textcolor;         // Beinhaltet die Farbwahl fuer die Vordergrundfarbe
uint16_t bkcolor;           // dto. fuer die Hintergrundfarbe


// ------------------------------------
//            Variable Text
// ------------------------------------

uint16_t aktxp;             // Beinhaltet die aktuelle Position des Textcursors in X-Achse
uint16_t aktyp;             // dto. fuer die Y-Achse
uint8_t  fontnr     = 0;    // standardmaessig ist 8x8 Font gesetzt
uint8_t  fontsizex  = 8;
uint8_t  fontsizey  = 8;

uint8_t  textsize;          // Skalierung der Ausgabeschriftgroesse
uint8_t  txoutmode = 0;     // Drehrichtung fuer die Textausgabe
uint8_t  fntfilled = 1;     // gibt an, ob eine Zeichenausgabe ueber einen Hintergrund gelegt
                            // wird, oder ob es mit der Hintergrundfarbe aufgefuellt wird
                            // 1 = Hintergrundfarbe wird gesetzt, 0 = es wird nur das Fontbitmap
                            // gesetzt, der Hintergrund wird belassen

// ------------------------------------
// fuer Displayausrichtung Orientation
// ------------------------------------

uint8_t orientation = 1;    // Standardmaessige Ausgabe quer

// Displaygroesse
uint16_t tftwidth = _xres;
uint16_t tftheight = _yres;

// ------------------------------------
//           Turtle-Grafiken
// ------------------------------------

 int t_lastx, t_lasty;       // x,y - Positionen der letzten Zeichenaktion von moveto


/* ----------------------------------------------------------
   rgbfromvalue

     Setzt einen 16-Bitfarbwert aus 3 einzelnen Farbwerten
     fuer (r)ot, (g)ruen und (b)lau zusammen.

       r,g,b  : 8-Bit Farbwerte fuer rot, gruen, blau. Aus
                diesen wird ein 16 Bit (RGB565) Farbwert
                generiert und dieser als Funktionsergebnis
                zurueck geliefert
   ---------------------------------------------------------- */
uint16_t rgbfromvalue(uint8_t r, uint8_t g, uint8_t b)
{
  uint16_t value;

  r= r >> 3;
  g= g >> 2;
  b= b >> 3;
  value= b;
  value |= (g << 5);
  value |= (r << 11);
  return value;
}

/* ----------------------------------------------------------
     rgbfromega

     liefert den 16-Bit Farbwert, der in der Ega-Farbpalette
     definiert ist.

         entry : Indexnummer der Farbe in egapalette
   ---------------------------------------------------------- */
uint16_t rgbfromega(uint8_t entry)
{
  return egapalette[entry];
}

#if (boardversion == 0)
  void byteout(uint16_t val)
  {
    // Bits an denen das Display angeschlossen ist ueber Schiebeoperationen
    // zusammenpfrimmeln. Dafuer gehts dann deutlich schneller !!!
    uint16_t outbye;

    val &= 0xff;

    outbye = (val & 0xfc) << 4;      // Bit 6..11  entspricht db2..db7
    outbye |= ((val & 0x03) << 12);  // Bit 12..13 entspricht db0..db1

    gpio_port_write(GPIOB,outbye | 1);   // Reset auf 1 belassen
  }
#endif

#if (boardversion == 1)

  void byteout(uint32_t val)
  {
    uint32_t outbyeA = 0;
    uint32_t outbyeB = 0;

    GPIO_BSRR(GPIOB)= 0x20ba0000;
    GPIO_BSRR(GPIOA)= 0x81000000;

    if (val & 0x01) outbyeB |= 0x2000;
    if (val & 0x02) outbyeB |= 0x0080;
    if (val & 0x04) outbyeB |= 0x0002;
    if (val & 0x08) outbyeB |= 0x0008;
    if (val & 0x10) outbyeB |= 0x0020;
    if (val & 0x20) outbyeB |= 0x0010;
    if (val & 0x40) outbyeA |= 0x8000;
    if (val & 0x80) outbyeA |= 0x0100;

    GPIO_BSRR(GPIOB) = outbyeB;
    GPIO_BSRR(GPIOA) = outbyeA;
  }

#endif

/* -------------------------------------------------------------
   LCD_BUS_WRITE

     legt ein Byte auf den Parallelbus und gibt anschliessend
     ein Taktimpuls aus (somit wird ein einzelnes Byte an das
     Display gesendet)
   ------------------------------------------------------------- */
void lcd_bus_write(uint8_t val)
{
  byteout(val);
  // lcd_wr Taktimpuls, ggf. ein Delay dazwischen
  lcd_wr_clr();
  lcd_wr_set();
}

/* -------------------------------------------------------------
   LCD_WRITE_COM

     sendet ein Befehlsbyte an das Display

     Parameter:
       val  : zu sendendes Kommando
   ------------------------------------------------------------- */
void lcd_write_com(uint8_t val)
{
  lcd_rs_clr();
  lcd_bus_write(val);     // val als Kommando schreiben
}

/* -------------------------------------------------------------
   LCD_WRITE_DATA

     sendet ein einzelnes Datum an das Display

     Parameter:
       val  : zu sendendes Datum
   ------------------------------------------------------------- */
void lcd_write_data(unsigned char val)
{
  lcd_rs_set();
  lcd_bus_write(val);    // val als Datum schreiben
}

/* ----------------------------------------------------------
     address_set

     setzt die Speicheradresse im LCD fuer nachfolgende
     Daten Schreiboperationen

       x1,y1, x2,y2  : Koordinaten im Displayram
   ---------------------------------------------------------- */
void address_set(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  lcd_write_com(0x2a);              // REG 2Ah = COLADDRSET (4 Werte)
  lcd_write_data(x1 >> 8);          // x1 LSB
  lcd_write_data(x1);               // x1 MSB
  lcd_write_data(x2 >> 8);          // x2 LSB
  lcd_write_data(x2);               // X2 MSB

  lcd_write_com(0x2b);              // REG 2Bh =PAGEADDRSET (4 Werte)
  lcd_write_data(y1 >> 8);          // y1 LSB
  lcd_write_data(y1);               // y1 MSB
  lcd_write_data(y2 >> 8);          // Y2 LSB
  lcd_write_data(y2);               // Y2 MSB

  lcd_write_com(0x2c);              // REG 2Ch = Memory Write
}

/* ----------------------------------------------------------
     invertcolor

     Farben werden invertiert (Negativbild) dargestellt
   ---------------------------------------------------------- */
void invertcolor()
{
  lcd_write_com(0x21);     // Invert Colors.
}

/* ----------------------------------------------------------
     normalcolor

     Farben werden normal dargestellt
   ---------------------------------------------------------- */
void normalcolor()
{
  lcd_write_com(0x20);     // Normal Colors.
}

/* -------------------------------------------------------------
   lcd_orientation

     legt fest, in welcher Bildschirmrichtung eine Ausgabe
     auf dem Display erfolgt.

     Parameter:
       ori   : legt die Ausgaberichtung fest.
               0 entspricht hierbei "hochkant",

               1 : 90 Grad zu ori= 0 gedreht
               2 : 180 Grad zu ori= 0 gedreht
               3 : 270 Grad zu ori= 0 gedreht
   ------------------------------------------------------------- */
void lcd_orientation (uint8_t ori)
{
  unsigned dat = 0;

  switch(ori)
  {
    case 0: dat = (1 << MEM_X) | (1 << MEM_BGR);
            tftwidth = _yres; tftheight = _xres; break;
    case 1: dat = (0 << MEM_Y)  | (0 << MEM_X)  | (1 << MEM_V) | (0 << MEM_L)  | (1 << MEM_BGR);
            tftwidth = _xres; tftheight = _yres; break;
    case 2: dat = (1 << MEM_Y) | (1 << MEM_BGR);
            tftwidth = _yres; tftheight = _xres; break;
    case 3: dat = (1 << MEM_Y) | (1<<MEM_X) | (1<<MEM_V) | (1 << MEM_BGR);
            tftwidth = _xres; tftheight = _yres; break;
    default : break;
  }
  lcd_write_com(0x36);     // memory access controll
  lcd_write_data(dat);
}

/* -------------------------------------------------------------
     lcd_init

     initialisiert das Display mit den in tabseq angegebenen
     werden (definiert in tft_parallel.h)
   ------------------------------------------------------------- */
void lcd_init(void)
{
  uint8_t  cmd_anz;
  uint8_t  arg_anz;
  uint16_t ms, i;

  const uint8_t *tabseq;

  lcd_d0_init();
  lcd_d1_init();
  lcd_d2_init();
  lcd_d3_init();
  lcd_d4_init();
  lcd_d5_init();
  lcd_d6_init();
  lcd_d7_init();

  lcd_rd_init();
  lcd_wr_init();
  lcd_rs_init();
  lcd_cs_init();
  lcd_rst_init();

  lcd_rd_set();
  lcd_wr_set();
  lcd_rs_set();
  lcd_cs_set();
  lcd_rst_set();

  // Display reset
  delay(5);
  lcd_rst_clr();
  delay(15);
  lcd_rst_set();
  delay(15);//delay of 15mS.

  lcd_cs_set();
  lcd_wr_set();
  lcd_cs_clr();


  tabseq= &lcdinit_seq[0];

  // ein einzelnes Kommando besteht aus mehreren Datenbytes. Zuerst wird ein Kommandobyte
  // auf dem SPI geschickt, anschliessend die zu diesem Kommandobytes dazugehoerigen Datenbytes
  // abschliessend wird evtl. ein Timingwait eingefuegt. Dieses wird fuer alle vorhandenen
  // Kommandos durchgefuehrt

  cmd_anz = *tabseq++;               // Anzahl Gesamtkommandos

  while(cmd_anz--)                                 // alle Kommandos auf SPI schicken
  {
    lcd_write_com(*tabseq++);                      // Kommando lesen
    arg_anz= *tabseq++;                            // Anzahl zugehoeriger Datenbytes lesen
    ms= arg_anz & delay_flag;                      // bei gesetztem Flag folgt ein Timingbyte
    arg_anz &= ~delay_flag;                        // delay_flag aus Anzahl Argumenten loeschen
    while(arg_anz--)                               // jedes Datenbyte des Kommandos
    {
      lcd_write_data(*tabseq++);                   // senden
    }
    if(ms)                                         // wenn eine Timingangabe vorhanden ist
    {
      ms= *tabseq++;                               // Timingzeit lesen
      if(ms == 255) ms = 500;
      for (i= 0; i< ms; i++) delay(1);             // und entsprechend "nichts" tun
    }
  }

  lcd_orientation(orientation);
  normalcolor();
}

/* -------------------------------------------------------------
     putpixel

     setzt einen Bildpunkt auf das LCD

        x,y   : Koordinaten an der der Punkt gezeichnet
                werden soll
        color : 16 - Bit RGB565 Farbwert der gezeichnet
                werden soll
   ------------------------------------------------------------- */
void putpixel(int x, int y, uint16_t color)
{
  // Test, ob Pixel im Displaybereich liegt
  if ((x < 0) || (y < 0) || (x >= tftwidth) || (y >= tftheight)) return;

  address_set(x, y, tftwidth - 1, tftheight - 1);

  lcd_write_com(0x2C);
  lcd_write_data(color >> 8);
  lcd_write_data(color);

}

/* -------------------------------------------------------------
     putpixeltx

     Pixelausgabe benutzt von den putchar Funktionen, damit
     fuer outtextxy die Ausgaberichtung vorgegeben werden
     kann

        x,y   : Koordinaten an der der Punkt gezeichnet
                werden soll
        color : 16 - Bit RGB565 Farbwert der gezeichnet
                werden soll
   ------------------------------------------------------------- */
void putpixeltx(int x, int y, uint16_t color)
{
  if (txoutmode) putpixel(_xres-1-y,x,color); else putpixel(x,y,color);
}


/* -------------------------------------------------------------
     clrscr

     loescht das Display mit der in bkcolor angegebenen Farbe
   ------------------------------------------------------------- */
void clrscr(void)
{
  unsigned int i, m;

  address_set(0, 0, tftwidth-1, tftheight-1);

  for (i = 0; i < _yres; i++)
    for (m = 0; m < _xres; m++)
    {
      lcd_write_data(bkcolor >> 8);
      lcd_write_data(bkcolor);
    }
}

/* -------------------------------------------------------------
     line

     Zeichnet eine Linie von den Koordinaten x0,y0 zu x1,y1
     mit der angegebenen Farbe

        x0,y0 : Koordinate linke obere Ecke
        x1,y1 : Koordinate rechte untere Ecke
        color : 16 - Bit RGB565 Farbwert der gezeichnet
                werden soll
     Linienalgorithmus nach Bresenham (www.wikipedia.org)

   ------------------------------------------------------------- */
void line(int x0, int y0, int x1, int y1, uint16_t color)
{

  //    Linienalgorithmus nach Bresenham (www.wikipedia.org)

  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2;                                     /* error value e_xy */

  for(;;)
  {

    putpixel(x0,y0,color);
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; }                  /* e_xy+e_x > 0 */
    if (e2 < dx) { err += dx; y0 += sy; }                  /* e_xy+e_y < 0 */
  }
}

/* ----------------------------------------------------------
     fastxline

     zeichnet eine Linie in X-Achse mit den X Punkten
     x1 und x2 auf der Y-Achse y1

        x1, x2 : Start-, Endpunkt der Linie
        y1     : Y-Koordinate der Linie
        color  : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll
   ---------------------------------------------------------- */
void fastxline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t color)
{
  uint16_t x;

  if (x2< x1) { x= x1; x1= x2; x= x2= x; }

  address_set(x1, y1, tftwidth - 1, tftheight - 1);

  for (x= x1; x< (x2+1); x++)
  {
    lcd_write_data(color >> 8);
    lcd_write_data(color);
  }
}

/* -------------------------------------------------------------
     rectangle

     Zeichnet ein Rechteck von den Koordinaten x0,y0 zu x1,y1
     mit der angegebenen Farbe

        x0,y0  : Koordinate linke obere Ecke
        x1,y1  : Koordinate rechte untere Ecke
        color  : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll
   ------------------------------------------------------------- */
void rectangle(int x1, int y1, int x2, int y2, uint16_t color)
{
  line(x1,y1,x2,y1, color);
  line(x2,y1,x2,y2, color);
  line(x1,y2,x2,y2, color);
  line(x1,y1,x1,y2, color);
}


/* ----------------------------------------------------------
     fillcret

     zeichnet ein ausgefuelltes Rechteck mit den
     Koordinatenpaaren x1/y1 (linke obere Ecke) und
     x2/y2 (rechte untere Ecke)

        x0,y0 : Koordinate linke obere Ecke
        x1,y1 : Koordinate rechte untere Ecke
        color : Zeichenfarbe
   ---------------------------------------------------------- */
void fillrect(int x1, int y1, int x2, int y2, uint16_t color)
{
  uint16_t y;

    if (y1> y2)
    {
      y= y1;
      y1= y2;
      y2= y;
    }

    for (y= y1; y< y2+1; y++)
    {
      fastxline(x1,y,x2,color);
    }

}

/* -------------------------------------------------------------
     ellipse

     Zeichnet eine Ellipse mit Mittelpunt an der Koordinate
     xm,ym mit den Hoehen- Breitenverhaeltnis a:b
     mit der angegebenen Farbe

        xm,ym  : Koordinate des Mittelpunktes der Ellipse
        a,b    : Hoehen- Breitenverhaeltnis
        color  : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll

     Ellipsenalgorithmus nach Bresenham (www.wikipedia.org)
   ------------------------------------------------------------- */
void ellipse(int xm, int ym, int a, int b, uint16_t color )
{
  // Algorithmus nach Bresenham (www.wikipedia.org)

  int dx = 0, dy = b;                       // im I. Quadranten von links oben nach rechts unten
  long a2 = a*a, b2 = b*b;
  long err = b2-(2*b-1)*a2, e2;             // Fehler im 1. Schritt */

  do
  {
    putpixel(xm+dx, ym+dy,color);            // I.   Quadrant
    putpixel(xm-dx, ym+dy,color);            // II.  Quadrant
    putpixel(xm-dx, ym-dy,color);            // III. Quadrant
    putpixel(xm+dx, ym-dy,color);            // IV.  Quadrant

    e2 = 2*err;
    if (e2 <  (2*dx+1)*b2) { dx++; err += (2*dx+1)*b2; }
    if (e2 > -(2*dy-1)*a2) { dy--; err -= (2*dy-1)*a2; }
  } while (dy >= 0);

  while (dx++ < a)                        // fehlerhafter Abbruch bei flachen Ellipsen (b=1)
  {
    putpixel(xm+dx, ym,color);             // -> Spitze der Ellipse vollenden
    putpixel(xm-dx, ym,color);
  }
}

/* -------------------------------------------------------------
     fillellipse

     Zeichnet eine ausgefuellte Ellipse mit Mittelpunt an der
     Koordinate xm,ym mit den Hoehen- Breitenverhaeltnis a:b
     mit der angegebenen Farbe

        xm,ym  : Koordinate des Mittelpunktes der Ellipse
        a,b    : Hoehen- Breitenverhaeltnis
        color  : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll

   Ellipsenalgorithmus nach Bresenham (www.wikipedia.org)
   ------------------------------------------------------------- */
void fillellipse(int xm, int ym, int a, int b, uint16_t color )
{
  // Algorithmus nach Bresenham (www.wikipedia.org)

  int dx = 0, dy = b;                       // im I. Quadranten von links oben nach rechts unten
  long a2 = a*a, b2 = b*b;
  long err = b2-(2*b-1)*a2, e2;             // Fehler im 1. Schritt */

  do
  {
    fastxline(xm+dx, ym+dy,xm-dx, color);            // I. und II.   Quadrant
    fastxline(xm-dx, ym-dy,xm+dx, color);            // III. und IV. Quadrant

    e2 = 2*err;
    if (e2 <  (2*dx+1)*b2) { dx++; err += (2*dx+1)*b2; }
    if (e2 > -(2*dy-1)*a2) { dy--; err -= (2*dy-1)*a2; }
  } while (dy >= 0);

  while (dx++ < a)                        // fehlerhafter Abbruch bei flachen Ellipsen (b=1)
  {
    putpixel(xm+dx, ym,color);             // -> Spitze der Ellipse vollenden
    putpixel(xm-dx, ym,color);
  }
}

/* -------------------------------------------------------------
     circle

     Zeichnet einen Kreis mit Mittelpunt an der Koordinate xm,ym
     und dem Radius r mit der angegebenen Farbe

        x ,y   : Koordinate des Mittelpunktes der Ellipse
        r      : Radius des Kreises
        color  : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll
   ------------------------------------------------------------- */
void circle(int x, int y, int r, uint16_t color )
{
  ellipse(x,y,r,r,color);
}

/* -------------------------------------------------------------
     fillcircle

     Zeichnet einen ausgefuellten Kreis mit Mittelpunt an der
     Koordinate xm,ym und dem Radius r mit der angegebenen Farbe

        x,y    : Koordinate des Mittelpunktes der Ellipse
        r      : Radius des Kreises
        color  : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll
   ------------------------------------------------------------- */
void fillcircle(int x, int y, int r, uint16_t color )
{
  fillellipse(x,y,r,r,color);
}


/* ----------------------------------------------------------
     showimage

     Kopiert ein im Flash abgelegtes Bitmap in den Screens-
     peicher. Bitmap muss byteweise in Zeilen gespeichert
     vorliegen Hierbei entspricht 1 Byte 8 Pixel.
     Bsp.: eine Reihe mit 6 Bytes entsprechen 48 Pixel
           in X-Achse

     ox,oy        : linke obere Ecke an der das Bitmap
                    angezeigt wird
     image        : das anzuzeigende Bitmap
     resX         : Anzahl der Bytes in X-Achse
     fwert        : Farbwert mit der das Pixel gezeichnet wird

     Speicherorganisation des Bitmaps:

     Y       X-Koordinate
     |        0  1  2  3  4  5  6  7    8  9 10 11 12 13 14 15
     K               Byte 0                    Byte 1
     o  0     D7 D6 D5 D4 D3 D2 D1 D0   D7 D6 D5 D4 D3 D2 D1 D0
     o
     r         Byte (Y*XBytebreite)     Byte (Y*XBytebreite)+1
     d  1     D7 D6 D5 D4 D3 D2 D1 D0   D7 D6 D5 D4 D3 D2 D1 D0
     i
     n
     a
     t
     e
   ---------------------------------------------------------- */
void showimage(char ox, char oy, const unsigned char* const image, uint16_t fwert)
{
  int x,y;
  uint8_t b,bp;
  uint8_t resX, resY;

  resX= image[0];
  resY= image[1];
  if ((resX % 8) == 0) { resX= resX / 8; }
                 else  { resX= (resX / 8)+1; }

  for (y=0;y< resY;y++)
  {
    for (x= 0;x<resX;x++)
    {
      b= image[y *resX +x+2];
      for (bp=8;bp>0;bp--)
      {
        if (b & (1 << (bp-1))) {putpixel(ox+(x*8)+8-bp,oy+y,fwert);}
      }
    }
  }
}


/* --------------------------------------------------
     gotoxy

     Setzt den Textcursor (NICHT Pixelkoordinate) an
     die angegebene Textkoordinate.

        x : X-Koordinate
        y : Y-Koordinate
   -------------------------------------------------- */
void gotoxy(unsigned char x, unsigned char y)
{
  aktxp= x*(fontsizex+(textsize*fontsizex));
  aktyp= y*(fontsizey+(textsize*fontsizey));
}


/* --------------------------------------------------
     gotoxy_12x16

     Setzt den Textcursor (NICHT Pixelkoordinate)
     fuer einen 12x16 Font

        x : X-Koordinate
        y : Y-Koordinate
   -------------------------------------------------- */
void gotoxy_12x16(unsigned char x, unsigned char y)
{
  aktxp= x*(12+(textsize*12));
  aktyp= y*(16+(textsize*16));
}


/* --------------------------------------------------
     lcd_putchar8x8

     gibt das in ch angegebene Zeichen auf dem
     Display mit einem 8x8 grossen Font aus

     Parameter:
        ch :    auszugebendes Zeichen
   -------------------------------------------------- */
void lcd_putchar8x8(unsigned char ch)
{
  uint8_t   i,i2;
  uint8_t   b;
  int       oldx,oldy;
  uint16_t  fontint;
  uint16_t  fmask;

  if (ch== 13)                                          // Fuer <printf> "/r" Implementation
  {
    aktxp= 0;
    return;
  }
  if (ch== 10)                                          // fuer <printf> "/n" Implementation
  {
    aktyp= aktyp+fontsizey+(fontsizey*textsize);
    return;
  }

  fmask= 1<<(fontsizex-1);

  oldx= aktxp;
  oldy= aktyp;
  for (i=0; i<fontsizey; i++)
  {
    b= font8x8[(ch-32)][i];
    fontint= b;

    for (i2= 0; i2<fontsizex; i2++)
    {
      if (fmask & fontint)
      {
        putpixeltx(oldx,oldy,textcolor);
        if ((textsize==1))
        {
          putpixeltx(oldx+1,oldy,textcolor);
          putpixeltx(oldx,oldy+1,textcolor);
          putpixeltx(oldx+1,oldy+1,textcolor);
        }
      }
      else
      {
        if (fntfilled)
        {
          putpixeltx(oldx,oldy,bkcolor);
          if ((textsize==1))
          {
            putpixeltx(oldx+1,oldy,bkcolor);
            putpixeltx(oldx,oldy+1,bkcolor);
            putpixeltx(oldx+1,oldy+1,bkcolor);
          }
        }
      }
      fontint= fontint<<1;
      oldx= oldx+1+textsize;
    }
    oldy++;
    if ((textsize==1)) {oldy++; }
    oldx= aktxp;
  }
  aktxp= aktxp+fontsizex+(fontsizex*textsize);
}

/* --------------------------------------------------
   LCD_PUTCHAR8x8_NOBK
     gibt das in ch angegebene Zeichen auf dem
     Display mit einem 8x8 grossen Font aus.

     Der Hintergrund des Zeichens wird hier bei
     NICHT neu gezeichnet, so dass ein Zeichen
     ueber eine eventuelle Grafik gelegt werden
     kann.

     Parameter:
        ch :    auszugebendes Zeichen
   -------------------------------------------------- */
void lcd_putchar8x8_nobk(unsigned char ch)
{
  uint8_t   i,i2;
  uint8_t   b;
  int       oldx,oldy;
  uint16_t  fontint;
  uint16_t  fmask;

  if (ch== 13)                                          // Fuer <printf> "/r" Implementation
  {
    aktxp= 0;
    return;
  }
  if (ch== 10)                                          // fuer <printf> "/n" Implementation
  {
    aktyp= aktyp+fontsizey+(fontsizey*textsize);
    return;
  }

  fmask= 1<<(fontsizex-1);

  oldx= aktxp;
  oldy= aktyp;
  for (i=0; i<fontsizey; i++)
  {
    b= font8x8[(ch-32)][i];
    fontint= b;

    for (i2= 0; i2<fontsizex; i2++)
    {
      if (fmask & fontint)
      {
        putpixel(oldx,oldy,textcolor);
        if ((textsize==1))
        {
          putpixel(oldx+1,oldy,textcolor);
          putpixel(oldx,oldy+1,textcolor);
          putpixel(oldx+1,oldy+1,textcolor);
        }
      }
      fontint= fontint<<1;
      oldx= oldx+1+textsize;
    }
    oldy++;
    if ((textsize==1)) {oldy++; }
    oldx= aktxp;
  }
  aktxp= aktxp+fontsizex+(fontsizex*textsize);
}


/* --------------------------------------------------
     lcd_putchar12x16

     gibt das in ch angegebene Zeichen auf dem
     Display mit einem 12x16 grossen Font aus

     Parameter:
       ch       : auszugebendes Zeichen
   -------------------------------------------------- */
void lcd_putchar12x16(unsigned char ch)
{

  #if (fnt12x16_enable == 1)

    uint8_t   i,i2;
    uint16_t  b;
    int       oldx,oldy;
    uint16_t  fontint;
    uint16_t  fmask;
    uint16_t  findex;

    if (ch== 13)                                          // Fuer <printf> "/r" Implementation
    {
      aktxp= 0;
      return;
    }
    if (ch== 10)                                          // fuer <printf> "/n" Implementation
    {
      aktyp= aktyp+16+(16*textsize);
      return;
    }

    fmask= 1<<(16-1);
    oldx= aktxp;
    oldy= aktyp;
    for (i=0; i<16; i++)
    {
      findex= (ch-32);
      b= (font12x16[findex][i*2])<<4;
      b|= ((font12x16[findex][(i*2)+1])<<12);
      fontint= b;

      for (i2= 0; i2<12; i2++)
      {
        if (fmask & fontint)
        {
          putpixeltx(oldx,oldy,textcolor);
          if ((textsize==1))
          {
            putpixeltx(oldx+1,oldy,textcolor);
            putpixeltx(oldx,oldy+1,textcolor);
            putpixeltx(oldx+1,oldy+1,textcolor);
          }
        }
        else
        {
          if (fntfilled)
          {
            putpixeltx(oldx,oldy,bkcolor);
            if ((textsize==1))
            {
              putpixeltx(oldx+1,oldy,bkcolor);
              putpixeltx(oldx,oldy+1,bkcolor);
              putpixeltx(oldx+1,oldy+1,bkcolor);
            }
          }
        }
        fontint= fontint<<1;
        oldx= oldx+1+textsize;
      }
      oldy++;
      if ((textsize==1)) {oldy++; }
      oldx= aktxp;
    }
    if (textsize==1) aktxp= aktxp + 24; else aktxp = aktxp +12;

  #endif
}

/* --------------------------------------------------
   LCD_PUTCHAR12x16_NOBK
     gibt das in ch angegebene Zeichen auf dem
     Display mit einem 12x16 grossen Font aus.

     Der Hintergrund des Zeichens wird hier bei
     NICHT neu gezeichnet, so dass ein Zeichen
     ueber eine eventuelle Grafik gelegt werden
     kann.

     Parameter:
       ch       : auszugebendes Zeichen
   -------------------------------------------------- */
void lcd_putchar12x16_nobk(unsigned char ch)
{
  uint8_t   i,i2;
  uint16_t  b;
  int       oldx,oldy;
  uint16_t  fontint;
  uint16_t  fmask;
  uint16_t  findex;

  if (ch== 13)                                          // Fuer <printf> "/r" Implementation
  {
    aktxp= 0;
    return;
  }
  if (ch== 10)                                          // fuer <printf> "/n" Implementation
  {
    aktyp= aktyp+16+(16*textsize);
    return;
  }

  fmask= 1<<(16-1);
  oldx= aktxp;
  oldy= aktyp;
  for (i=0; i<16; i++)
  {
    findex= (ch-32);
    b= (font12x16[findex][i*2])<<4;
    b|= ((font12x16[findex][(i*2)+1])<<12);
    fontint= b;

    for (i2= 0; i2<12; i2++)
    {
      if (fmask & fontint)
      {
        putpixel(oldx,oldy,textcolor);
        if ((textsize==1))
        {
          putpixel(oldx+1,oldy,textcolor);
          putpixel(oldx,oldy+1,textcolor);
          putpixel(oldx+1,oldy+1,textcolor);
        }
      }
      fontint= fontint << 1;
      oldx= oldx+1+textsize;
    }
    oldy++;
    if ((textsize==1)) {oldy++; }
    oldx= aktxp;
  }
  if (textsize==1) aktxp= aktxp + 24; else aktxp = aktxp +12;
}


/* --------------------------------------------------
     outtextxy

     Ausgabe eines Textes an den GRAFIK-Koordinaten
     x,y. Der Text wird hierbei ueber den Hintergrund
     gelegt.

       x,y      :  Grafikkoordinaten, ab der der
                   Text ausgegeben wird (linke,
                   obere Ecke)
       dir      : Ausgaberichtung
                    0 = horizontal
                    1 = vertikal
       *dataPtr :  Zeiger auf den Text

     Beispiel in Verbindung mit Makro PSTR

     outtextxy(12,13,0,PSTR("Hallo Welt"));
   -------------------------------------------------- */
void outtextxy(int x, int y, uint8_t dir, char *dataPtr)
{
  uint8_t oldfill, oldx, oldy;

  unsigned char c;
  int tmp;

  if (dir)
  {
    txoutmode= 1;
    x= _xres-x-fontsizex;
    tmp= x; x= y; y= tmp;
  }

  oldfill= fntfilled;
  fntfilled= 0;
  oldx= aktxp; oldy= aktyp;

  aktxp= x; aktyp= y;
  for (c= *dataPtr; c; ++dataPtr, c= *dataPtr)
  {
    lcd_putchar(c);
  }
  fntfilled= oldfill; aktxp= oldx; aktyp= oldy;
  txoutmode= 0;
}


/* --------------------------------------------------
     setfont

     legt den Schriftstil fest, der bei einer
     Zeichenausgabe verwendet werden soll

        nr   : Schriftstil
                 0 : 8x8 Font
                 1 : 12x16 Font
   -------------------------------------------------- */
void setfont(uint8_t nr)
{
  switch(nr)
  {
    case 0:
    {
      fontsizex= 8;
      fontsizey= 8;
      fontnr= 0;
      break;
    }
    case 1:
    {
      fontsizex= 12;
      fontsizey= 16;
      fontnr= 1;
      break;
    }
    default:
    {
      break;
    }
  }
}

/* --------------------------------------------------
     lcd_putchar

     gibt ein Zeichen auf dem Display aus

     Parameter:
       ch   : auszugebendes Zeichen
   -------------------------------------------------- */
void lcd_putchar(char ch)
{
  switch (fontnr)
  {
    case 0:  lcd_putchar8x8(ch); break;
    case 1:  lcd_putchar12x16(ch); break;
    default: break;
  }
}

/* --------------------------------------------------------
      turtle_moveto

                        Turtlegrafik

      setzt den Ausgangspunkt fuer Turtlegrafiken auf
      die angegebenen Koordinaten

        x,y  : Koordinaten, an der das Zeichnen beginnt
   -------------------------------------------------------- */
void turtle_moveto(int x, int y)
{
 t_lastx= x; t_lasty= y;
}

/* --------------------------------------------------------
     turtle_lineto

                       Turtlegrafik

     zeichnet eine Linie von der letzten Position zur
     angegebenen x,y - Koordinate mit der Farbe col

         x,y  : Position bis zu der eine Linie gezogen
                wird
        col   : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll
   -------------------------------------------------------- */
void turtle_lineto(int x, int y, uint16_t col)
{
  line(x,y, t_lastx, t_lasty,col);
  turtle_moveto(x,y);
}


