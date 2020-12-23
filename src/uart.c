/* -------------------------------------------------------
                         uart.c

     Library fuer rudimentaere RS232-Funktionen

     MCU   :  STM32F303
     Takt  :  interner Takt

     28.09.2016  R. Seelig

     Anmerkung:

     USART1
        PA9 : TxD
        PA10: RxD

     USART2
        PA2:  TxD
        PA3:  RxD
   ------------------------------------------------------ */


#include "uart.h"

/* -------------------------------------------------------
                      UART_INIT

  initialisiert serielle Schnittstelle mit anzugebender
  Baudrate. Protokoll 1 Startbit, 8 Databit, 1 Stopbit
  keine Paritaet (8N1)
  
  Initialisierung ist abhaengig von com_port Eins-
  stellung in uart.h

  USART1 / com_port = 1
  PA9:  TxD
  PA10: RxD

      oder

  USART2 / com_port = 2
  PA2:  TxD
  PA3:  RxD
 ------------------------------------------------------- */
void uart_init(int baud)
{
  rcc_periph_clock_enable(RCC_USART1);

  #if (com_port == 1)
    rcc_periph_clock_enable(RCC_USART1);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO9);
  #endif
  #if (com_port == 2)
  rcc_periph_clock_enable(RCC_USART2);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO2);
  #endif

  usart_set_baudrate(COMPORT, baud);
  usart_set_databits(COMPORT, 8);
  usart_set_stopbits(COMPORT, USART_STOPBITS_1);
  usart_set_mode(COMPORT, USART_MODE_TX_RX);
  usart_set_parity(COMPORT, USART_PARITY_NONE);
  usart_set_flow_control(COMPORT, USART_FLOWCONTROL_NONE);

  usart_enable(COMPORT);

}

/* -------------------------------------------------------
                      UART_PUTCHAR

     sendet ein Zeichen auf der seriellen Schnittstelle
   ------------------------------------------------------- */
void uart_putchar(uint8_t ch)
{
  usart_send_blocking(COMPORT, ch);
}

/* -------------------------------------------------------
                      UART_GETCHAR

     wartet solange, bis ein Zeichen auf der seriellen
     Schnittstelle eintrifft, liest dieses ein und gibt
     das Zeichen als Return-Wert zurueck
   ------------------------------------------------------- */
uint8_t uart_getchar(void)
{
  return usart_recv_blocking(COMPORT);
}

/* -------------------------------------------------------
                      UART_ISCHAR

     testet, ob ein Zeichen auf der seriellen Schnitt-
     stelle eingetroffen ist, liest aber ein eventuell
     vorhandenes Zeichen NICHT ein
   ------------------------------------------------------- */
uint8_t uart_ischar(void)
{
  return (USART_SR(COMPORT) & USART_SR_RXNE);
}
