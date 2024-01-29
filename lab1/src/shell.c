#include "shell.h"

void shell()
{
    do
    {
        char cmd[BUF_SIZE];
        shell_decode(cmd);
        do_(cmd);
    } while (1);
    
}

void shell_decode(char *cmd)
{
    uart_printf("\r# ");
    int end = 0, idx = 0, i;
    cmd[0] = '\0';
    char c;
    while((c = uart_getc()) != '\n'){
        // decode CSI
        if(c==27){
            enum ANSI_ESC key = parse_CSI();
            switch (key){
                case CursorForward:
                    if(idx<end) idx++;
                    break;
                case CursorBackward:
                    if(idx>0) idx--;
                    break;
                case Delete:
                    for(i=idx ; i<end ; i++){
                        cmd[i] = cmd[i+1];
                    }
                    cmd[--end] = '\0';
                    break;
                case Unknown:
                    uart_flush();
                    break;
                default:
                    break;
            }
        }else if(c==3){ // CTRL-C
            cmd[0] = '\0';
            break;
        }else if(c==8 || c==127){ // Backapce
            if(idx>0){
                idx--;
                for(i=idx ; i<end ; i++){
                    cmd[i] = cmd[i+1];
                }
                cmd[--end] = '\0';
            }
        }else{
            if(idx<end){
                for(i=end ; i>idx ; i--){
                    cmd[i] = cmd[i-1];
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
    if(c=='['){
        c = uart_getc();
        if(c=='C'){
            return CursorForward;
        }else if(c=='D'){
            return CursorBackward;
        }else if(c=='3'){
            c = uart_getc();
            if(c=='~'){
                return Delete;
            }
        }
    }
    return Unknown;
}

void do_(char *cmd)
{
    if (!strcmp(cmd, "")) {
        return;
    }
    else if (!strcmp(cmd, "help")) {
        uart_printf("help: print all available commands\n");
        uart_printf("hello: print Hello World!\n");
        uart_printf("reboot: reboot the device\n");
    }
    else if (!strcmp(cmd, "hello")) {
        uart_printf("Hello World!\n");
    }
    else if (!strcmp(cmd, "reboot")) {
        uart_printf("Rebooting...\n");
        reset(500);
        // while (1); // hang until reboot
    }
    else {
        uart_printf("shell: command not found: %s\n", cmd);
    }
}
