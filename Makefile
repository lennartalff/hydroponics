MCU=atmega2560
TARGET=grow_box
SOURCES=main.c uart.c ec.c
#
PROGRAMMER=wiring
PORT=-P/dev/ttyUSB0
BAUD=-b115200
F_CPU=16000000
#
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-c -Os -DF_CPU=$(F_CPU)UL
LDFLAGS=

all: hex eeprom

hex: $(TARGET).hex

eeprom: $(TARGET)_eeprom.hex

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -j .data -j .text $(TARGET).elf $(TARGET).hex

$(TARGET)_eeprom.hex: $(TARGET).elf
	avr-objcopy -O ihex -j .eeprom --change-section-lma .eeprom=1 $(TARGET).elf $(TARGET)_eeprom.hex

$(TARGET).elf: $(OBJECTS)
	avr-gcc $(LDFLAGS) -mmcu=$(MCU) $(OBJECTS) -o $(TARGET).elf

.c.o:
	avr-gcc $(CFLAGS) -mmcu=$(MCU) $< -o $@

size:
	avr-size --mcu=$(MCU) -C $(TARGET).elf

program:
	avrdude -p$(MCU) $(PORT) $(BAUD) -c$(PROGRAMMER) -D -Uflash:w:$(TARGET).hex:a

clean_tmp:
	rm -rf *.o
	rm -rf *.elf

clean:
	rm -rf *.o
	rm -rf *.elf
	rm -rf *.hex
