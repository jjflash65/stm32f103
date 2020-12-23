/* -----------------------------------------------------
                        i2c_devices_soft.h

    Header zum Softwaremodul fuer I2C Bus mittels 
    Software (Bitbanging) und einigen I2C-Devices.

    Implementierte I2C-Devices:

         - DS1307  (real time clock)
         - RDA5807 (UKW receiver)
         - LM75    (Temperatursensor)
         - 24LCxx  (EEProm)
         - SSD1306 (OLED-Display)

    Hardware  : STM32F103
    IDE       : make - Projekt
    Library   : libopencm3
    Toolchain : arm-none-eabi

    03.03.2020   R. Seelig
  ------------------------------------------------------ */

#ifndef in_i2c_devices
  #define in_i2c_devices

  #include <stdint.h>
  #include <string.h>

  #include <libopencm3.h>
  #include "sysf103_init.h"

  // Pinanschluss SDA
  #define sda_pin        GPIO11
  #define i2c_sda_hi()   ( gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, sda_pin) )
  #define i2c_sda_lo()   ( gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, sda_pin) )
  #define i2c_is_sda()   ( gpio_get(GPIOB, sda_pin))

  // Pinanschluss SCL
  #define scl_pin        GPIO10
  #define i2c_scl_hi()   ( gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, scl_pin) )
  #define i2c_scl_lo()   ( gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, scl_pin) )
  #define i2c_is_scl()   ( gpio_get(GPIOB, scl_pin))


  /* --------------------------------------------------------------------------
             Prototypen und Deklarationen fuer I2C Bus und I2C-Devices
     --------------------------------------------------------------------------- */

  // I2C Bus Controll
  // --------------------------------------------------------------------------

  #define i2c_delay(nr)    { __asm volatile ("nop"); }
  void i2c_master_init(void);
  void i2c_sendstart(void);
  uint8_t i2c_start(uint8_t addr);
  void i2c_stop();
  void i2c_startaddr(uint8_t addr, uint8_t rwflag);
  void i2c_write_nack(uint8_t data);
  uint8_t i2c_write(uint8_t data);
  uint8_t i2c_write16(uint16_t data);
  uint8_t i2c_read(uint8_t ack);

  #define i2c_read_ack()    i2c_read(1)
  #define i2c_read_nack()   i2c_read(0)


  // EEProm
  // --------------------------------------------------------------------------

  #define eep_addr            0xa0

  void eep_write(uint16_t adr, uint8_t value);
  void eep_erase(void);
  void eep_writebuf(uint16_t adr, uint8_t *buf, uint16_t len);
  uint8_t eep_read(uint16_t adr);
  void eep_readbuf(uint16_t adr, uint8_t *buf, uint16_t len);


  // RTC (real time clock) DS1307
  // --------------------------------------------------------------------------

  #define rtc_addr            0xd0

  struct my_datum
  {
    uint8_t jahr;
    uint8_t monat;
    uint8_t tag;
    uint8_t dow;
    uint8_t std;
    uint8_t min;
    uint8_t sek;
  };

  uint8_t rtc_read(uint8_t addr);
  void rtc_write(uint8_t addr, uint8_t value);
  uint8_t rtc_getwtag(struct my_datum *date);
  uint8_t rtc_bcd2dez(uint8_t value);
  struct my_datum rtc_readdate(void);
  void rtc_writedate(struct my_datum *date);


  // Temperatursensor LM75
  // --------------------------------------------------------------------------

  #define lm75_addr    0x90

  int lm75_read(void);

  // UKW-Radio RDA5807
  // --------------------------------------------------------------------------

  #define fbandmin     870        // 87.0  MHz unteres Frequenzende
  #define fbandmax     1080       // 108.0 MHz oberes Frequenzende
  #define sigschwelle  87         // Schwelle ab der ein Sender als "gut empfangen" gilt


  extern uint16_t aktfreq;
  extern uint8_t  aktvol;

  extern const uint16_t festfreq[6];

  extern uint8_t  rda5807_adrs;
  extern uint8_t  rda5807_adrr;
  extern uint8_t  rda5807_adrt;

  extern uint16_t rda5807_regdef[10];
  extern uint16_t rda5807_reg[16];

  void rda5807_writereg(uint8_t reg);
  void rda5807_write(void);
  void rda5807_reset(void);
  void rda5807_poweron(void);
  int rda5807_setfreq(uint16_t channel);
  void rda5807_setvol(int setvol);
  void rda5807_setmono(void);
  void rda5807_setstereo(void);

/* -----------------------------------------------------------------
     Ende RDA5807
   ----------------------------------------------------------------- */

  // EEProm 24LCxx
  // --------------------------------------------------------------------------

  #define eep_addr              0xa0

  #define eep_pagesize          0x20            // Anzahl Bytes, die vom
                                                // EEProm als Block geschrieben
                                                // oder gelesen werden koennen

  void eep_write(uint16_t adr, uint8_t value);
  void eep_erase(void);
  void eep_writebuf(uint16_t adr, uint8_t *buf, uint16_t len);
  uint8_t eep_read(uint16_t adr);
  void eep_readbuf(uint16_t adr, uint8_t *buf, uint16_t len);

  // SSD1306 OLED - Display
  // --------------------------------------------------------------------------

  #define ssd1306_addr          0x78

  extern uint8_t aktxp;
  extern uint8_t aktyp;
  extern uint8_t doublechar;
  extern uint8_t bkcolor;
  extern uint8_t textcolor;

  void ssd1306_writecmd(uint8_t cmd);
  void ssd1306_writedata(uint8_t data);
  void ssd1306_init(void);
  void gotoxy(uint8_t x, uint8_t y);
  void clrscr(void);
  void oled_putchar(uint8_t ch);

  // PCF8574 I/O - Expander
  // --------------------------------------------------------------------------

  #define pcf8574_addr          0x40

  void pcf8574_write(uint8_t value);
  uint8_t pcf8574_read(void);


#endif
