/* -----------------------------------------------------
                        smallio_printf_demo.c

    Demonstriert die printf-Faehigkeiten von smallio.

    Dies hier ist die Demo fuer "Anfaenger" die ein
    erstes Erfolgserlebnis suchen. Hier muss sich noch
    nicht sehr mit der Hardware auseinandergesetzt
    werden (noch nicht).

    Eine einzige Angabe von

                  #include "smallio.h"

    und ein

                  smallio_init();

    in der Main-Funktion initialiseren ein einfaches
    System, bei dem ein Einrichten von Portpins mittels
    Bitbanging extrem einfach ist. Zudem wird die
    serielle Schnittstelle initialisiert und ein einfache
    printf-Funktion zur Verfuegung gestellt.

    SMALLIO ist auch fuer STM8, MCS51 und NXP-
    Controller verfuegbar

    Hardware  : STM32F103
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    03.02.2020   R. Seelig
  ------------------------------------------------------ */


#include "smallio.h"

#define led_init()     ( PA5_output_init() )
#define led_on()       ( PA5_clr() )            // ... gegen +Vcc geschaltet
#define led_off()      ( PA5_set() )

#define tast_init()    ( PA13_input_init() )
#define is_tast()      ( is_PA13() )



int main(void)
{
  uint16_t       counter = 3121;
  int            zahl;
  uint16_t       ch;
  volatile float wuwert;

  smallio_init();

  led_init();
  tast_init();

  printfkomma= 3;

  wuwert= 166.666 / 1.297;

  printf("\n\r-------------------------------------\n\r");
  printf("\n\r STM32F103 / %d MHz 19200bd 8N1",  rcc_ahb_frequency/1000000);
  printf("\n\r Juni 2019 R. Seelig \n\r");
  printf("\n\r APB = %d MHz",  rcc_apb1_frequency/1000000);
  printf("\n\r AHB = %d MHz",  rcc_ahb_frequency/1000000);
  printf("\n\r-------------------------------------\n\r");
  printf("\n\rEin Integer        : %d",82137988);
  printf("\n\rEin 16-Bit Integer : %d",counter);
  printf("\n\rEine Festkommazahl : %k",82137988);
  printf("\n\rEin 32-Bit Hexwert : %x",0xca4b3a4c);
  printf("\n\rGleitkommazahl     : %.3f",wuwert);

  printf("\n\n\r Zahleneingabe: ");
  zahl= readint();
  printf("\n\r die Zahl war      : %d", zahl);

  printf("\n\rTaste fuer Counterstop... \n\n\r");

  while(keypressed()) {ch= getchar(); putchar(ch); }  // eventuell eingegangene Zeichen alle loeschen

  while (1)
  {
    led_off();
    delay(500);
    led_on();

    printf("  Counter: %xh  Taster= ", counter);

    if (is_tast()) printf("1     \r");
            else printf("0     \r");

    delay(500);
    counter++;
    counter = counter % 3600;

    if (keypressed())
    {
      ch= getchar();
      printf("\n\n\rGedrueckte Taste war: %c\n\r", ch );
      printf("Beliebige Taste fuer Counterstart...\n\n\r");
      ch= getchar();

    }
  }
}
