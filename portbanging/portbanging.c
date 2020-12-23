/* ------------------------------------------------
                   portbanging.c

    Programm zum Ueberpruefen der I/O Ports A und B
    des STM32F103.

    Steuerung ueber serielle Schnittstelle

    Hardware  : STM32F103
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    03.02.2020   R. Seelig
   ----------------------------------------------- */

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include <libopencm3.h>

#include "sysf103_init.h"
#include "uart.h"
#include "my_printf.h"


uint8_t   aktport;
int       akt_pa, akt_pb;               // Pufferspeicher der Portpins
int       aktvalue;

#define printf    my_printf

/* ------------------------------------------------
                      getaktvalue

     liest den Pufferspeicher des gewaehlten Ports
   ------------------------------------------------ */
uint16_t getaktvalue(uint8_t selport)
{
  uint16_t b;

  switch (selport)
  {
    case 'a' : b= akt_pa; break;
    case 'b' : b= akt_pb; break;
    default : break;
  }
  return b;
}

/* --------------------------------------------------------
   my_putchar

   wird von my-printf / printf aufgerufen und hier muss
   eine Zeichenausgabefunktion angegeben sein, auf das
   printf einen Bytestream sendet !
   -------------------------------------------------------- */
void my_putchar(char c)
{
  uart_putchar(c);
}


/* --------------------------------------------------------
                        showstatus

     zeigt den aktuellen Status der Portpins an
   -------------------------------------------------------- */
void showstatus(void)
{
  printf("aktuell gewaehlter Port: P%c\n\r", aktport-32);
  printf("Ausgaenge P%c           : 0x%x\n\r", aktport-32, aktvalue );
}

/* --------------------------------------------------------
   -------------------------------------------------------- */
int main(void)
{
  char      ch;
  uint8_t   sanz;

  sys_init();
  uart_init(19200);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 0x10FF);

  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 0xFFE3);


  printf("\n\n\rSTM32F103C8T6 115200 Bd R.Seelig\n\n\r");
  printf("p = Portwahl  0..9,a..f = Bit-Togglen gewaehlter Port\n\n\r");

  aktport= 'a';
  akt_pa= 0xffff; akt_pb= 0xffff;
  aktvalue= getaktvalue(aktport);

  gpio_set(GPIOA, 0xffff);
  gpio_set(GPIOB, 0xffff);

  showstatus();

  while (1)
  {
    ch= uart_getchar();
    if (ch== 'p')
    {
      printf("\n\rPortnummer [a..b]: ");
      aktport= uart_getchar();
      printf("%c\n\r",aktport);
      aktvalue= getaktvalue(aktport);
      showstatus();
    }
    if ( ((ch>= '0') && (ch<= '9')) || ((ch>= 'a') && (ch<= 'f')) )
    {
      if (ch <= '9')  { sanz= ch-'0'; } else { sanz= ch-'a'+10; }

      aktvalue = aktvalue ^ (1 << (sanz));            // Bits togglen
      showstatus();
      switch (aktport)
      {
        case 'a' : {

                     gpio_clear(GPIOA, 0xffff);
                     gpio_set(GPIOA, aktvalue);

                     akt_pa= aktvalue;
                     break;
                   }
        case 'b' : {
                     gpio_clear(GPIOB, 0xffff);
                     gpio_set(GPIOB, aktvalue);

                     akt_pb= aktvalue;
                     break;
                   }
        default  : break;
      }
    }
  }
}

