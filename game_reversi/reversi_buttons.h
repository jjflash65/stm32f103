/* -------------------------------------------------------------
                          reversi_buttons.h

     Othello - Computerspiel

     Tastensteuerung fuer das Reversi-Spiel

     MCU      :  STM32F103
     Takt     :  Takt 72 MHz

     29.10.2019  R. Seelig
   ------------------------------------------------------------ */

#ifndef in_reversibuttons
  #define in_reversibuttons

// ----------------- Spieltasten ---------------

#define boardvers     3

#if (boardvers == 1)
  // Button-TFT-Shield                                                                 [MMCV5

  #define buttonleft_init()    PB1_input_init()
  #define is_buttonleft()      (!(is_PB1()))
  #define buttonright_init()   PB4_input_init()
  #define is_buttonright()     (!(is_PB4()))
  #define buttonup_init()      PB5_input_init()
  #define is_buttonup()        (!(is_PB5()))
  #define buttondwn_init()     PB3_input_init()
  #define is_buttondwn()       (!(is_PB3()))

#elif (boardvers == 2)
  // Multidisplayboard
  // auf dem Board sind Tasten an PB1, PB3, PB5, PB4 und PB13 vorhanden

  //  PB13 Taste ganz links
  //  PB1        nach rechts
  //  PB4        nach links
  //  PB3        nach oben
  //  PB5        nach unten

  #define buttonleft_init()    PB4_input_init()
  #define is_buttonleft()      (!(is_PB4()))
  #define buttonright_init()   PB1_input_init()
  #define is_buttonright()     (!(is_PB1()))
  #define buttonup_init()      PB3_input_init()
  #define is_buttonup()        (!(is_PB3()))
  #define buttondwn_init()     PB5_input_init()
  #define is_buttondwn()       (!(is_PB5()))

#elif (boardvers == 3)

  //  PB7         nach links
  //  PC15        nach rechts
  //  PC14        nach oben
  //  PA0         nach unten

  #define buttonleft_init()    PB7_input_init()
  #define is_buttonleft()      (!(is_PB7()))
  #define buttonright_init()   PC15_input_init()
  #define is_buttonright()     (!(is_PC15()))
  #define buttonup_init()      PC14_input_init()
  #define is_buttonup()        (!(is_PC14()))
  #define buttondwn_init()     PA0_input_init()
  #define is_buttondwn()       (!(is_PA0()))

#else
  #error "Keine Boardversion fuer Tasten angegeben"
#endif

  #define entpr_time         40           // Tastenentprellzeit in mS
  #define double_klicktime  100           // Zeit, die ein Doppelklick auf die Taste UP verstreichen
                                          // darf, um sie als Doppelklick zu erkennen

  enum { NO_KEY, UP, LEFT, RIGHT, DOWN, ENTER };

  /* -------------------------------------------------------------
                             globale Variable
     ------------------------------------------------------------- */
  extern uint8_t spos;                    // speichert die Position des Auswahlquadrats beim Anwaehlen
                                          // eines Feldes


  /* -------------------------------------------------------------
                             Prototypen
     ------------------------------------------------------------- */

  void button_init(void);
  uint8_t button_get(void);
  uint8_t stone_gettoset(void);

  extern void stone_set(uint8_t pos, uint8_t mode);


#endif
