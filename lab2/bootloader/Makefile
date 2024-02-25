SRC_DIR = src
OUT_DIR = out
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)
CFLAGS = -g -Wall -O0 -ffreestanding -nostdinc -nostdlib -nostartfiles -I include
PREFIX = aarch64-linux-gnu-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

# FOR GDB DEBUG
SESSION="QEMU"
QEMU_CMD="qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -serial null -serial stdio -S -s"
GDB_CMD="gdb-multiarch"


all: clean dir bootloader.img

start.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.o

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

bootloader.img: start.o $(OBJS)
	$(LD) -nostdlib -nostartfiles start.o $(OBJS) -T linker.ld -o bootloader.elf
	$(OBJCOPY) -O binary bootloader.elf bootloader.img

clean:
	rm bootloader.elf $(OUT_DIR)/*.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -serial null -serial stdio

pty:
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -serial null -serial pty

# asm:
# 	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -d in_asm
dir:
	mkdir -p $(OUT_DIR)

# FOR DEBUG
.PHONY: debug
debug:
	make close || echo "closed."
	tmux new-session -d -s $(SESSION)
	tmux rename-window -t $(SESSION):0 'Main'
	tmux split-window -v -p 20 -t $(SESSION):Main
	tmux send-keys -t $(SESSION):Main $(QEMU_CMD) C-m
	tmux select-pane -t $(SESSION):Main -U
	tmux send-keys -t $(SESSION):Main $(GDB_CMD) C-m
	tmux send-keys -t $(SESSION):Main "file bootloader.elf" C-m
	tmux send-keys -t $(SESSION):Main "target remote :1234" C-m
	tmux attach -t $(SESSION)

.PHONY: close
close:
	tmux kill-session -t $(SESSION)