############################################################
#
#                         Makefile
#
############################################################

PROJECT       = i2c_explore

# hier alle zusaetzlichen Softwaremodule angegeben
SRCS          = ../src/sysf103_init.o
SRCS         += ../src/my_printf.o
SRCS         += ../src/uart.o
SRCS         += ../src/i2c_devices_soft.o

INC_DIR       = -I./ -I../include

LSCRIPT       = stm32f103c8.ld

# FLASHERPROG Auswahl fuer STM32:
# 0 : STLINK-V2, 1 : 1 : stm32flash_rts  2 : stm32chflash 3 : DFU_UTIL
# FLASHERPROG Auswahl fuer LPC
# 4 : flash1114_rts

PROGPORT      = /dev/ttyUSB0
CH340RESET    = 0
ERASEFLASH    = 1
FLASHERPROG   = 1


include ../lib/libopencm3.mk
