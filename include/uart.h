/* -------------------------------------------------------
                         uart.h

     Library fuer rudimentaere RS232-Funktionen

     MCU   :  STM32303
     Takt  :  interner Takt

     USART2 / com_port == 2

     PA2 :  TxD
     PA3 :  RxD

     USART1 / com_port == 1

     PA9 :  TxD
     PA10:  RxD

     02.01.2018  R. Seelig

   ------------------------------------------------------ */

#ifndef in_serial
  #define in_serial

  #include <stdint.h>
  #include <libopencm3.h>

  // Anmerkung: Auf dem Nucleo-Board ist USART2 auf dem STLINK-V2 aufgelegt

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
