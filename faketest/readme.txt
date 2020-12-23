stm32_test_115200bd.hex
---------------------------------------------------------------------------------------------

In letzter Zeit sind gefaelschte STM32F103C8T6 Mikrocontroller, besonders auf der sogenannten
BluePill aufgetaucht.

Diese Controller haben leistungsmaessige Einbussen bzw. Fehlfunktionen hinsichtlich den
originalen Controllern der Firma ST.

Auesserlich erkennbar sind diese daran, dass sie zusaetzlich zu der Einkerbung die den An-
schlusspin 1 markiert, 2 zusaetzliche flachere Vertiefungen besitzen.

Ausserdem ist ein Teil der Beschriftung: MYS807 (Seriennummer).

Die Firmware stm32_test_115200bd.hex testet den Controller daraufhin, ob es sich um eine
Faelschung oder nicht handelt.

Einbussen der Faelschungen sind:

        - I2C Acknowledgefehler
        - DMA Fehler
        - UART funktioniert nicht mit der maximalen Geschwindigkeit

Die Firmware stm32_test_115200bd.hex testet, ob es sich um eine Faelschung oder nicht handelt.
Ueber die serielle Schnittstelle (PA9 / PA10; Baudrate 115200bd) wird ein Statusbericht
ausgegeben, der einzelne Funktionen des Controllers testet.

Die Firmware kann ueber den seriellen Bootloader (der auch mit der Faelschung funktioniert)
geflasht werden:

        stm32flash -w stm32_test_115200bd.hex -g0 /dev/ttyUSB0

oder

        ./upload

