#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <signal.h>


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

#define WIDTH 70
#define HEIGHT 30

enum dir {
    NADA  = 0,
    UP    = 1,
    DOWN  = 2,
    RIGHT = 3,
    LEFT  = 4,
};


void reset_terminal() ;
void configure_terminal();
int read_key( char *buf, int k);
int read_input();
void print_key(int key);
void signal_handler(int signum) ;
int nanosleep(const struct timespec *req, struct timespec *rem);

i32 mod_mas(i32 x, i32 y);
i32 mod_menos(i32 x, i32 y);


int main(void){
    srand((unsigned) time(NULL));
    configure_terminal();
    signal(SIGINT, signal_handler);

    printf("\x1B[1;1H");
    for(i32 i=0; i<WIDTH*HEIGHT; i++){
        int fila = i/WIDTH;
        int columna = i%WIDTH;
        if( columna==WIDTH-1){
            printf("|");
        } else if( fila==HEIGHT-1){
            printf("-");
        } else {
            printf(" ");
        }
        if(columna==WIDTH-1){
            printf("\n");
        }

    }

    u8 end = 0;
    int largo = 10;
    int direccion = RIGHT;

    int head = 2;
    int tail = 0;
    int serpiente[WIDTH*HEIGHT][2] = {0};
    serpiente[head][0] = WIDTH/2;
    serpiente[head][1] = HEIGHT/2;

    int manzana[2] = {0};
    manzana[0] = rand() % WIDTH;
    manzana[1] = rand() % HEIGHT;
    printf("\x1B[%d;%dH", manzana[1], manzana[0]);
    printf("m");
    struct timespec req = {};
    struct timespec rem = {};
    while(!end){
        //dibujo
        printf("\x1B[%d;%dH", serpiente[head][1], serpiente[head][0]);
        printf("o");
        printf("\x1B[%d;%dH", serpiente[tail][1], serpiente[tail][0]);
        printf(" ");
        head = mod_mas(head,1);
        tail = mod_menos(head,largo);
        serpiente[head][1] = serpiente[mod_menos(head,1)][1];
        serpiente[head][0] = serpiente[mod_menos(head,1)][0];

        //lectura de teclado y cambio de dirección
        int key = read_input();
        switch (key) {
            case UP:    if (direccion != DOWN)  direccion = key; break;
            case DOWN:  if (direccion != UP)    direccion = key; break;
            case LEFT:  if (direccion != RIGHT) direccion = key; break;
            case RIGHT: if (direccion != LEFT)  direccion = key; break;
        }
        
        switch (direccion) {
            case UP:    serpiente[head][1] = (serpiente[head][1] - 1 + HEIGHT)% HEIGHT; break;
            case DOWN:  serpiente[head][1] = (serpiente[head][1] + 1 + HEIGHT)% HEIGHT; break;
            case LEFT:  serpiente[head][0] = (serpiente[head][0] - 1 + WIDTH)% WIDTH; break;
            case RIGHT: serpiente[head][0] = (serpiente[head][0] + 1 + WIDTH)% WIDTH; break;
        }

        //lógica de juego
        for (u16 i=1; i<largo; i++){
            i32 cuerpo =  mod_menos(head, i);
            if(serpiente[head][1] == serpiente[cuerpo][1] && serpiente[head][0] == serpiente[cuerpo][0]){
                end =1;
            }
        }
        if(serpiente[head][1] == manzana[1] && serpiente[head][0] == manzana[0]){
            largo +=1;
            manzana[0] = rand() % WIDTH;
            manzana[1] = rand() % HEIGHT;
            printf("\x1B[%d;%dH", manzana[1], manzana[0]);
            printf("m");
        }

        printf("\x1B[%d;%dH", serpiente[head][1], serpiente[head][0]);
        printf("x");
        //printf("\x1B[1;1H");
        //printf("head: %d, %d, %d\n", head, serpiente[head][1], serpiente[head][0]);
        //printf("tail: %d, %d, %d\n", tail,  serpiente[tail][1], serpiente[tail][0]);
        //printf("\n%d", head);

        fflush(stdout);
        req.tv_nsec = 0.1 * 1000000000;
        nanosleep(&req, &rem);
    }
    return 0;
}

i32 mod_mas(i32 x, i32 y){
    return  (x + y)%(WIDTH*HEIGHT);
}
i32 mod_menos(i32 x, i32 y){
    return  (x - y + WIDTH*HEIGHT)%(WIDTH*HEIGHT);
}

void reset_terminal() {
    printf("\e[m"); // reset color changes
    printf("\e[?25h"); // show cursor
    printf("\x1B[1;1H");
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
            case 'A': return UP;
            case 'B': return DOWN;
            case 'C': return RIGHT;
            case 'D': return LEFT;
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
    if (key == UP) printf("Up\n");
    if (key == DOWN) printf("Down\n");
    if (key == RIGHT) printf("Right\n");
    if (key == LEFT) printf("Left\n");
}

void signal_handler(int signum) {
    reset_terminal();
    signal(signum, SIG_DFL);
    raise(signum);
}

