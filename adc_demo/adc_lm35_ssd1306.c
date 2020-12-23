/* -----------------------------------------------------
                        adc_lm35_ssd1306.c

    Demo fuer ADC-Wandlung. Am analogen Pin (hier PA0)
    ist ein LM35 angeschlossen (analoger Sensor,
    produziert 10mV / Grad Celcius.

    Ausgabe erfolgt an einem I2C SSD1306 Display


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
#include "sysf103_init.h"
#include "my_printf.h"

#include "tftmono.h"

#define demo_speed     3000

#define printf         my_printf


/* --------------------------------------------------------
   my_putchar

   wird von my-printf / printf aufgerufen und hier muss
   eine Zeichenausgabefunktion angegeben sein, auf das
   printf einen Bytestream sendet !
   -------------------------------------------------------- */
void my_putchar(char ch)
{
  lcd_putchar(ch);
}


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

  uint8_t xpos= 2;
  uint8_t ypos= 4;


  sys_init();

  lcd_init();
  lcd_enable();

  adc_setup();
  adc_array[0]= 0;                                  // ADC1 , Input 0 waehlen
  adc_set_regular_sequence(ADC1, 1, adc_array);

  clrscr();
  gotoxy(0,0);
  printf("    STM32F103\n\r");
  printf("    ADC-Demo\n\r");
  printf("----------------");
  gotoxy(xpos, ypos);
  printf("ADC: ");
  while(1)
  {

    adc_start_conversion_direct(ADC1);
    while (!(adc_eoc(ADC1)));                       // warten bis Wandlung fertig

    mwert = adc_read_regular(ADC1);                 // ADC-Wert holen

    gotoxy(xpos+55, ypos);
    printf("        ");
    gotoxy(xpos+5, ypos);
    printf("%.2f V", (float) ((mwert*3.3) / 4096) );
    delay(300);
  }

}
