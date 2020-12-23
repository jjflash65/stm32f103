/* -----------------------------------------------------
                         timer2_demo.c

    Demonstriert im Gegensatz zum Demo in der Stopuhr
    eine weitere Methode, einen Timer (hier Timer2)
    zu verwenden

    Hardware  : STM32F103
    Takt      : 72 MHz
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    03.02.2020   R. Seelig
  ------------------------------------------------------ */


#include "smallio.h"


volatile uint32_t   ms_cnt= 0;           // Millisekundenzaehler, wird in tim3_isr aufaddiert

/* -------------------------------------------------------
                          tim2_isr

     Interrupt Service Routine fuer Timer2

   ------------------------------------------------------- */
void tim2_isr(void)
{
  ms_cnt++;

  // Interrupt quittieren
  TIM_SR(TIM2) &= ~TIM_SR_UIF;
}


/* -------------------------------------------------------
                          tim2_init

     initialisiert Timer2 fuer Interruptaufruf jede
     Millisekunde
   ------------------------------------------------------- */
static void tim2_init(void)
{
  // Timer2 Initialisierung ruft jede Millisekunde < tim2_isr > auf


  rcc_periph_clock_enable(RCC_TIM2);

  nvic_enable_irq(NVIC_TIM2_IRQ);
  nvic_set_priority(NVIC_TIM2_IRQ, 1);

  TIM_CNT(TIM2) = 1;                    // Startwert des Timers
  TIM_PSC(TIM2) = 7200;                 // (72MHz / 1000
  TIM_ARR(TIM2) = 10;                   // bei erreichen von 10 wird ein Interrutp ausgeloest
  TIM_DIER(TIM2) |= TIM_DIER_UIE;
  TIM_CR1(TIM2) |= TIM_CR1_CEN;

}

/* -----------------------------------------------------
                             main
   ----------------------------------------------------- */

int main(void)
{
  uint16_t count;

  smallio_init();

  count = 0;
  tim2_init();

  printf("\n\r-------------------------------------\n\r");
  printf("             Timer2 Demo");
  printf("\n\r-------------------------------------\n\r");

  while(1)
  {
    if (ms_cnt >= 1000)
    {
      ms_cnt= 0;
      printf("  Sekunden:  %d \r", count);
      count++;
    }
    delay(100);
  }
}
