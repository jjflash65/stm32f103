/* -----------------------------------------------
                    serial_demo

     Demoprogramm fuer serielle Schnittstelle und
     einer "abgespeckten" printf-Version

    Hardware  : STM32F103
    IDE       : keine (Editor / make)
    Library   : libopencm3
    Toolchain : arm-none-eabi

     28.01.2020   R. Seelig

   ----------------------------------------------- */

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <libopencm3.h>
#include "sysf103_init.h"
#include "uart.h"
#include "my_printf.h"

#define printf    my_printf

#define BAUDRATE 19200

/* --------------------------------------------------------
   my_putchar

   wird von my-printf / printf aufgerufen und hier muss
   eine Zeichenausgabefunktion angegeben sein, auf das
   printf dann schreibt !
   -------------------------------------------------------- */
void my_putchar(char ch)
{
  uart_putchar(ch);
}


/* --------------------------------------------------------
                             main
   -------------------------------------------------------- */
int main(void)
{
  uint8_t count;

  sys_init();

  uart_init(BAUDRATE);

  printf("\n\r-------------------------------------\n\r");
  printf("\n\r STM32F103C8T6 / %dMHz - %dbd 8N1", rcc_ahb_frequency/1000000, BAUDRATE);
  printf("\n\r Februar 2020 R. Seelig \n\r");
  printf("\n\r APB = %d MHz", rcc_apb1_frequency/1000000);
  printf("\n\r AHB = %d MHz", rcc_ahb_frequency/1000000);
  printf("\n\r-------------------------------------\n\r");
  printf("\n\r    Ein einfacher Integer Zaehler\n\n\r");

  count= 0;
  while(1)
  {
    printf("    count= %d \r",count);
    count++;
    delay(1000);
  }
}
