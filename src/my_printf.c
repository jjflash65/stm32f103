/* -------------------------------------------------------
                      my_printf.c

     Library-Header fuer eine eigene (sehr kleine) printf-
     Funktion.

     Compiler : arm-none-eabi-gcc

     fuer | myprintf | muss im Hauptprogramm irgendwo
     ein my_putchar vorhanden sein.

     Bsp.:

        void my_putchar(char ch)
        {
          txlcd_putchar(ch);
        }

     21.09.2016  R. Seelig
   ------------------------------------------------------ */

#include "my_printf.h"

char printfkomma = 1;
char noklzero    = 0;

extern void my_putchar(char ch);


/* ------------------------------------------------------------
                            PUTINT
     gibt einen Integer dezimal aus. Ist Uebergabe
     "komma" != 0 wird ein "Kommapunkt" mit ausgegeben.
     Groesste darstellare Zahl ist 99.999.999

     Bsp.: 12345 wird als 123.45 ausgegeben.
     (ermoeglicht Pseudofloatausgaben im Bereich)
   ------------------------------------------------------------ */
void putint(int i, char komma)
{
  typedef enum boolean { FALSE, TRUE }bool_t;

  static int zz[]  = { 10000000, 1000000, 100000, 10000, 1000, 100, 10 };
  bool_t not_first = FALSE;

  uint8_t zi;

  komma= 8-komma;

  if (!i)
  {
    my_putchar('0');
  }
  else
  {
    if(i < 0)
    {
      my_putchar('-');
      i = -i;
    }

    int z, b;

    for(zi = 0; zi < 7; zi++)
    {
      z = 0;
      b = 0;

      while(z + zz[zi] <= i)
      {
        b++;
        z += zz[zi];
      }

      if(b || not_first)
      {
        my_putchar('0' + b);
        not_first = TRUE;
      }

      if (zi+1 == komma)
      {
        if (!not_first)
          if (!(noklzero)) my_putchar('0');
        my_putchar('.');
        not_first= TRUE;
      }

      i -= z;
    }
    my_putchar('0' + i);
  }
}

/* ------------------------------------------------------------
                       HEXNIBBLEOUT
     gibt die unteren 4 Bits eines chars als Hexaziffer aus.
     Eine Pruefung ob die oberen vier Bits geloescht sind
     erfolgt NICHT !
  -------------------------------------------------------------  */
void hexnibbleout(uint8_t b)
{
  if (b< 10) b+= '0'; else b+= 55;
  my_putchar(b);
}

/* ------------------------------------------------------------
                            PUTHEX
     gibt einen Integer hexadezimal aus. Ist die auszugebende
     Zahl >= 0xff erfolgt die Ausgabe 2-stellig, ist sie
     groesser erfolgt die Ausgabe 4-stellig.

     Ist out16 gesetzt, erfolgt ausgabe immer 4 stellig
   ------------------------------------------------------------ */
void puthex(uint16_t h, char out16)
{
  uint8_t b;

  if ((h> 0xff) || out16)                    // 16 Bit-Wert
  {
    b= (h >> 12);
    hexnibbleout(b);
    b= (h >> 8) & 0x0f;
    hexnibbleout(b);
  }
  b= h;
  b= (h >> 4) & 0x0f;
  hexnibbleout(b);
  b= h & 0x0f;
  hexnibbleout(b);
}


void putstring(char *p)
{
  while(*p)
  {
    my_putchar( *p++ );
  }
}


/* ------------------------------------------------------------
                             MY_PRINTF
     alternativer Ersatz fuer printf.

     Aufruf:

         my_printf("Ergebnis= %d",zahl);

     Platzhalterfunktionen:

        %s     : Ausgabe Textstring
        %d     : dezimale Ausgabe
        %x     : hexadezimale Ausgabe
                 ist Wert > 0xff erfolgt 4-stellige
                 Ausgabe
                 is Wert <= 0xff erfolgt 2-stellige
                 Ausgabe
        %k     : Integerausgabe als Pseudokommazahl
                 12345 wird als 123.45 ausgegeben
        %c     : Ausgabe als Asciizeichen

   ------------------------------------------------------------ */

void my_printf(const char *s,...)
{
  int       arg1;
  uint32_t  xarg1;
  char     *arg2;
  char      ch;
  va_list   ap;

  va_start(ap,s);
  do
  {
    ch= *s;
    if(ch== 0) return;

    if(ch=='%')            // Platzhalterzeichen
    {
      s++;
      uint8_t token= *s;
      switch(token)
      {
        case 'd':          // dezimale Ausgabe
        {
          arg1= va_arg(ap,int);
          putint(arg1,0);
          break;
        }
        case 'x':          // hexadezimale Ausgabe
        {
          xarg1= va_arg(ap,uint32_t);
          if (xarg1 <= 0xFFFF)
          {
            puthex(xarg1, 0);
          }
          else
          {
            puthex(xarg1 >> 16,0);
            puthex(xarg1 & 0xffff,1);
          }

          break;
        }
        case 'k':
        {
          arg1= va_arg(ap,int);
          putint(arg1,printfkomma);     // Integerausgabe mit Komma: 12896 zeigt 12.896 an
          break;
        }
        case 'c':          // Zeichenausgabe
        {
          arg1= va_arg(ap,int);
          my_putchar(arg1);
          break;
        }
	case '%':
        {
          my_putchar(token);
          break;
        }
        case 's':
        {
          arg2= va_arg(ap,char *);
          putstring(arg2);
          break;
        }
      }
    }
    else
    {
      my_putchar(ch);
    }
    s++;
  }while (ch != '\0');
}
