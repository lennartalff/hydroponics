

BUILD_DIR=build
IDIR=include
OBJ_DIR=$(BUILD_DIR)/obj
OUT_DIR=bin

MCU=atmega2560
PROGRAMMER=avrispv2
#wiring
PORT=-P/dev/avrisp
BAUD=-b57600
F_CPU=16000000

FIRMWARE_TARGET=Firmware_$(MCU)
FIRMWARE_SRC_DIR=src/Firmware
FIRMWARE_SOURCES=$(wildcard $(FIRMWARE_SRC_DIR)/*.c)
FIRMWARE_BUILD_DIR=$(BUILD_DIR)/Firmware
FIRMWARE_OBJ_DIR=$(OBJ_DIR)/Firmware
FIRMWARE_OUT_DIR=$(OUT_DIR)/Firmware
FIRMWARE_OBJECTS=$(patsubst $(FIRMWARE_SRC_DIR)/%.c, $(FIRMWARE_OBJ_DIR)/%.o, $(FIRMWARE_SOURCES))

FIRMWARE_CFLAGS=-Os -std=c99 -DF_CPU=$(F_CPU)UL -I $(IDIR) -Wall -Wextra -Wpedantic -Wunused
FIRMWARE_LDFLAGS=

OBJECTS=$(patsubst src/%.c, $(ODIR)/%.o, $(SOURCES))
CFLAGS=-c -Os -std=c99 -I $(IDIR) -Wall -Wextra -Wpedantic -Wunused
LDFLAGS=



all: firmware

firmware: hex eeprom 

hex: $(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET).hex

eeprom: $(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET)_eeprom.hex

$(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET).hex: $(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET).elf
	avr-objcopy -O ihex -j .data -j .text $< $@

$(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET)_eeprom.hex: $(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET).elf
	avr-objcopy -O ihex -j .eeprom --change-section-lma .eeprom=1 $< $@

$(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET).elf: $(FIRMWARE_OBJECTS)
	mkdir -p $(@D)
	avr-gcc $(FIRMWARE_LDFLAGS) -mmcu=$(MCU) -o $@ $^


$(FIRMWARE_OBJ_DIR)/%.o: $(FIRMWARE_SRC_DIR)/%.c
	mkdir -p $(@D)
	avr-gcc $(FIRMWARE_CFLAGS) -mmcu=$(MCU) -c -o $@ $<

size:
	avr-size --mcu=$(MCU) -C $(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET).elf

program: all
	avrdude -p$(MCU) $(PORT) $(BAUD) -c$(PROGRAMMER) -Uflash:w:$(FIRMWARE_OUT_DIR)/$(FIRMWARE_TARGET).hex:a

clean_tmp:
	rm -rf $(BUILD_DIR)
	rm -rf $(FIRMWARE_OUT_DIR)/*.elf

clean:
	rm -rf $(OUT_DIR)
	rm -rf $(BUILD_DIR)
