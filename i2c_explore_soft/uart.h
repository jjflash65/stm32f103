/* -------------------------------------------------------
                         uart.h

     Library fuer rudimentaere RS232-Funktionen

     MCU   :  STM32F103C8
     Takt  :  interner Takt

     28.09.2016  R. Seelig

     Anmerkung:

     PA9 : TxD
     PA10: RxD
   ------------------------------------------------------ */

#ifndef in_serial
  #define in_serial

  #include <stdint.h>
  #include <libopencm3.h>

  #define com_port            1                    // 1 = Anschluesse PA9 / PA10  (USART1)
                                                   // 2 = Anschluesse PA2 / PA3   (USART2)

  #if (com_port == 1)
    #define COMPORT           USART1
  #endif

  #if (com_port == 2)
    #define COMPORT           USART2
  #endif


  void uart_init(int baud);
  void uart_putchar(uint8_t ch);
  uint8_t uart_getchar(void);
  uint8_t uart_ischar(void);

#endif
