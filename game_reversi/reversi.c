/* -------------------------------------------------------------
                           reversi.c
                      YetAnotherOthelloGame

     Reveris/Othello - Computerspiel fuer STM32F103 mit
     160x128 TFT SPI-Display

     Ausgangsprojekt war ein Quellcode fuer Linux-Konsole
     mit (leider) unbekanntem Author.

     Port fuer einen STM32F103 Mikrocontroller mit hinzu-
     gefuegter Grafikausgabe auf Display

     Hardware :  GLCD 160x128; 4 Tasten

     MCU      :  STM32F103
     Takt     :  Takt 72 MHz

     14.02.2020  R. Seelig

     Hinweis:
     In pindefs.h sind die Anschluesse fuer das Display
     vorgegeben, gewaehlt ist hier durch tftdisplay.h
     die Definition 6:

        lcd_clk =>  PA5
        lcd_din =>  PA7
        lcd_ce  =>  PB6
        lcd_dc  =>  PA12
        lcd_rst =>  PA8

     In reversi_buttons.h sind die Anschluesse fuer die
     Spieletasten vorgegeben. Default ist Boardversion 3
     eingestellt:

        Button links  =>  PB7
        Button rechts =>  PC15
        Button hoch   =>  PC14
        Button runter =>  PA0

   ------------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>

// rudimentaere Includes
#include "sysf103_init.h"
#include "my_printf.h"
#include "tftdisplay.h"
#include "gfx_pictures.h"

#include "spiro.h"                      // Spirograph-Generator fuer Hintergrundmuster

#include "reversi_ki.h"
#include "reversi_buttons.h"

#include "reversi_logo.h"
#include "reversi_logo_pal.h"


int playerscore, cpuscore;


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


#define printf    my_printf


/* -------------------------------------------------------------
     wird von my_printf aufgerufen um ein Zeichen auf dem
     Ausgabegeraet darzustellen.
   ------------------------------------------------------------- */
void my_putchar(char ch)
{
  lcd_putchar(ch);
}

/* -------------------------------------------------------------
                             textfeld

     zeichnet ein Textfeld fuer spaetere Textausgaben

     Uebergabe:

        xp    : x-Textkoordinate an der Textfeld gezeichnet wird
        yp    : dto. y-Textkoordinate
        txlen : Anzahl der Zeichen, die das Textfeld aufnehmen
                kann
   ------------------------------------------------------------- */
void textfeld(uint8_t xp, uint8_t yp, uint8_t txlen)
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


/* --------------------------------------------------------------
                          stone_set

     setzt einen Spielstein oder einen Rahmen auf das Spiel-
     feld.

     Spielfeldpositionen:
     +---------------------------------------+
     | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 |
     +---------------------------------------+
     | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 |
     +---------------------------------------+
     | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 |
     +---------------------------------------+
     | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 |
     +---------------------------------------+
     | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 |
     +---------------------------------------+
     | 61 | 62 | 63 | 64 | 65 | 66 | 67 | 68 |
     +---------------------------------------+
     | 77 | 72 | 73 | 74 | 75 | 76 | 77 | 78 |
     +---------------------------------------+
     | 88 | 82 | 83 | 84 | 85 | 86 | 87 | 88 |
     +---------------------------------------+

     Uebergabe:
        pos   : ausgewaehlte Spielfeldposition

        mode = 0 : schwarzer Stein wird mit inaktivem
                   Rahmen gezeichnet
        mode = 1 : weisser Stein wird mit inaktivem
                   Rahmen gezeichnet
        mode = 2 : leeres Feld

        mode = 3 : inaktiver Rahmen wird gezeichnet
        mode = 4 : aktiver Rahmen wird gezeichnet
   -------------------------------------------------------------- */
void stone_set(uint8_t pos, uint8_t mode)
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

/* -------------------------------------------------------------
                         field_drawempty

                 zeichnet ein leeres Spielfeld
   ------------------------------------------------------------- */
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


/* -------------------------------------------------------------
                         printboard

     zeichnet das aktuelle Spielfeld, wird von der Reversi-KI
     in reversi_ki.c aufgerufen

     Uebergabe:

       *board   : Zeiger auf das zu zeichnende Board
   ------------------------------------------------------------- */
void printboard (int *board)
{
  static char piecenames[5] = ".OX?";

  int i;
  int  row, col;
  char stone;
  int  wscore, bscore;


  wscore= 0; bscore= 0;
  for (i= 1; i<= 88; i++)
  {
    if (board[i] == 1) wscore++;
    if (board[i] == 2) bscore++;
  }

  for (row=1; row<=8; row++)
  {
    for (col=1; col<=8; col++)
    {
      stone= piecenames[board[col + (10 * row)]];
      if (stone == 'O') stone_set( (row*10) + col, 1);
      if (stone == 'X') stone_set( (row*10) + col, 0);

      bkcolor= rgbfromega(7);
      textcolor= rgbfromega(15);

      gotoxy(17,3); printf("%d", wscore);
      if (wscore< 10) printf(" ");
      textcolor= rgbfromega(0);
      gotoxy(17,5); printf("%d", bscore);
      if (bscore< 10) printf(" ");
    }
  }
}

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


/* -------------------------------------------------------
                        levellogo_show

     zeigt einen Screen zur Schwierigkeitsauswahl an

     Uebergabe:

        xofs, yofs : Koordinaten, an der der Screen
                     gezeichnet wird
   ------------------------------------------------------- */
void levellogo_show(uint16_t xofs, uint16_t yofs)
{
  bkcolor= rgbfromvalue(0x00, 0x00, 0x50);
  clrscr();

  spiro_generate(80, 200, 70, 80, 140,512, rgbfromvalue(0x0,0x0, 0x90));

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


/* -------------------------------------------------------
                        select_show

     zeigt Schwierigkeitsstufen als Text an

     Uebergabe:
        xofs, yofs : Koordinaten, an der der Screen
                     gezeichnet wird
        select     : Schwierigkeitsstufe, die voreinge-
                     stellt wird.
   ------------------------------------------------------- */
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


/* -------------------------------------------------------
                       get_difficult

     waehlt auf dem Levellogo-Screen die Schwierigkeit aus

     Uebergabe:
        xofs, yofs : Koordinaten, an der der Screen
                     gezeichnet wird

     Rueckgabe: gewaehlte Schwierigkeit
                  0 = leicht
                  1 = mittel
                  2 = schwierig
------------------------------------------------------- */
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


/* -------------------------------------------------------
                        status_meld

     Statusmeldungen, die waehrend des Spiels auf der
     unteren Displayzeile ausgegeben werden.

     Statusmeld wird von der KI in reversi_ki.c aufgerufen

     Uebergabe:
         nr : Statusmeldung
   ------------------------------------------------------- */
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
