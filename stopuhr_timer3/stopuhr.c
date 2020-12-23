/* -------------------------------------------------------
                         stopuhr.c

     mist die Zeit, die an Pin, hier die Taste - buts -
     ein Hi-Signal anliegt (als Demo fuer Timer3 mit
     Interrupt)

     Einstellungen des verwendeten Displays in tftdisplay.h
     und tft_pindefs.h

     MCU   :  STM32F103
     Takt  :  72 MHz

     14.02.2020  R. Seelig
   ------------------------------------------------------ */

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <libopencm3.h>

#include "sysf103_init.h"
#include "my_printf.h"
#include "tftdisplay.h"

// ----------------- Tastenbelegung ---------------

#define boardvers     3

#if (boardvers == 1)
  // Button-TFT-Shield

  #define butl_init()   PB1_input_init()
  #define is_butl()     (!(is_PB1()))
  #define butr_init()   PB4_input_init()
  #define is_butr()     (!(is_PB4()))
  #define buts_init()   PB5_input_init()
  #define is_buts()     (!(is_PB5()))

#elif (boardvers == 2)
  // auf dem Board sind Tasten an PB1, PB3, PB5, PB4 und PB13 vorhanden

  //  PB13 Taste ganz links
  //  PB1        nach rechts
  //  PB4        nach links
  //  PB3        nach oben
  //  PB5        nach unten

  #define butl_init()   PB4_input_init()
  #define is_butl()     (!(is_PB4()))
  #define butr_init()   PB1_input_init()
  #define is_butr()     (!(is_PB1()))
  #define buts_init()   PB3_input_init()
  #define is_buts()     (!(is_PB3()))
  #define butd_init()   PB5_input_init()
  #define is_butd()     (!(is_PB5()))

#elif (boardvers == 3)

  //  PC15        nach rechts
  //  PB7         nach links
  //  PC14        nach oben
  //  PA0         nach unten

  #define butl_init()   PB7_input_init()
  #define is_butl()     (!(is_PB7()))
  #define butr_init()   PC15_input_init()
  #define is_butr()     (!(is_PC15()))
  #define buts_init()   PC14_input_init()
  #define is_buts()     (!(is_PC14()))
  #define butd_init()   PA0_input_init()
  #define is_butd()     (!(is_PA0()))

#else
  #error "Keine Boardversion fuer Tasten angegeben"
#endif

#define button_init()  { butl_init(); butr_init(); buts_init(); }


#define printf       my_printf


volatile uint32_t   ms_cnt= 0;           // Millisekundenzaehler, wird in tim3_isr aufaddiert
volatile char       ms_newvalue= 0;
volatile char       ms_keystate= 0;


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


/* -------------------------------------------------------
                          tim3_isr

     Interrupt Service Routine fuer Timer3

     Stellt fest, ob der Anschluss fuer buts einen Level-
     wechsel von 0 nach 1 hatte, wenn dem so ist, setzt
     er in ms_keystate das Flag, dass die Stopuhr laueft
     und bei Wiedereintreten in diese ISR zaehlt er die
     (globale) Variable ms_cnt hoch (ueber die in der
     main gepollt wird).
   ------------------------------------------------------- */
void tim3_isr(void)
{
  if (!ms_newvalue)
  {

    // solange die Taste gedrueckt ist, die Millisekunden
    // zaehlen
    if ( (is_buts()) && ms_keystate)
    {
      ms_cnt++;
    }

    // Taste wurde zum Start gedrueckt, ms_keystate zeigt
    // an, dass der Start erfolgt ist
    if ( (is_buts()) && (!ms_keystate) )
    {
      ms_cnt= 0;
      ms_keystate= 1;
    }

    // wenn Taste losgelassen wurde, in ms_newvalue anzeigen,
    // dass ein neues Ergebnis vorliegt
    if ( (!is_buts()) && ms_keystate)
    {
      ms_newvalue= 1;
    }
  }

  // Interrupt quittieren
  TIM_SR(TIM3) &= ~TIM_SR_UIF;
}


/* -------------------------------------------------------
                          tim3_init

     initialisiert Timer3 fuer Interruptaufruf jede
     Millisekunde
   ------------------------------------------------------- */
static void tim3_init(void)
{
  // Timer3 Initialisierung ruft jede Millisekunde < tim3_isr > auf
  rcc_periph_clock_enable(RCC_TIM3);

  timer_reset(TIM3);
  timer_set_prescaler(TIM3, 7199);      // (72MHz / 10000) - 1
  timer_set_period(TIM3, 10);           // 10 KHz fuer 10 Ticks = 1 KHz = 1ms Overflow
  nvic_enable_irq(NVIC_TIM3_IRQ);
  timer_enable_update_event(TIM3);
  timer_enable_irq(TIM3, TIM_DIER_UIE);
  timer_enable_counter(TIM3);
}



/* -------------------------------------------------------
                                main
   ------------------------------------------------------- */
int main(void)
{

  sys_init();

  lcd_init();
  lcd_enable();
  button_init();
  tim3_init();

  lcd_orientation(2);
  setfont(0);
  delay(300);

  bkcolor= rgbfromega(white);
  clrscr();                                  // mit weissem Hintergrund loeschen

  // gruenen Balken am oberen Displayrand mit Schritf "Stopuhr"
  fillrect(0,0,159,20,rgbfromega(green));
  textcolor= rgbfromega(yellow);
  bkcolor= rgbfromega(green);
  gotoxy(6,1); printf("Stopuhr");

  // grundsaetzliche Anzeige
  textcolor= rgbfromega(blue);
  bkcolor= 0xffff;
  gotoxy(6,4); printf("Time [s]: " );
  printfkomma= 1;
  setfont(1);

  //Zeitausgabe
  gotoxy(3,3); printf("%k   ", ms_cnt / 10);

  // waehrend die Stopzeit laueft, Anzeige der laufenden Zeit
  // in roter Schrift
  textcolor= rgbfromega(lightred);
  while(1)
  {
    // abgelaufene Zeit alle 0,1 S anzeigen
    if ((ms_keystate) && (ms_cnt % 10)== 0)
    {
      gotoxy(3,3); printf("%k   ", ms_cnt / 100);
    }

    // nach loslassen der Taste Zeit mit 1/1000 Sekunde
    // anzeigen
    if (ms_newvalue)
    {
      printfkomma= 2;

      // Anzeige der angehaltenen Stopuhr in blauer Schrift
      textcolor= rgbfromega(blue);
      gotoxy(3,3); printf("%k   ", ms_cnt / 10);

      textcolor= rgbfromega(lightred);
      printfkomma= 1;
      delay(30);
      ms_keystate= 0;
      ms_cnt= 0;
      ms_newvalue= 0;
    }
  }
}

