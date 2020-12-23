LIBOPENCM3DIR ?= ../lib/libopencm3

SOC ?= STM32F1

CFLAGS  = -std=gnu99 -Wall -Os -g
CFLAGS += -mcpu=cortex-m3 -mthumb
CFLAGS += -nostartfiles -ffreestanding
CFLAGS += -ffunction-sections -fdata-sections -flto

ifeq ($(INC_DIR),)
	INC_DIR    := -I./ -I../include
endif

CFLAGS += -I$(LIBOPENCM3DIR)/include
# CFLAGS += -I/usr/local/gcc-arm-none-eabi/arm-none-eabi/include
CFLAGS += $(INC_DIR)

CFLAGS += -L$(LIBOPENCM3DIR)/lib
CFLAGS += -L$(LIBOPENCM3DIR)/../
CFLAGS += -D$(SOC)

AS      = arm-none-eabi-as
CC      = arm-none-eabi-gcc
CPP     = arm-none-eabi-g++
LD      = arm-none-eabi-gcc
SIZE    = arm-none-eabi-size
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

LDFLAGS += -lopencm3_stm32f1 -Wl,--gc-sections -flto -lm -lc -lgcc -lnosys

ifeq ($(PROGPORT),)
	PROGPORT   := /dev/ttyUSB0
endif

ifeq ($(LSCRIPT),)
	LSCRIPT   := stm32f103c8.ld
endif

.PHONY: all lib size flash list clean cleanlib

all: clean $(PROJECT).elf size

lib:
	make -C $(LIBOPENCM3DIR)

$(PROJECT).elf: $(PROJECT).c $(SRCS) | lib
	$(CC) $(CFLAGS)  -o $@ -T $(LSCRIPT) $^ $(LDFLAGS)
	$(OBJCOPY) -O binary $(PROJECT).elf $(PROJECT).bin

$(PROJECT).list: $(PROJECT).elf
	$(OBJDUMP) -D $< > $(PROJECT).list

size: $(PROJECT).elf
	$(SIZE)  $(PROJECT).elf 1>&2

flash: $(PROJECT).bin

####################################################################

# STLINK-V2


ifeq ($(FLASHERPROG), 0)

ifeq ($(ERASEFLASH), 1)
	st-flash erase
endif
	st-flash write $< 0x8000000
endif

####################################################################

# serieller Bootloader mit Aktivierung ueber Reset-Controller (ATtiny13)
# RTS Leitung pulst 3 mal zum Aktivieren des Bootloadermodus

ifeq ($(FLASHERPROG), 1)

ifeq ($(CH340RESET), 1)
	@ch340reset
	@sleep 1
endif

	stm32flash_rts -b 230400 -w $< -g 0 $(PROGPORT)
endif

####################################################################

# serieller Bootloader mit Aktivierung ueber direkte RTS-Leitung
# der seriellen Schnittstelle

ifeq ($(FLASHERPROG), 2)

ifeq ($(CH340RESET), 1)
	ch340reset
	@sleep 1
endif

	stm32chflash -b 460800 -w $< -g 0 $(PROGPORT)
endif

####################################################################

# DFU-UTIL Bootloader

ifeq ($(FLASHERPROG), 3)
	dfu-util -a0 -s 0x8000000 -D $<
endif

####################################################################


clean:
	rm -f *.bin
	rm -f *.o
	rm -f *.elf
	rm -f *.list
	rm -f ../src/*.o

cleanlib: clean
	rm -f $(LIBOPENCM3DIR)/lib/stm32/f1/*.d
	rm -f $(LIBOPENCM3DIR)/lib/stm32/f1/*.o
