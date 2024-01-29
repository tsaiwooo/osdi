SRC_DIR = src
OUT_DIR = out
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -I include
PREFIX = aarch64-linux-gnu-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

all: clean dir kernel8.img

start.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.o

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	$(LD) -nostdlib -nostartfiles start.o $(OBJS) -T linker.ld -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf $(OUT_DIR)/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio
debug:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -S -s
# asm:
# 	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm
dir:
	mkdir -p $(OUT_DIR)