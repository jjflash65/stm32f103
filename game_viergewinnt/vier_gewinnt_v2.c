/* -------------------------------------------------------
                         vg_vers2.c

     Computerspiel

     Hardware :  GLCD 128x128 mit ILI9163 Chipsatz
     MCU      :  STM32F103
     Takt     :  Takt 72 MHz

     14.12.2016  R. Seelig
   ------------------------------------------------------ */

#include <stdlib.h>
#include <ctype.h>
#include <libopencm3.h>

#include "c4.h"

#include "sysf103_init.h"
#include "tftdisplay.h"
#include "gfx_pictures.h"
#include "math_fixed.h"

#include "vg_logo2.h"
#include "vg_logo2pal.h"

#define my_outtextxy(x,y,ptr)   outtextxy(x,y,0,ptr)

// ----------------- Spieltasten ---------------

#define boardvers     3

#if (boardvers == 1)
  // Button-TFT-Shield                                                                 [MMCV5

  #define buttonleft_init()    PB1_input_init()
  #define is_buttonleft()      (!(is_PB1()))
  #define buttonright_init()   PB4_input_init()
  #define is_buttonright()     (!(is_PB4()))
  #define buttonup_init()      PB5_input_init()
  #define is_buttonup()        (!(is_PB5()))
  #define buttondwn_init()     PB3_input_init()
  #define is_buttondwn()       (!(is_PB3()))

#elif (boardvers == 2)
  // Multidisplayboard
  // auf dem Board sind Tasten an PB1, PB3, PB5, PB4 und PB13 vorhanden

  //  PB13 Taste ganz links
  //  PB1        nach rechts
  //  PB4        nach links
  //  PB3        nach oben
  //  PB5        nach unten

  #define buttonleft_init()    PB4_input_init()
  #define is_buttonleft()      (!(is_PB4()))
  #define buttonright_init()   PB1_input_init()
  #define is_buttonright()     (!(is_PB1()))
  #define buttonup_init()      PB3_input_init()
  #define is_buttonup()        (!(is_PB3()))
  #define buttondwn_init()     PB5_input_init()
  #define is_buttondwn()       (!(is_PB5()))

#elif (boardvers == 3)

  //  PB7         nach links
  //  PC15        nach rechts
  //  PC14        nach oben
  //  PA0         nach unten

  #define buttonleft_init()    PB7_input_init()
  #define is_buttonleft()      (!(is_PB7()))
  #define buttonright_init()   PC15_input_init()
  #define is_buttonright()     (!(is_PC15()))
  #define buttonup_init()      PC14_input_init()
  #define is_buttonup()        (!(is_PC14()))
  #define buttondwn_init()     PA0_input_init()
  #define is_buttondwn()       (!(is_PA0()))

#else
  #error "Keine Boardversion fuer Tasten angegeben"
#endif

#define xofs   16
#define yofs   20
#define HUMAN           0
#define COMPUTER        1

uint8_t  playerscore = 0,
         cpuscore = 0;


void drawo(uint8_t kx, uint8_t ky)
{
  if (c4_is_winner(1))
  {
    textcolor= rgbfromvalue(0xff,0xa0,0x60);
  }
  else
    textcolor= rgbfromvalue(0x00,0x60,0x00);
  putcharxy((kx*17)+xofs+9, (ky*17)+yofs+8, 'o');
}

void drawcoin(int8_t kx, int8_t ky, uint8_t value)
{
  int yo;

  yo= yofs;
  if (ky== -1) yo -= 3;

  if (value== 0)
  {
    fillcircle( (kx*17)+xofs+12, (ky*17)+yo+12, 7, rgbfromega(14));
    circle( (kx*17)+xofs+12, (ky*17)+yo+12, 7, rgbfromvalue(0,0,0x80));
  }
  if (value== 1)
  {
    fillcircle( (kx*17)+xofs+12, (ky*17)+yo+12, 7, rgbfromvalue(0xff,0,0));
    circle( (kx*17)+xofs+12, (ky*17)+yo+12, 7, rgbfromvalue(0,0,0x80));
  }

  if (value== 3)
  {
    fillcircle( (kx*17)+xofs+12, (ky*17)+yo+12, 7, rgbfromvalue(0x00, 0x00, 0x50));
  }
}

void c4_show_wincoins(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
  uint8_t x,y, xa, ya, i;

  if (x1== x2) xa= 0;
  if (x1> x2)  xa= -1;
  if (x1< x2)  xa= 1;
  if (y1== y2) ya= 0;
  if (y1> y2)  ya= -1;
  if (y1< y2)  ya= 1;
  x= x1; y= y1;
  for (i= 0; i< 4; i++)
  {
    drawo(x,5-y);
    x += xa;
    y += ya;
  }
}

void drawleerfeld(uint16_t feldcol)
{
  int kx, ky;

  fillcircle(9+xofs, 9+yofs, 9,feldcol);
  fillcircle(9+xofs, 99+yofs, 9,feldcol);
  fillcircle(117+xofs, 9+yofs, 9,feldcol);
  fillcircle(117+xofs, 99+yofs, 9,feldcol);

  fillrect(9+xofs,yofs, 117+xofs, 108+yofs, feldcol);
  fillrect(xofs, 9+yofs, 9+xofs, 99+yofs, feldcol);
  fillrect(115+xofs, 9+yofs, 126+xofs, 99+yofs, feldcol);

  for (ky= 0; ky < 6; ky++)
  {
    for (kx= 0; kx < 7; kx++)
    {
      fillcircle( (kx*17)+xofs+12, (ky*17)+yofs+12, 7, 0);
      circle( (kx*17)+xofs+12, (ky*17)+yofs+12, 7, rgbfromvalue(0,0,0x80));
    }
  }
}

static void print_board(int width, int height)
{
  int x, y;
  char **board;

  board = c4_board();
  for (y=height-1; y>=0; y--)
  {
    for (x=0; x<width; x++)
    {
      if (!(board[x][y] == C4_NONE))
      {
        drawcoin(x,5-y,board[x][y]);
      }
    }
  }
}


static char selectpos = 3;

char get_player_col(void)
{
  int x;

//  fillrect(0,0, 127,yofs-1, rgbfromega(15));
  for (x= 0; x< 7; x++)
    drawcoin(x, -1, 3);

  drawcoin(selectpos, -1, 0);

  while(!(is_buttondwn()))
  {
    if (is_buttonleft())
    {
      if (selectpos> 0)
      {
        delay(50);
        while(is_buttonleft());
        delay(50);
        drawcoin(selectpos, -1, 3);
        selectpos--;
        drawcoin(selectpos, -1, 0);
      }
    }
    if (is_buttonright())
    {
      if (selectpos< 6)
      {
        delay(50);
        while(is_buttonright());
        delay(50);
        drawcoin(selectpos, -1, 3);
        selectpos++;
        drawcoin(selectpos, -1, 0);
      }
    }
  }
  drawcoin(selectpos, -1, 3);
  delay(200);
  return selectpos;
}

int lw_x1 = -1, lw_x2 = 160;
int lw_y1 = -1, lw_y2 = 128;

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

/* -------------------------------------------------------
                        score_show

     zeigt den Spielstand zwischen Computer und Spieler
     an.
   ------------------------------------------------------- */
void score_show(void)
{
  char zstr[7];

  bkcolor= rgbfromvalue(0x00, 0x00, 0x50);
  clrscr();

  spiro_generate(80, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));
//  spiro_generate(240, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));

  fillellipse(64+xofs,23, 50,20, rgbfromvalue(0x40, 0x40, 0xff));
  ellipse(64+xofs,23, 50,20, rgbfromega(8));
  ellipse(64+xofs,23, 48,18, rgbfromega(15));
  textsize= 1;
  textcolor= rgbfromvalue(0xff, 0x7a, 0x2b);
  textcolor= 0;
  outtextxy(27+xofs,17, 0, "Score");
  textsize= 0;

  textcolor= rgbfromega(14);
  outtextxy(20+xofs, 53, 0, "Player:");
  itoa(playerscore, zstr, 10);
  outtextxy(92+xofs, 53, 0, zstr);

  textcolor= rgbfromega(12);
  outtextxy(20+xofs, 65, 0, "CPU  :");
  itoa(cpuscore, zstr, 10);
  outtextxy(92+xofs, 65, 0, zstr);
}

/* -------------------------------------------------------
                        buttons_init
   ------------------------------------------------------- */
void buttons_init(void)
{
  buttonleft_init();
  buttonright_init();
  buttonup_init();
  buttondwn_init();
}
/* ---------------------------------------------------------------------
                                   MAIN
   --------------------------------------------------------------------- */

int main(void)
{
  uint16_t vgblue;
  int      player[2], level[2];
  int      turn;
  int      num_of_players, move;
  int      width, height;
  int      x1, y1, x2, y2;

  sys_init();

  buttons_init();
  lcd_init();
  lcd_enable();
  outmode= 2;
  bkcolor= rgbfromega(0);
  clrscr();

  pcx256_show(0,0, &pcx_vglogo[0], &pcx_vglogopal[0]);
  while( !(is_buttonup() ));
  delay(50);
  while( is_buttonup() );
  delay(50);

  score_show();
  while( !(is_buttonup() ));
  delay(50);
  while( is_buttonup() );
  delay(50);

  while(1)
  {
    textcolor= rgbfromega(1);

    vgblue= rgbfromvalue(0x40,0x20,0xff);
  //  vgblue= rgbfromvalue(0x0,0x10,0xaf);
    bkcolor= rgbfromvalue(0x00, 0x00, 0x50);
    clrscr();
    spiro_generate(80, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));
    spiro_generate(240, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));

    drawleerfeld(vgblue);

    width= 7; height= 6; num_of_players= 1;

    player[0] = HUMAN;
    player[1] = COMPUTER;

    turn= 0;                          // 0 = Spieler beginnt, 1 = Computer beginnt

    level[1]= 7;

    c4_new_game(width, height, 4);
    selectpos= 3;

    do {
        print_board(width, height);
        if (player[turn] == HUMAN)
        {
            do {
                  move = get_player_col();
            }
            while (!c4_make_move(turn, move, NULL));
        }
        else {
            c4_auto_move(turn, level[turn], &move, NULL);
        }

        turn = !turn;

    } while (!c4_is_winner(0) && !c4_is_winner(1) && !c4_is_tie());

    print_board(width, height);

    if (c4_is_winner(0)) {
        if (num_of_players == 1)
        {
          textcolor= rgbfromvalue(0xf0,0xf0,0x0);
          my_outtextxy(20,6,"Spieler gewinnt");
          playerscore++;
        }
        else
        {
          textcolor= rgbfromvalue(0xff,0x40,0x0);
          my_outtextxy(20,6,"Computer gewinnt");
          cpuscore++;
        }
        c4_win_coords(&x1, &y1, &x2, &y2);
        c4_show_wincoins(x1,y1,x2,y2);
    }
    else if (c4_is_winner(1)) {
        if (num_of_players == 1)
        {
          textcolor= rgbfromvalue(0xff,0x40,0x0);
          my_outtextxy(20,6,"Computer gewinnt");
          cpuscore++;
        }
        else
        {
          textcolor= rgbfromvalue(0xf0,0xf0,0x0);
          my_outtextxy(20,6,"Spieler gewinnt");
          playerscore++;
        }
        c4_win_coords(&x1, &y1, &x2, &y2);
        c4_show_wincoins(x1,y1,x2,y2);
    }
    else {
        textcolor= rgbfromvalue(0x80,0x80,0x80);
        my_outtextxy(20,6,"Unentschieden");
    }

    delay(3000);
    score_show();

    while( !(is_buttonup() ));
    delay(50);
    while( is_buttonup() );
    delay(50);

    c4_end_game();

  }
}
