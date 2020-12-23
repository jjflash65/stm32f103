/* -------------------------------------------------------------
                           spiro.h

     Spirograph-Generator

     zeichnet ein Spirograph auf dem Display

     29.10.2019  R. Seelig
   ------------------------------------------------------------ */

#ifndef in_spirograph
  #define in_spirograph

#include <stdlib.h>
#include <stdio.h>

#include "sysf103_init.h"
#include "tftdisplay.h"
#include "math_fixed.h"



// Defines legen fest, innerhalb welcher Koordinaten eine Linie
// gezeichnet werden kann. Hier fuer ein 160x128 Pixel Display

#define lw_x1  -1
#define lw_x2  160
#define lw_y1  -1
#define lw_y2  128


void spiro_generate(int c_width, int c_height, int inner, int outer, int evol, int resol, uint16_t col);


#endif
