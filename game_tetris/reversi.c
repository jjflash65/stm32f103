/* -------------------------------------------------------
                         reversi.c

                   YetAnotherOthelloGame

     Reversi/Othello Game fuer STM32F103 mit 160x128 TFT
     SPI Display

     MCU   :  STM32F103 / BluePill
     Takt  :  72 MHz

     13.02.2020  R. Seelig

     Hinweis:
     In pindefs.h sind die Anschluesse fuer das Display
     vorgegeben, gewaehlt ist hier durch tftdisplay.h
     die Definition 6:

        lcd_clk =>  PA5
        lcd_din =>  PA7
        lcd_ce  =>  PB6
        lcd_dc  =>  PA12
        lcd_rst =>  PA8

   ------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>

#include "sysf103_init.h"
#include "my_printf.h"

#include "tftdisplay.h"
#include "gfx_pictures.h"
#include "math_fixed.h"

#include "reversi_logo.h"
#include "reversi_logo_pal.h"

#define stone_len         14                            // Pixelgroesse eines Feldes
#define stone_rad         4                             // Radius eines Spielsteins
#define stone_xofs        2                             // x-Offset, ab derer das Spielfeld gezeichnet wird
#define stone_yofs        2                             // dto. y-Offset
#define stonefillcol      0x0540                        // Spielfeldhintergrundfarbe gruen
#define framcol_active    0xffea                        // Farbe fuer selektierten Spielstein gelb
#define framcol_inactive  0x0000                        // dto. unselektiert schwarz
#define whitestonecol     0xffff                        // Farbe des Spielsteins weiss
#define whitestonefrm     0xa594
#define blackstonecol     0x52aa                        // Farbe des Spielsteins schwarz
#define blackstonefrm     0x0000


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

#define entpr_time         40           // Tastenentprellzeit in mS
#define double_klicktime  100           // Zeit, die ein Doppelklick auf die Taste UP verstreichen
                                        // darf, um sie als Doppelklick zu erkennen

enum { NO_KEY, UP, LEFT, RIGHT, DOWN, ENTER };

#define printf    my_printf


/* -------------------------------------------------------------
                           Prototypen
   ------------------------------------------------------------- */
int human (void);
int maxdiffstrategy3(int, int *);
int maxweighteddiffstrategy1(int, int *);
int maxweighteddiffstrategy3(int, int *);
void status_meld(uint8_t nr);


/* -------------------------------------------------------------
          globale Variable und Konstante der original KI
   ------------------------------------------------------------- */

const int ALLDIRECTIONS[8]={-11, -10, -9, -1, 1, 9, 10, 11};
const int BOARDSIZE=100;

const int EMPTY = 0;
const int WHITE = 1;
const int BLACK = 2;
const int OUTER = 3;             // the value of a square on the perimeter

const int WIN   = 2000;          // a WIN and LOSS for player are outside
const int LOSS  = -2000;         // the range of any board evaluation function */

long int BOARDS;


void * STRATEGIES[5][4] =
{
  {"human", "human", human},
  {"diff3", "maxdiff, 3-move minmax lookahead", maxdiffstrategy3},
  {"wdiff1", "maxweigteddiff, 1-move minmax lookahead", maxweighteddiffstrategy1},
  {"wdiff3", "maxweigteddiff, 3-move minmax lookahead", maxweighteddiffstrategy3},
  {NULL, NULL, NULL}
};

typedef int (* fpc) (int, int *);

int     playerscore, cpuscore;          // Anzahl der Siege fuer Computer und Spieler
uint8_t spos;                           // speichert die Position des Auswahlquadrats beim Anwaehlen
                                        // eines Feldes


// --------------------------------------------------------------
//                   Funktionen fuer Tasten
// --------------------------------------------------------------

void button_init(void)
{
  buttonright_init();
  buttonup_init();
  buttonleft_init();
  buttondwn_init();
}

uint8_t button_get(void)
{
  uint8_t  result = 0;
  uint32_t tcount;
  uint8_t  doubleklick;

  while (!result)
  {
    if (is_buttonup())
    {
      delay(entpr_time);
      while(is_buttonup());
      delay(entpr_time);

      // erfassen, ob ein doppeltes Betaetigen des Tasters erfolgt ist
      tcount= 0;
      doubleklick= 0;
      while ((tcount< double_klicktime) && (!doubleklick))
      {
        if (is_buttonup()) doubleklick= 1;
        tcount++;
        delay(1);
      }

      if (doubleklick)
      {
        delay(entpr_time);
        while(is_buttonup());
        delay(entpr_time);
        result= 5;
      }
      else
      {
        result= 1;
      }
    }
    if (is_buttonleft())
    {
      delay(entpr_time);
      while(is_buttonleft());
      delay(entpr_time);
      result= 2;
    }
    if (is_buttonright())
    {
      delay(entpr_time);
      while(is_buttonright());
      delay(entpr_time);
      result= 3;
    }
    if (is_buttondwn())
    {
      delay(entpr_time);
      while(is_buttondwn());
      delay(entpr_time);
      result= 4;
    }
  }
  return result;
}

// --------------------------------------------------------------
//                 Funktionen fuer Displayausgaben
// --------------------------------------------------------------

void my_putchar(char ch)
{
  lcd_putchar(ch);
}


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

// Defines legen fest, innerhalb welcher Koordinaten eine Linie
// gezeichnet werden kann
#define lw_x1  -1
#define lw_x2  160
#define lw_y1  -1
#define lw_y2  128

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


void stone_set(uint8_t pos, uint8_t mode)
/*
    mode = 0 : schwarzer Stein wird mit inaktivem Rahmen gezeichnet
    mode = 1 : weisser Stein wird mit inaktivem Rahmen gezeichnet
    mode = 2 : leeres Feld

    mode = 3 : inaktiver Rahmen wird gezeichnet
    mode = 4 : aktiver Rahmen wird gezeichnet
*/
{
  int x, y;

  pos -= 11;

  x= ((pos % 10)*stone_len)+stone_xofs;
  y= ((pos / 10)*stone_len)+stone_yofs;

  switch (mode)
  {
    case 0 :
    {
      fillcircle(x+ (stone_len/2), y+ (stone_len/2), stone_rad, blackstonecol);
      circle(x+ (stone_len/2), y+ (stone_len/2), stone_rad, blackstonefrm);
      break;
    }
    case 1 :
    {
      fillcircle(x+ (stone_len/2), y+ (stone_len/2), stone_rad, whitestonecol);
      circle(x+ (stone_len/2), y+ (stone_len/2), stone_rad, whitestonefrm);
      break;
    }
    case 2 : fillrect(x,y, x+stone_len, y+stone_len, stonefillcol); break;
    case 3 : rectangle(x,y, x+stone_len, y+stone_len, framcol_inactive); break;
    case 4 : rectangle(x,y, x+stone_len, y+stone_len, framcol_active); break;
    default: break;
  }
}

void field_drawempty(void)
{
  int x,y;

  for (x= 1; x< 9; x++)
    for (y= 10; y< 81; y += 10)
    {
      stone_set(y+x, 2);
      stone_set(y+x, 3);
    }
}

// --------------------------------------------------------------
//      Funktionen fuer Bedienereingaben zur Spielesteuerung
// --------------------------------------------------------------

uint8_t stone_gettoset(void)
{
  uint8_t opos;
  uint8_t ch;

  stone_set(spos, 4);
  do
  {
    opos= spos;
    ch= button_get();
    switch (ch)
    {
      case UP :
      {
        if (spos > 20) spos-= 10; else spos= (spos % 10)+80;
        break;
      }
      case DOWN :
      {
        if (spos < 80) spos+= 10; else spos= (spos % 10)+10;
        break;
      }
      case LEFT :
      {
        if ((spos % 10) > 1) spos-= 1; else spos= ((spos / 10) * 10)+8;
        break;
      }
      case RIGHT :
      {
        if ((spos % 10) < 8) spos+= 1; else spos= ((spos / 10) * 10)+1;
        break;
      }
      default: break;
    }
    stone_set(opos, 3);
    stone_set(spos, 4);
  } while (ch != 5);
  return spos;
}

void textfeld(uint8_t xp, uint8_t yp, uint8_t txlen)
// xp und yp sind Textkoordinaten !
{
  uint16_t x, y;

  for (x= (xp*fontsizex)-2; x< ((xp*fontsizex)+((txlen*fontsizex)+2)); x++)
  {
    for (y= (yp*fontsizey)-2; y< ((yp*fontsizey)+ fontsizey+2); y++)
    {
      putpixel(x,y, rgbfromega(7));
    }
  }
  for (x= (xp*fontsizex)-2; x< ((xp*fontsizex)+((txlen*fontsizex)+2)); x++)
  {
    putpixel(x,(yp*fontsizey)-3, rgbfromega(0));
    putpixel(x+1,(yp*fontsizey)+fontsizey+2, rgbfromega(15));
  }
  for (y= (yp*fontsizey)-2; y< ((yp*fontsizey)+ fontsizey+2); y++)
  {
    putpixel((xp*fontsizex)-2, y, rgbfromega(0));
    putpixel(((xp*fontsizex)+((txlen*fontsizex)+2)), y, rgbfromega(15));
  }
}


/* ---------------------------------------------------------------------
                        Originale Funktionen der KI
   ---------------------------------------------------------------------

     the possible square values are integers (0-3), but we will
     actually want to print the board as a grid of symbols,
     . instead of 0, b instead of 1, w instead of 2, and ? instead
     of 3 (though we might only print out this latter case for
     purposes of debugging).
   --------------------------------------------------------------------- */

char nameof (int piece)
{
  static char piecenames[5] = ".OX?";

  return(piecenames[piece]);
}

/* --------------------------------------------------------------
   if the current player is WHITE (1), then the opponent is BLACK (2),
   and vice versa
   -------------------------------------------------------------- */
int opponent (int player)
{
  switch (player)
  {
    case 1  : return 2;
    case 2  : return 1;
    default : return 0;
  }
}

/* --------------------------------------------------------------
   The copyboard function mallocs space for a board, then copies
   the values of a given board to the newly malloced board.
   -------------------------------------------------------------- */
int *copyboard (int * board)
{
  int i, * newboard;

  newboard = (int *)malloc(BOARDSIZE * sizeof(int));
  for (i= 0; i< BOARDSIZE; i++) newboard[i] = board[i];
  return newboard;
}


/* --------------------------------------------------------------
   the initial board has values of 3 (OUTER) on the perimeter,
   and EMPTY (0) everywhere else, except the center locations
   which will have two WHITE (1) and two BLACK (2) values.
   -------------------------------------------------------------- */
int *initialboard (void)
{
  int i, * board;

  board = (int *)malloc(BOARDSIZE * sizeof(int));
  for (i = 0; i<= 9; i++) board[i]=OUTER;
  for (i = 10; i<= 89; i++)
  {
    if (i%10 >= 1 && i%10 <= 8) board[i]= EMPTY;
                           else board[i]= OUTER;
  }
  for (i = 90; i<= 99; i++) board[i]=OUTER;
  board[44]= BLACK; board[45]= WHITE; board[54]= WHITE; board[55]= BLACK;
  return board;
}


/* --------------------------------------------------------------
   count the number of squares occupied by a given player (1 or 2,
   or alternatively WHITE or BLACK)
   -------------------------------------------------------------- */
int count (int player, int * board)
{
  int i, cnt;

  cnt= 0;
  for (i= 1; i<= 88; i++)
    if (board[i] == player) cnt++;
  return cnt;
}


void printboard (int * board)
{
  int  row, col;
  char stone;

  for (row=1; row<=8; row++)
  {
    for (col=1; col<=8; col++)
    {
      stone= nameof(board[col + (10 * row)]);
      if (stone == 'O') stone_set( (row*10) + col, 1);
      if (stone == 'X') stone_set( (row*10) + col, 0);

      bkcolor= rgbfromega(7);
      textcolor= rgbfromega(15);
      gotoxy(17,3); printf("%d",count(WHITE, board));
      textcolor= rgbfromega(0);
      gotoxy(17,5); printf("%d",count(BLACK, board));
    }
  }
}


/* ---------------------------------------------------------------------
                   Routines for insuring legal play
   ---------------------------------------------------------------------
     The code that follows enforces the rules of legal play for Othello.
   --------------------------------------------------------------------- */

int validp (int move)
// a "valid" move must be a non-perimeter square
{
  if ((move >= 11) && (move <= 88) && (move%10 >= 1) && (move%10 <= 8))
     return 1;
  else return 0;
}


int findbracketingpiece(int square, int player, int * board, int dir)
{
  while (board[square] == opponent(player)) square = square + dir;
  if (board[square] == player) return square;
                          else return 0;
}

int wouldflip (int move, int player, int * board, int dir)
{
  int c;

  c = move + dir;
  if (board[c] == opponent(player))
     return findbracketingpiece(c+dir, player, board, dir);
  else return 0;
}

int legalp (int move, int player, int * board)
{
  int i;

  if (!validp(move)) return 0;
  if (board[move]==EMPTY)
  {
    i= 0;
    while (i<=7 && !wouldflip(move, player, board, ALLDIRECTIONS[i])) i++;
    if (i==8) return 0;
         else return 1;
  }
  else return 0;
}


void makeflips (int move, int player, int * board, int dir)
{
  int bracketer, c;

  bracketer = wouldflip(move, player, board, dir);
  if (bracketer)
  {
    c = move + dir;
    do
    {
      board[c] = player;
      c = c + dir;
    } while (c != bracketer);
  }
}

void makemove (int move, int player, int * board)
{
  int i;

  board[move] = player;
  for (i= 0; i<= 7; i++) makeflips(move, player, board, ALLDIRECTIONS[i]);
}


int anylegalmove (int player, int * board)
{
  int move;

  move = 11;
  while (move <= 88 && !legalp(move, player, board)) move++;
  if (move <= 88) return 1
           ; else return 0;
}

int nexttoplay (int * board, int previousplayer)
{
  int opp;
  opp = opponent(previousplayer);
  if (anylegalmove(opp, board)) return opp;
  if (anylegalmove(previousplayer, board))
  {
    status_meld(2);
    return previousplayer;
  }
  return 0;
}


int *legalmoves (int player, int * board)
{
  int move, i, * moves;

  moves = (int *)malloc(65 * sizeof(int));
  moves[0] = 0;
  i = 0;
  for (move=11; move<=88; move++)
    if (legalp(move, player, board))
    {
      i++;
      moves[i]=move;
    }
  moves[0]= i;
  return moves;
}

/* ---------------------------------------------------------------------
                        Strategies for playing
   --------------------------------------------------------------------- */

int human (void)
// if a human player, then get the next move from standard input
{
  int move;

  move= stone_gettoset();
  return move;
}


/* --------------------------------------------------------------
   diffeval and weighteddiffeval are alternate utility functions
   for evaluation the quality (from the perspective of player)
   of terminal boards in a minmax search.
   -------------------------------------------------------------- */
int diffeval (int player, int * board)
{                                        // utility is measured
  int i, ocnt, pcnt, opp;                // by the difference in
  pcnt=0; ocnt = 0;                      // number of pieces
  opp = opponent(player);
  for (i=1; i<=88; i++)
  {
    if (board[i]==player) pcnt++;
    if (board[i]==opp) ocnt++;
  }
  return (pcnt-ocnt);
}

int weighteddiffeval (int player, int * board)
{
  int i, ocnt, pcnt, opp;

  const int weights[100]={0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                          0,120,-20, 20,  5,  5, 20,-20,120,  0,
                          0,-20,-40, -5, -5, -5, -5,-40,-20,  0,
                          0, 20, -5, 15,  3,  3, 15, -5, 20,  0,
                          0,  5, -5,  3,  3,  3,  3, -5,  5,  0,
                          0,  5, -5,  3,  3,  3,  3, -5,  5,  0,
                          0, 20, -5, 15,  3,  3, 15, -5, 20,  0,
                          0,-20,-40, -5, -5, -5, -5,-40,-20,  0,
                          0,120,-20, 20,  5,  5, 20,-20,120,  0,
                          0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

  pcnt=0; ocnt=0;
  opp = opponent(player);
  for (i=1; i<=88; i++)
  {
    if (board[i]==player) pcnt=pcnt+weights[i];
    if (board[i]==opp) ocnt=ocnt+weights[i];
  }
  return (pcnt - ocnt);
}


int minmax (int player, int * board, int ply, int (* evalfn) (int, int *))
{
  int i, max, ntm, newscore, bestmove, * moves, * newboard;
  int maxchoice (int, int *, int, int (*) (int, int *));
  int minchoice (int, int *, int, int (*) (int, int *));

  moves = legalmoves(player, board);                     // get all legal moves for player
  max = LOSS - 1;                                        // any legal move will exceed this score
  for (i=1; i <= moves[0]; i++)
  {
    newboard = copyboard(board); BOARDS = BOARDS + 1;
    makemove(moves[i], player, newboard);
    ntm = nexttoplay(newboard, player);
    if (ntm == 0)
    {
      // game over, so determine winner
      newscore = diffeval(player, newboard);
      if (newscore > 0) newscore = WIN;                  // Spieler gewinnt
      if (newscore < 0) newscore = LOSS;                 // Computer gewinnt
    }
    if (ntm == player)                                   // Computer kann nicht ziehen
       newscore = maxchoice(player, newboard, ply-1, evalfn);
    if (ntm == opponent(player))
       newscore = minchoice(player, newboard, ply-1, evalfn);
    if (newscore > max)
    {
      max = newscore;
      bestmove = moves[i];                               // besseren Spielzug gefunden
    }
    free(newboard);
  }
  free(moves);
  return(bestmove);
}

int maxchoice (int player, int * board, int ply,
               int (* evalfn) (int, int *))
{

  int i, max, ntm, newscore, * moves, * newboard;
  int minchoice (int, int *, int, int (*) (int, int *));

  if (ply == 0) return((* evalfn) (player, board));
  moves = legalmoves(player, board);
  max = LOSS - 1;
  for (i=1; i <= moves[0]; i++)
  {
    newboard = copyboard(board); BOARDS = BOARDS + 1;
    makemove(moves[i], player, newboard);
    ntm = nexttoplay(newboard, player);
    if (ntm == 0)
    {
      newscore = diffeval(player, newboard);
      if (newscore > 0) newscore = WIN;
      if (newscore < 0) newscore = LOSS;
    }
    if (ntm == player)
       newscore = maxchoice(player, newboard, ply-1, evalfn);
    if (ntm == opponent(player))
       newscore = minchoice(player, newboard, ply-1, evalfn);
    if (newscore > max) max = newscore;
    free(newboard);
  }
  free(moves);
  return(max);
}

int minchoice (int player, int * board, int ply,
               int (* evalfn) (int, int *))
{
  int i, min, ntm, newscore, * moves, * newboard;

  if (ply == 0) return((* evalfn) (player, board));
  moves = legalmoves(opponent(player), board);
  min = WIN+1;
  for (i=1; i <= moves[0]; i++)
  {
    newboard = copyboard(board); BOARDS = BOARDS + 1;
    makemove(moves[i], opponent(player), newboard);
    ntm = nexttoplay(newboard, opponent(player));
    if (ntm == 0)
    {
      newscore = diffeval(player, newboard);
      if (newscore > 0) newscore = WIN;
      if (newscore < 0) newscore = LOSS;
    }
    if (ntm == player)
       newscore = maxchoice(player, newboard, ply-1, evalfn);
    if (ntm == opponent(player))
       newscore = minchoice(player, newboard, ply-1, evalfn);
    if (newscore < min) min = newscore;
    free(newboard);
  }
  free(moves);
  return(min);
}




/* --------------------------------------------------------------
   the following strategies use minmax search
   -------------------------------------------------------------- */

int maxdiffstrategy3(int player, int * board)
{
  return(minmax(player, board, 3, diffeval));
}

/* --------------------------------------------------------------
    use weigteddiffstrategy as utility function
   -------------------------------------------------------------- */
int maxweighteddiffstrategy1(int player, int * board)
{
   return(minmax(player, board, 1, weighteddiffeval));
}

int maxweighteddiffstrategy3(int player, int * board)
{
   return(minmax(player, board, 3, weighteddiffeval));
}


/* -----------------------------------------------------------------
                     Coordinating matches
   ----------------------------------------------------------------- */

/* --------------------------------------------------------------
   get the next move for player using strategy
   -------------------------------------------------------------- */
void getmove (int (* strategy) (int, int *), int player, int * board)
// Achtung, rekursive !
{
  int move;

  printboard(board);
  move = (* strategy)(player, board);
  if (legalp(move, player, board))
  {
    makemove(move, player, board);
  }
  else
  {
    status_meld(1);                             // unerlaubter Zug
    getmove(strategy, player, board);
  }
}


uint8_t othello (int (* blstrategy) (int, int *),
                 int (* whstrategy) (int, int *))
{
  int *board;
  int player;

  int wanz, banz;                                       // nimmt Anzahl der Steine auf, die zum
                                                        // Schluss auf dem Spielfeld liegen
  int result;

  board = initialboard();
  player = WHITE;                                       // hier ist angegeben, wer das Spiel beginnt
  do
  {
    if (player == WHITE) getmove(blstrategy, WHITE, board);
    else getmove(whstrategy, BLACK, board);
    player = nexttoplay(board, player);
  }
  while (player != 0);

  // das Spiel ist beendet
  printboard(board);
  wanz= count(WHITE, board);
  banz= count(BLACK, board);
  if (wanz == banz) result = 5;                         // unentschieden
  if (wanz > banz) result = 4;                          // Spieler gewinnt
  if (wanz < banz) result = 3;                          // CPU gewinnt

  return result;
}

uint8_t playgame (int difficult)
{
  uint8_t result;

  int p1, p2;
  int (* strfn1)(int, int *);
  int (* strfn2)(int, int *);

  p1= 0;
  p2= difficult;

  strfn1 = STRATEGIES[p1][2]; strfn2 = STRATEGIES[p2][2];
  result= othello(strfn1, strfn2);
  return result;
}

// --------------------------------------------------------------
//             Funktionen der Displaydarstellung
// --------------------------------------------------------------

/* -------------------------------------------------------
                        score_show

     zeigt den Spielstand zwischen Computer und Spieler
     an.
   ------------------------------------------------------- */
void score_show(uint16_t xofs, uint16_t yofs)
{
  char zstr[7];

  bkcolor= rgbfromvalue(0x00, 0x00, 0x50);
  clrscr();

  spiro_generate(80, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));
//  spiro_generate(240, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));

  fillellipse(xofs,yofs, 50,20, rgbfromvalue(0x40, 0x40, 0xff));
  ellipse(xofs,yofs, 50,20, rgbfromega(8));
  ellipse(xofs,yofs, 48,18, rgbfromega(15));


  setfont(1);
  textcolor= rgbfromega(0);
  outtextxy(xofs-30, yofs-8, 0,"Score");
  textcolor= rgbfromega(14);
  outtextxy(xofs-29, yofs-7, 0,"Score");
  setfont(0);

  textcolor= rgbfromega(14);
  outtextxy(xofs-39, yofs+43, 0, "Player:");
  itoa(playerscore, zstr, 10);
  outtextxy(xofs+37, yofs+43, 0, zstr);

  textcolor= rgbfromega(12);
  outtextxy(xofs-39, yofs+57, 0, "CPU   :");
  itoa(cpuscore, zstr, 10);
  outtextxy(xofs+37, yofs+57, 0, zstr);
}


void levellogo_show(uint16_t xofs, uint16_t yofs)
{
  bkcolor= rgbfromvalue(0x00, 0x00, 0x50);
  clrscr();

  spiro_generate(80, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));
//  spiro_generate(240, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));

  fillellipse(xofs,yofs, 50,20, rgbfromvalue(0xff, 0x7a, 0x2b));
  ellipse(xofs,yofs, 50,20, rgbfromega(8));
  ellipse(xofs,yofs, 48,18, rgbfromega(15));

  setfont(1);
  textcolor= rgbfromega(0);
  outtextxy(xofs-30, yofs-8, 0,"Level");
  textcolor= rgbfromega(14);
  outtextxy(xofs-29, yofs-7, 0,"Level");
  setfont(0);
}

void select_show(uint16_t xofs, uint16_t yofs, uint8_t select)
{
  uint16_t col_unsel, col_sel;

  col_unsel= rgbfromvalue(0xa0, 0xa0, 0xa0);
  col_sel= rgbfromvalue(0xff, 0xfb, 0x2a);
  setfont(1);

  if (select == 0) textcolor= col_sel; else textcolor= col_unsel;
  outtextxy(xofs, yofs, 0,"leicht");
  if (select == 1) textcolor= col_sel; else textcolor= col_unsel;
  outtextxy(xofs+2, yofs+18, 0,"mittel");
  if (select == 2) textcolor= col_sel; else textcolor= col_unsel;
  outtextxy(xofs+2, yofs+36, 0,"schwer");
}

uint8_t get_difficult(uint16_t xofs, uint16_t yofs)
{
  uint8_t ch, sel;

  levellogo_show(xofs, yofs);
  sel= 1;
  do
  {
    select_show(xofs-36, yofs+34, sel);
    ch= button_get();
    if (ch== 1)                         // Taste nach oben
    {
      if (sel> 0) sel--; else sel= 2;
    }
    if (ch== 4)                         // Taste nach unten
    {
      if (sel< 2) sel++; else sel= 0;
    }
  } while(ch != 5);
  return sel+1;
}

void status_meld(uint8_t nr)
{
  uint16_t oldbk;

  oldbk= bkcolor;
  bkcolor= rgbfromvalue(0x40, 0x40, 0x40);
  switch (nr)
  {
    case 0 :
    {
      gotoxy(0,15);
      printf("                    ");
      break;
    }
    case 1 :
    {
      gotoxy(1,15);
      textcolor= rgbfromega(14);
      printf("Ungueltiger Zug");
      gotoxy(1,15);
      delay(1000);
      printf("                  ");
      break;
    }
    case 2 :
    {
/*
      // irgendwie sieht das unschoen aus, wenn dem so ist, aus diesem
      // Grunde ist das weggelassen
      gotoxy(1,15);
      textcolor= rgbfromega(14);
      printf("kein Zug moeglich");
      gotoxy(1,15);
      delay(1000);
      printf("                  ");
*/
      break;
    }
    case 3 :
    {
      gotoxy(1,15);
      textcolor= rgbfromega(12);
      printf("   CPU gewinnt");
      gotoxy(1,15);
      delay(2500);
      printf("                  ");
      cpuscore++;
      break;
    }
    case 4 :
    {
      gotoxy(1,15);
      textcolor= rgbfromega(14);
      printf(" Spieler gewinnt");
      gotoxy(1,15);
      delay(2500);
      printf("                  ");
      playerscore++;
      break;
    }
    case 5:
    {
      gotoxy(1,15);
      textcolor= rgbfromega(14);
      printf("  unentschieden");
      gotoxy(1,15);
      delay(2500);
      printf("                  ");
      cpuscore++;
      playerscore++;
      break;
    }
    default : break;
  }
  bkcolor= oldbk;
}

/* ----------------------------------------------------------------------------------
                                       MAIN
   ---------------------------------------------------------------------------------- */
int main (void)
{
  uint8_t difficulty;
  uint8_t result;

  sys_init();
//  jtag_disable();
  lcd_init();
  lcd_orientation(2);
  textsize= 0;

  // Bildschirm zum Abfragen der Schwierigkeitsstufe und Auswahl
  // derselben
  bkcolor= rgbfromvalue(0x00, 0x00, 0x50);
  clrscr();
  pcx256_show(0,0, &reversi_logo[0], &logopal[0]);
    button_get();
    delay(100);

  difficulty= get_difficult(80, 30);
  playerscore= 0; cpuscore= 0;

  do
  {
    textcolor= rgbfromega(15);
    spiro_generate(60, 200, 70, 80, 140, 512, rgbfromvalue(0x0,0x0,0xff));

    // Spielebildschirm (die Schwierigkeitsstufe bleibt waehrend der Laufzeit
    // immer dieselbe (difficulty ist ausserhalb dieser Schleife)
    setfont(0);
    bkcolor= rgbfromvalue(0xa0, 0x70, 0x18);
  //  bkcolor= 0;
    clrscr();

    textcolor= rgbfromega(15);
    spiro_generate(80, 200, 70, 80, 140,512, rgbfromvalue(0xcc,0x80, 0x00));
    field_drawempty();

    outtextxy(118,7,0, "Score");
    textfeld(15,3,4);
    textfeld(15,5,4);
    status_meld(0);

    spos= 55;                           // Startposition des Auswahlquadrats
    // das eigentliche Spiel starten
    result= playgame(difficulty);
    status_meld(result);
    score_show(80,30);
    button_get();
    delay(500);
  } while (1);

}
