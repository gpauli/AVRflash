###############################################################################
# Makefile for the project AVRflash
###############################################################################
# Revision: $Id: Makefile,v 1.1.1.1 2011/01/19 09:33:55 gp Exp $

#---------------------------------------------------------------------
# ATMega8
# Adressierung in Words 2 Bytes / Adresse
#---------------------------------------------------------------------
# Fuse high byte:
# 0xd9 = 1 1 0 1   1 0 0 1
#        ^ ^ ^ ^   ^ ^ ^ ^
#        | | | |   | | | +------ BOOTRST
#        | | | |   | |  +------- BOOTSZ0
#        | | | |   | +---------- BOOTSZ1
#        | | | |   |                   
#        | | | |   + --------- EESAVE (preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WTDON
#        +-------------------- RSTDISBL
# Fuse low byte:
# 0xe4 = 1 1 1 0   0 1 0 0
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 ( 8M  internal RC)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ BODEN (if 0: Clock output enabled)
#        +-------------------- BODLEVEL (if 0: divide by 8)
# Lock Bits:
# 0x2f = 0 0   1 0 1 1 1 1
#              \--+--/ \ /
#                 |     +----- LB 1..0 ( 11 = no locking 
#                 |                      10 = lock write 
#                 |                           - flash,eeprom,fuse
#                 |                      00 = lock write,read
#                 |                           - flash,eeprom,fuse
#                 |                           + write: BLB )
#                 +----------- BLB 3..0 ( 1111 = no restriction
#                                         xxx0 = SPM !write appl
#                                         xx0x = LPM !read appl
#                                                !int in app
#                                         x0xx = SPM !write boot
#                                         0xxx = LPM !read boot
#                                                !int in boot
#---------------------------------------------------------------------

MAX_UPLOAD=14336

FUSEOPT=-U hfuse:w:0xd9:m -U lfuse:w:0xe4:m 
LOCKOPT=-U lock:w:0x2f:m

AVRD = /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin
AVRDUDE = $(AVRD)/avrdude

## General Flags
PROJECT = AVRflash
MCU = atmega8
#MCU = atmega16
#MCU = atmega48
#MCU = atmega88
#MCU = atmega328p
TARGET = $(PROJECT).elf
CC = $(AVRD)/avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) -g

## Compile options common for all C compilation units.
CFLAGS = $(COMMON) -DDEBUG_LEVEL=1
CFLAGS += -Wall -DF_CPU=8000000UL -Os -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-g

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,--relax,--gc-sections 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings
DEBUG_BIN_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

## Objects that must be built in order to link
OBJECTS = oddebug.o $(PROJECT).o

## Objects explicitly added by the user
LINKONLYOBJECTS =  

## Build
all:
	@make firmware

firmware: $(TARGET) $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss $(PROJECT).bin size

.o: .c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -d -S $< > $@

%.bin: $(TARGET)
	avr-objcopy -O binary $(DEBUG_BIN_FLAGS)  $< $@

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf dep/* $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss $(PROJECT).bin

size: ${TARGET}
	@echo
	@$(AVRD)/avr-size -C --mcu=${MCU} ${TARGET}
	@echo "Max Upload: " $(MAX_UPLOAD)
	@echo

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

fuse:
	avrdude -c arduino -P /dev/tty.usbserial-A800evpY -b 19200 -p $(MCU) $(FUSEOPT)

lock:
	avrdude -c arduino -P /dev/tty.usbserial-A800evpY -b 19200 -p $(MCU) $(LOCKOPT)

isp: 
	avrdude -c arduino -P /dev/tty.usbserial-A800evpY -b 19200 -p $(MCU) -U flash:w:$(PROJECT).hex

info:
	avrdude -c arduino -P /dev/tty.usbserial-A800evpY -b 19200 -p $(MCU) -v




