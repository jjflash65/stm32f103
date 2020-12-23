/* -----------------------------------------------------
                          blinky.c

    Ein erstes Blink-Programm fuer den STM32F103
    auf der "BluePill" oder einem Eigenbau-Board

    Hardware  : STM32F103
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    03.02.2020   R. Seelig
  ------------------------------------------------------ */

#include <libopencm3.h>
#include <math.h>

#include "sysf103_init.h"

#define led1_init()   PC13_output_init()        // Onboard LED BluePill an PC13
#define led1_set()    PC13_clr()
#define led1_clr()    PC13_set()

/*
#define led1_init()   PA5_output_init()         // Onboard LED (Selbstbau-PCB) PA5 LED
#define led1_set()    PA5_set()
#define led1_clr()    PA5_clr()
*/

#define blkspeed      250

int main(void)
{

  sys_init();

  led1_init();

  while(1)
  {
    led1_set();
    delay(blkspeed);
    led1_clr();
    delay(blkspeed);
  }
}
