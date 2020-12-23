/* -------------------------------------------------------------
                          reversi_buttons.c

     Othello - Computerspiel

     Tastensteuerung fuer das Reversi-Spiel

     MCU      :  STM32F103
     Takt     :  Takt 72 MHz

     29.10.2019  R. Seelig
   ------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>

#include "sysf103_init.h"

#include "reversi_buttons.h"

/* -------------------------------------------------------------
                           globale Variable
   ------------------------------------------------------------- */
uint8_t spos;                    // speichert die Position des Auswahlquadrats beim Anwaehlen
                                 // eines Feldes

/* -------------------------------------------------------
                       button_init

     Initialisiert die GPIO-Anschluesse an die die Tasten
     angeschlossen sind als Eingaenge
   ------------------------------------------------------- */
void button_init(void)
{
  buttonright_init();
  buttonup_init();
  buttonleft_init();
  buttondwn_init();
}

/* -------------------------------------------------------
                        button_get

     wartet auf einen Tastendruck und gibt den Wert der
     Taste zurueck.

     Rueckgabe: Tastenwert  Funktion
                    1 ------ Taste oben
                    2 ------ Taste links
                    3 ------ Taste rechts
                    4 ------ Taste unten
                    5 ------ Doppelt gedrueckte Taste
                             oben (Enter)
   ------------------------------------------------------- */
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

/* -------------------------------------------------------
                      stone_gettoset

     Zeichnet ein aktive Auswahl einer Spieleposition,
     die mit den Tasten veraendert werden kann. Ein
     Doppeldruecken der Taste "oben" (Enter) gibt den
     Wert des Angewaehlten Spielfeldes zurueck.

     Benoetigt innerhalb des Programmcodes eine Funktion
     "stone_set" um den Rahmen zeichnen zu koennen

     Rueckgabe: ausgewaehlte Spieleposition
   ------------------------------------------------------- */
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
