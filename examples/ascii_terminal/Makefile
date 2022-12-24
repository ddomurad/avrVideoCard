SRC_DIR := ./src
OUT_DIR := ./out
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OUT_DIR)/%.o,$(SRC_FILES))
OUT_PRG	:= $(OUT_DIR)/terminal.elf
OUT_HEX	:= $(OUT_DIR)/terminal.hex

MMCU := atmega328p

DEFS := -DF_CPU=16000000
CFLAGS := -Wall -O1 -mmcu=$(MMCU)

PROGRAMMER := arduino
PROGRAMMER_PORT := /dev/ttyUSB0
PROGRAMMER_RATE := 57600

GCC=avr-gcc

all: clean $(OBJ_FILES)
	$(GCC) -mmcu=$(MMCU) $(OBJ_FILES) -o $(OUT_PRG)
	objcopy -O ihex $(OUT_PRG) $(OUT_HEX)
	avr-size -C --mcu=$(MMCU) $(OUT_PRG)

all_sim: clean $(OBJ_FILES)
	$(GCC) $(OBJ_FILES) -o $(OUT_PRG)

clean:
	rm ${OUT_DIR} -rf
	mkdir ${OUT_DIR}

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c 
	$(GCC) $(CFLAGS) -c $(DEFS) $< -o $@ 
	
flash: all
	avrdude -p $(MMCU) -c $(PROGRAMMER) -P $(PROGRAMMER_PORT) -b $(PROGRAMMER_RATE) -D -U flash:w:$(OUT_HEX)