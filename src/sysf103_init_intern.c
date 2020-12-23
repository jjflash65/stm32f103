/* -------------------------------------------------------------
                       sysf103_init_intern.c

   initialisiert MCU STM32F103 fuer einen "allgemeinen"
   Gebrauch.

   Das System wird auf 64MHz interner Takt eingestellt, die
   Funktion < sys_tick_handler > wird jede ms per Interrupt
   ausgefuehrt und in dieser wird eine Variable tick_ms
   hochgezaehlt.

   Der Takt fuer GPIO-Pins des Ports A,B und C wird einge-
   schaltet und im hier zugehoerigen Header sind mittels
   defines die GPIO - Pins als Input / Output konfigurierbar.

    Hardware  : STM32F103C8
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    18.02.2020   R. Seelig

   ------------------------------------------------------------- */

#include "sysf103_init_intern.h"

volatile int tick_ms = 0;

/* -----------------------------------------------------
     sys_tick_handler

     Interrupt, wird im Intervall aufgerufen, der in
     systick_setup konfiguriert wird
   ----------------------------------------------------- */
void sys_tick_handler(void)
{
  tick_ms++;
}

/* -----------------------------------------------------
     delay

     Verzoegerungsschleife. Grundverzoegerungszeit wird
     durch den Systemticker bestimmt.

        c :  Verzoegerungszeit = c * systickerintervall
   ----------------------------------------------------- */
void delay(int c)
{
  volatile int end_time = tick_ms + c;

  while (tick_ms < end_time)
  {
    __asm volatile("wfi");
  }
}

/* -----------------------------------------------------
     systick_setup

     konfiguriert den Systemticker
   ----------------------------------------------------- */
void systick_setup(void)
{
  systick_clear();
//  systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
  systick_set_reload(8000);                      // (FCPU / 8 / 1000) * uS
  systick_interrupt_enable();
  systick_counter_enable();
}

/* -----------------------------------------------------
     jtag_enable

     schaltet die Verfuegbarkeit fuer JTAG (ST-LINK v2)
     ein
   ----------------------------------------------------- */
void jtag_enable(void)
{
  // JTAG-Verfuegbarkeit einschalten
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_AFIO);
  gpio_primary_remap(0x00300200, AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF);
}

/* -----------------------------------------------------
     jtag_enable

     schaltet die Verfuegbarkeit fuer JTAG (ST-LINK v2)
     aus (Vorsicht, danach ist das Zielsystem nur ueber
     einen Bootloader oder mittels eines "Connect under
     reset" erreichbar)
   ----------------------------------------------------- */
void jtag_disable(void)
{
  // JTAG aus und die PINS als GPIO verfuegbar machen
  // !!! wird JTAG ausgeschaltet, so ist ein Upload nur noch mit einem Reset
  // waehrend des Verbindungsaufbaus moeglich

  rcc_periph_clock_enable(RCC_AFIO);
  gpio_primary_remap(0x00300200, AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF);
  rcc_periph_clock_enable(RCC_GPIOA);
}

/* -----------------------------------------------------
     gpio_clkon

     schaltet den Taktgenerator fuer die I/O Pins
     ein (PortA und PortB)
   ----------------------------------------------------- */
void gpio_clkon(void)
{
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);

}

/* -----------------------------------------------------
     sys_init

     konfiguriert das System fuer 64 MHz bei Verwendung
     des internen RC-Oszillators (kein Quarz not-
     wendig.
   ----------------------------------------------------- */
void sys_init(void)
{
  rcc_clock_setup_in_hsi_out_64mhz();
  systick_setup();
  gpio_clkon();
}
