/* -------------------------------------------------------
                         pwm_test.c

     Demo fuer die PWM des STM32F103

     Ausgangspin der PWM ist PA6

     MCU   :  STM32F103 / BluePill
     Takt  :  72 MHz

     27.05.2019  R. Seelig
   ------------------------------------------------------ */

#include <libopencm3.h>
#include <math.h>

#include "sysf103_init.h"

/* -------------------------------------------------------
                          pwm3_init

     verwendet Timer3 als PWM und laueft mit 72MHz (bei
     72 MHz Core-Takt)

     Die PWM steht an PA6 an.

     Uebergabe:
        t_periode  : Gesamtzyklen der PWM (max. 65535)
        t_pulse    : Zyklen der Pulsdauer (muss kleiner
                     als t_periode sein)
   ------------------------------------------------------- */
static void pwm3_init(uint32_t t_periode, uint32_t t_pulse)
{

  // TIM3 Takt einschalten
  rcc_periph_clock_enable(RCC_TIM3);

  // Takteinstellungen
  // Modus Edge: Direction - Bit bestimmt, ob up / down Counter
  // Direction-Bit nicht gesetzt -> up - counter
  TIM3_CR1 = TIM_CR1_CKD_CK_INT_MUL_4 | TIM_CR1_CMS_EDGE;

  // Zaehlerstand, bei der der Timer zurueck gesetzt wird
  // 0x4000 = 10 Bit Aufloesung
  // 0x8000 = 12 Bit Aufloesung
  TIM3_ARR = t_periode;

  // keinen Taktteiler (Prescaler)
  TIM3_PSC = 0;

  // Updategenerierung: keine Aktion
  TIM3_EGR = TIM_EGR_UG;

  // Output compare 1 mode
  // Preload
  TIM3_CCMR1 |= TIM_CCMR1_OC1M_PWM2 | TIM_CCMR1_OC1PE;

  // OC1 auf Ausgang (CC1E)
  // aktiv LOW
  TIM3_CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;

  // Capture compare (Vergleichswert) bei dem der Ausgang
  // auf LOW schaltet
  TIM3_CCR1 = t_pulse;

  // ARR reload enable
  TIM3_CR1 |= TIM_CR1_ARPE;

  // Timer3  enable
  TIM3_CR1 |= TIM_CR1_CEN;

  // PA6 = alternate Function Timer3 CH1
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO_TIM3_CH1);
}

#define t_periode    4096                       // 12 Bit-Aufloesung
#define u_hi_pegel   3300                       // ein Hi-Pegel des Pins hat 3300 mV

/* -------------------------------------------------------
                         spg_set

     setzt ein Puls/Pausen-Verhaeltnis an PA6 das der
     angegebenen Spannung entspricht.

     Wird dieses Rechtecksignal ueber einen Tiefpass ge-
     filtert, wird sich die gewaehlte Analogspannung
     einstellen.

     Uebergabe:
       voltage : zu setzende Spannung in mV
   ------------------------------------------------------- */
void spg_set(uint16_t voltage)
{
  float f;

  f= ( (float)t_periode / (float)u_hi_pegel) * (float)voltage;
  TIM3_CCR1 = (uint16_t)f;

}

/* -------------------------------------------------------
                          main
   ------------------------------------------------------- */
int main(void)
{

  sys_init();
  pwm3_init(t_periode, t_periode/2);      // pulse:pause = 1:1

  spg_set(1000);

  while(1);

}
