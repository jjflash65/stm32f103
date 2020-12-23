/* -----------------------------------------------------
                        spi_f103.h

    Header Softwaremodul zum Umgang mit Interface SPI
    STM32F103 Controller

    Hardware  : STM32F103
    IDE       : keine (Editor / make)
    Library   : libopencm3
    Toolchain : arm-none-eabi

    06.12.2016   R. seelig
  ------------------------------------------------------ */

#ifndef in_spif103
  #define in_spif103

  #include <stdint.h>
  #include <libopencm3.h>
  #include "sysf103_init.h"

  //  PA4 = SS, PA5 = SCK, PA6 = MISO, PA7 = MOSI

  #define spi_ss     GPIO4
  #define spi_sck    GPIO5
  #define spi_miso   GPIO6
  #define spi_mosi   GPIO7

// ------------------ Prototypen -----------------------

  void spi_init(void);
  void spi_out(uint8_t data);
  uint8_t spi_in(void);

#endif

