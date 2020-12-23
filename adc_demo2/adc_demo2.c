/* -----------------------------------------------------
                        adc_demo2.c

    Demo fuer ADC-Wandlung. Am analogen Pin (hier PA0)
    ist ein LM35 angeschlossen (analoger Sensor,
    produziert 10mV / Grad Celcius.

    Ausgabe erfolgt ueber UART


    Hardware  : STM32F103
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    29.05.2017   R. Seelig
  ------------------------------------------------------ */


#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <libopencm3.h>
#include "smallio.h"


/* --------------------------------------------------------
                        adc_setup
   -------------------------------------------------------- */
void adc_setup(void)
{
  rcc_periph_clock_enable(RCC_ADC1);

  adc_power_off(ADC1);             // ADC waehrend Konfiguration aus

  // Konfiguration fuer einfache Wandlung
  adc_disable_scan_mode(ADC1);
  adc_set_single_conversion_mode(ADC1);
  adc_disable_external_trigger_regular(ADC1);
  adc_set_right_aligned(ADC1);

  adc_power_on(ADC1);
  delay(1);

  adc_reset_calibration(ADC1);
  adc_calibration(ADC1);

  delay(1);
}


/* -------------------------------------------------------------
                              M-A-I-N
   ------------------------------------------------------------- */
int main(void)
{

  uint8_t adc_array[16];
  uint16_t mwert;

  smallio_init();
  printfkomma= 3;


  adc_setup();
  adc_array[0]= 0;                                  // ADC1 , Input 0 waehlen
  adc_set_regular_sequence(ADC1, 1, adc_array);


  printf("\n\n\r----------------\n\r");
  printf("    STM32F103\n\r");
  printf("    ADC-Demo\n\r");
  printf("----------------\n\n\r");
  while(1)
  {

    adc_start_conversion_direct(ADC1);
    while (!(adc_eoc(ADC1)));                       // warten bis Wandlung fertig

    mwert = adc_read_regular(ADC1);                 // ADC-Wert holen

    printf("  ADC: %.2f V     \r", (float) ((mwert*3.3) / 4096) );
    delay(300);
  }

}
