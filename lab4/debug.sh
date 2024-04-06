SESSION="QEMU"
QEMU_CMD="qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -S -s"
GDB_CMD="gdb-multiarch"

.PHONY: debug
debug:
    make close || echo "closed."
    tmux new-session -d -s $(SESSION)
    tmux rename-window -t $(SESSION):0 'Main'
    tmux split-window -v -p 20 -t $(SESSION):Main
    tmux send-keys -t $(SESSION):Main $(QEMU_CMD) C-m
    tmux select-pane -t $(SESSION):Main -U
    tmux send-keys -t $(SESSION):Main $(GDB_CMD) C-m
    tmux send-keys -t $(SESSION):Main "file kernel8.elf" C-m
    tmux send-keys -t $(SESSION):Main "target remote :1234" C-m
    tmux attach -t $(SESSION)

.PHONY: close
close:
    tmux kill-session -t $(SESSION)
