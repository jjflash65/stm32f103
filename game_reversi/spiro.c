/* -------------------------------------------------------------
                           spiro.c

     Spirograph-Generator

     zeichnet ein Spirograph auf dem Display

     29.10.2019  R. Seelig
   ------------------------------------------------------------ */

#include "spiro.h"

/* -------------------------------------------------------------
     line_in_window

     Zeichnet eine Linie von den Koordinaten x0,y0 zu x1,y1
     mit der angegebenen Farbe

        x0,y0 : Koordinate linke obere Ecke
        x1,y1 : Koordinate rechte untere Ecke
        color : 16 - Bit RGB565 Farbwert der gezeichnet
                werden soll
     Linienalgorithmus nach Bresenham (www.wikipedia.org)

   ------------------------------------------------------------- */
void line_in_window(int x0, int y0, int x1, int y1, uint16_t color)
{

  //    Linienalgorithmus nach Bresenham (www.wikipedia.org)

  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2;                                     /* error value e_xy */

  for(;;)
  {
    if ((x0 > lw_x1) && (x0 < lw_x2) && (y0 > lw_y1) && (y0 < lw_y2))
      putpixel(x0,y0,color);
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; }                  /* e_xy+e_x > 0 */
    if (e2 < dx) { err += dx; y0 += sy; }                  /* e_xy+e_y < 0 */
  }
}

/* --------------------------------------------------------
                       turtle_draw

                       Turtlegrafik

     setzt den (virtuellen) Turtlecursor oder
     zeichnet eine Linie von der letzten Position zur
     angegebenen x,y - Koordinate mit der Farbe col

         x,y  : Position bis zu der eine Linie gezogen
                wird
        col   : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll
        mode  : 1 => zeichnen
                0 => neue Koordinaten setzen
   -------------------------------------------------------- */
void turtle_draw(int x, int y, uint16_t col, uint8_t mode)
{
  static int t_lastx = 0;
  static int t_lasty = 0;

  if (mode)
  {
    line_in_window(x,y, t_lastx, t_lasty,col);
  }
  t_lastx= x; t_lasty= y;
}

/* --------------------------------------------------------
                      spiro_generate

     zeichnet ein "Spirograph"

     inner:  innerer Radius
     outer:  aeusserer Radius
     evol :  Anzahl der "Schwingungen" fuer eine Umdrehung
     resol:  Aufloesung (Schrittweite) beim Zeichnen
             des Graphen
     col  :  Farbe, mit der der Graph gezeichnet wird
   -------------------------------------------------------- */
void spiro_generate(int c_width, int c_height, int inner, int outer, int evol, int resol, uint16_t col)
{
  float     inner_xpos, inner_ypos;
  float     outer_xpos, outer_ypos;
  float     j, k;
  int       i;

  inner_xpos = (c_width / 2.0f);
  inner_ypos = (c_height / 2.0f) + inner;

  outer_xpos= inner_xpos;
  outer_ypos= inner_ypos + outer;
  turtle_draw(outer_xpos, outer_ypos, 0, 0);                 // Ausgangsposition setzen

  for (i= 0; i< resol + 1; i++)
  {
    j= ((float)i / resol) * (2.0f * MY_PI);
    inner_xpos = (c_width / 2.0f) + (inner * fk_sin(j));
    inner_ypos = (c_height / 2.0f) + (inner * fk_cos(j));

    k= j * ((float)evol / 10.0f);

    outer_xpos= inner_xpos + (outer * fk_sin(k));
    outer_ypos= inner_ypos + (outer * fk_cos(k));

    turtle_draw(outer_xpos, outer_ypos, col, 1);
  }
}
