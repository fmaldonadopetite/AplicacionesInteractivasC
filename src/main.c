#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>
#include <string.h>


typedef int8_t i8;      //char
typedef uint8_t u8;
typedef int16_t i16;    //short
typedef uint16_t u16;
typedef int32_t i32;    //int, long (size of pointer),
typedef uint32_t u32;   //size_t
typedef int64_t i64;    //long long
typedef uint64_t u64;   //pointer

/*
 * Para evitar el delay al mantener presionada una tecla en el terminal: xset r rate 220 40
 * el default es: 660 25
 *
 */
static struct termios old_termios, new_termios;

void reset_terminal() {
    printf("\e[m"); // reset color changes
    printf("\e[?25h"); // show cursor
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void configure_terminal(){
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios; //save it to be able to reset on exit
                               //
    new_termios.c_lflag &= ~(ICANON | ECHO); //turn off echo + non-canonical mode
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    printf("\e[?25l"); //hide cursor
    atexit(reset_terminal);
}

int read_key( char *buf, int k){
    if (buf[k] == '\033' && buf[k+1] == '[') {
        switch (buf[k+2]) {
            case 'A': return 1; //UP
            case 'B': return 2; //DOWN
            case 'C': return 3; //RIGHT
            case 'D': return 4; //LEFT
        }
    }
    return 0;
}
int read_input(){
    char buf[4096]; //maximum input buffer
    int n = read(STDIN_FILENO, buf, sizeof(buf));
    int final_key = 0;
    for(int k=0; k<=n-3; k+=3){
        i32 key = read_key(buf,k);
        if (key == 0) continue;
        final_key = key;
    }

    return final_key;
}

void print_key(int key){
    if (key == 1) printf("Up\n");
    if (key == 2) printf("Down\n");
    if (key == 3) printf("Right\n");
    if (key == 4) printf("Left\n");
}

void signal_handler(int signum) {
    reset_terminal();
    signal(signum, SIG_DFL);
    raise(signum);
}

int nanosleep(const struct timespec *req, struct timespec *rem);
int main(void){
    configure_terminal();

    signal(SIGINT, signal_handler);

    struct timespec req = {};
    struct timespec rem = {};

    while(1){
        int key = read_input();
        print_key(key);

        req.tv_nsec = 0.1 * 1000000000;
        nanosleep(&req, &rem);
    }
    return 0;
}
