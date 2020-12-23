PROJECT         = tetris_v2

# hier alle zusaetzlichen Softwaremodule angeben
SRCS            = ../src/sysf103_init.o
SRCS           += ../src/my_printf.o
SRCS           += ../src/tftdisplay.o
SRCS           += ../src/gfx_pictures.o

SRC_DIR         = -I./ -I../src -I../include

INC_DIR         = -I./ -I../include

LSCRIPT         = stm32f103cb.ld

# FLASHERPROG Auswahl fuer STM32:
# 0 : STLINK-V2, 1 : 1 : stm32flash_rts  2 : stm32chflash 3 : DFU_UTIL
# FLASHERPROG Auswahl fuer LPC
# 4 : flash1114_rts

PROGPORT      = /dev/ttyUSB0
CH340RESET    = 0
ERASEFLASH    = 1
FLASHERPROG   = 0


include ../lib/libopencm3.mk

