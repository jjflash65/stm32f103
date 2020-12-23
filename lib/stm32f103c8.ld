MEMORY
{
    ram : ORIGIN = 0x20000000, LENGTH = 20K
    rom : ORIGIN = 0x08000000, LENGTH = 64k
}

INCLUDE libopencm3_stm32f1.ld
