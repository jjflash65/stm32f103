/* -----------------------------------------------------
                        spi_f103.c

    Softwaremodul zum Umgang mit Interface SPI
    STM32F030F4P6 Controller

    Hardware  : STM32F103
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    07.10.2016   R. seelig
  ------------------------------------------------------ */

#include "spi_f103.h"

/* -------------------------------------------------------------
   SPI_INIT

      initialisiert SPI-Schnittstelle (vorzugsweise zur
      Benutzung mit SPI LC-Displays)
   ------------------------------------------------------------- */
void spi_init(void)
{
//  rcc_periph_clock_enable(RCC_AFIO);
  rcc_periph_clock_enable(RCC_SPI1);

  delay(100);

  // Pins der SPI Schnittstelle konfigurieren
  // GPIOs: SS=PA4, SCK=PA5, MISO=PA6 and MOSI=PA7

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO4 | GPIO5 | GPIO7 );

  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO6);

  // Reset SPI, SPI_CR1 register cleared, SPI disabled
  spi_reset(SPI1);

  spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_2, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                  SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

  // NSS software management
  spi_enable_software_slave_management(SPI1);
  spi_set_nss_high(SPI1);

  spi_enable(SPI1);

}

/* -------------------------------------------------------------
   SPI_OUT

      Byte ueber SPI senden
      data ==> zu sendendes Datum
   ------------------------------------------------------------- */
void spi_out(uint8_t data)
{
  spi_send(SPI1, (uint8_t) data);
}

/* -------------------------------------------------------------
   SPI_IN

      Byte ueber SPI einlesen
   ------------------------------------------------------------- */
uint8_t spi_in(void)
{
  return (spi_read(SPI1) );
}
