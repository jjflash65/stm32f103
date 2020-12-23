/* -------------------------------------------------------------
                          reversi_ki.h

     Header fuer die KI von Reversi

     Ausgangsprojekt war ein Quellcode fuer Linux-Konsole
     mit (leider) unbekanntem Author.

     30.10.2019  R. Seelig
   ------------------------------------------------------------ */

#ifndef in_reversi_ki
  #define in_reversi_ki

  typedef int (* fpc) (int, int *);


  /* -------------------------------------------------------------
                             Prototypen
     ------------------------------------------------------------- */

  extern uint8_t stone_gettoset(void);                  // externe Funktion zur Eingabe des Spielerzuges
  extern void printboard (int *board);                  // externe Funktion zur Anzeige des Spielfeldes
  extern void status_meld(uint8_t nr);

  uint8_t playgame (int difficult);


#endif
