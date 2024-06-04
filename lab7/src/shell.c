#include "shell.h"

void shell()
{
    do {
        char cmd[BUF_SIZE];
        shell_decode(cmd);
        do_(cmd);
    } while (1);
}

void shell_decode(char* cmd)
{
    uart_printf("\r# ");
    int end = 0, idx = 0, i;
    cmd[0] = '\0';
    char c;
    while ((c = uart_getc()) != '\n') {
        // decode CSI
        if (c == 27) {
            enum ANSI_ESC key = parse_CSI();
            switch (key) {
            case CursorForward:
                if (idx < end)
                    idx++;
                break;
            case CursorBackward:
                if (idx > 0)
                    idx--;
                break;
            case Delete:
                for (i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
                break;
            case Unknown:
                uart_flush();
                break;
            default:
                break;
            }
        } else if (c == 3) { // CTRL-C
            cmd[0] = '\0';
            break;
        } else if (c == 8 || c == 127) { // Backapce
            if (idx > 0) {
                idx--;
                for (i = idx; i < end; i++) {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        } else {
            if (idx < end) {
                for (i = end; i > idx; i--) {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }
        uart_printf("\r# %s \r\e[%dC", cmd, idx + 2);
        // uart_printf("\r# %s \r\e[%dC", cmd, idx + 2);
    }
    uart_printf("\n");
}

enum ANSI_ESC parse_CSI()
{
    char c = uart_getc();
    if (c == '[') {
        c = uart_getc();
        if (c == 'C') {
            return CursorForward;
        } else if (c == 'D') {
            return CursorBackward;
        } else if (c == '3') {
            c = uart_getc();
            if (c == '~') {
                return Delete;
            }
        }
    }
    return Unknown;
}

void do_(char* cmd)
{
    if (!strcmp(cmd, "")) {
        return;
    } else if (!strcmp(cmd, "help")) {
        uart_printf("help\t:\tprint all available commands\n");
        uart_printf("hello\t:\tprint Hello World!\n");
        uart_printf("reboot\t:\treboot the device\n");
        uart_printf("ls\t:\tlist all the files in the folder\n");
        uart_printf("bd_info\t:\tget board revision and memory size\n");
        uart_printf("cat <file_name>\t:\tshow the contents of the file\n");
        uart_printf("exec <program>\t:\ttest el1 to el0 exception\n");
        uart_printf("timer_on\t:\tturn on the timer\n");
        uart_printf("async_test\t:\tinput a string and print with asynchronous\n");
        uart_printf(
            "setTimeout <message> <seconds>\t:\tprint messages every seconds deponds "
            "on what you input\n");
        uart_printf("pid\t:\tget current thread id\n");
        uart_printf("fork_test\t:\ttest fork\n");
        uart_printf("sys_exe <function> <arguments>\t:\tcall system exec\n");
    } else if (!strcmp(cmd, "hello")) {
        uart_printf("Hello World!\n");
    } else if (!strcmp(cmd, "reboot")) {
        uart_printf("Rebooting...\n");
        reset(500);
        // while (1); // hang until reboot
    } else if (!strcmp(cmd, "ls")) {
        ls();
    } else if (cmd[0] == 'c' && cmd[1] == 'a' && cmd[2] == 't') {
        // char *ch = cmd+4;
        // char filename[100];
        // char *file_ptr = filename;
        // while(*ch!='\0'){
        //     *file_ptr++ = *ch++;
        // }
        // *file_ptr = '\0';
        // cat(filename);
        // uart_printf("%s\n",filename);
        cat(cmd + 4);
    } else if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'e' && cmd[3] == 'c') {
        uart_printf("exec program......\n");
        exec_(cmd + 5);
    } else if (!strcmp("bd_info", cmd)) {
        // mailbox call a exeception at number 4
        sysinfo();
    } else if (!strcmp("timer_on", cmd)) {
        // core_timer_enable();
        // timer_on();
        add_irq(&print_time_mes, "timer_on", 2, 1);
    } else if (!strcmp("async_test", cmd)) {
        uart_printf("please input a string\n");
        *IRQs1 |= AUX_INT;
        enable_interrupt();
        enable_uart_r_interrupt();

        while (1) {
            char ch = async_getc();
            // enable_uart_r_interrupt();

            async_send(ch);
            if (ch == '\n') {
                async_send('\r');
                break;
            }
        }

        disable_uart_w_interrupt();
        disable_uart_r_interrupt();
        *IRQ_disable1 |= AUX_INT;
    } else if (!strncmp("setTimeout", cmd, 10)) {
        char* ch = cmd + 11;
        // store message
        char message[128];
        char* ptr = message;
        while (*ch != ' ') {
            *ptr++ = *ch++;
        }
        ch++;
        *ptr = '\0';
        // store seconds
        int seconds = 0;
        while (*ch >= '0' && *ch <= '9') {
            seconds *= 10;
            seconds += (*ch - 48);
            ch++;
        }
        add_irq(&print_time_mes, message, seconds, 1);
    } else if (!strcmp("pid", cmd)) {
        uart_printf("current id = %d\n", getpid());
    } else if (!strcmp("fork_test", cmd)) {
        // int res = 0;
        // res = fork();
        // // child
        // if (res == 0) {
        //     uart_printf("id = %d\n", res);
        //     uart_printf("child_test\n");
        //     exit();
        // } // parent
        // else {
        //     uart_printf("child id = %d\n", res);
        //     // exit();
        // }
        fork_test();
    } else if (!strncmp("sys_exe", cmd, 7)) {
        char* ch = cmd + 8;
        // store message
        char function[128];
        char* ptr = function;
        while (*ch != ' ') {
            *ptr++ = *ch++;
        }
        ch++;
        *ptr = '\0';
        char* argv[4];
        for (int i = 0; i < 4; ++i) {
            argv[i] = kmalloc(sizeof(char) * 32);
        }
        int count = 0;
        char* arg = argv[count];
        // store arguments
        while (*ch != '\0') {
            if (*ch == ' ') {
                *arg = '\0';
                count++;
                arg = argv[count];
                // uart_printf("%s = space\n", argv[count - 1]);
            } else {
                *arg++ = *ch;
            }
            ch++;
        }
        count++;
        *arg = '\0';
        // uart_printf("function = %s\n", function);
        // exec(function, argv);
        // exec("syscall.img", argv);
        exec("vfs1.img", argv);
    } else {
        uart_printf("shell: command not found: %s\n", cmd);
    }
}
