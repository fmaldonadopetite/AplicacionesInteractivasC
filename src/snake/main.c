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


static u8 end=0;

#define WIDTH 70
#define HEIGHT 30
#define FPS 20

typedef enum {
    NADA  = 0,
    UP    = 1,
    DOWN  = 2,
    RIGHT = 3,
    LEFT  = 4,
    QUIT  = 5,
} Input;


Input read_input();
void signal_handler(int signum) ;
i32 nanosleep(const struct timespec *req, struct timespec *rem);

i32 mod_mas(i32 x, i32 y);
i32 mod_menos(i32 x, i32 y);

void signal_handler(__attribute__((unused)) int signum) {
    end=1;
}

int main(void){
    //Configuración de terminal
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios; //save it to be able to reset on exit
    new_termios.c_lflag &= ~(ICANON | ECHO); //turn off echo + non-canonical mode
    new_termios.c_cc[VTIME] = 0;
    new_termios.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    printf("\e[?25l"); //hide cursor
    signal(SIGINT, signal_handler);//para cerrar con C-c (SIGINT)

    //semilla aleatoria
    srand((unsigned) time(NULL));

    //para la función nanosleep
    struct timespec req = {};
    struct timespec rem = {};
    req.tv_nsec = (1.0/FPS) * 1000000000;

    //parámetros iniciales del juego
    int largo = 10;
    int direccion = RIGHT;
    int head, tail, manzana[2];
    int serpiente[WIDTH*HEIGHT][2] = {0};
    head = 0;
    serpiente[head][0] = WIDTH/2;
    serpiente[head][1] = HEIGHT/2;


    //dibujo de marco
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

    //dibujo de manzana
    manzana[0] = rand() % WIDTH;
    manzana[1] = rand() % HEIGHT;
    printf("\x1B[%d;%dH", manzana[1], manzana[0]);
    printf("m");

    //ciclo principal
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
        Input key = read_input();
        switch (key) {
            case UP:    if (direccion != DOWN)  direccion = key; break;
            case DOWN:  if (direccion != UP)    direccion = key; break;
            case LEFT:  if (direccion != RIGHT) direccion = key; break;
            case RIGHT: if (direccion != LEFT)  direccion = key; break;
            case QUIT:  end = 1; break;
            default: break;
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
        nanosleep(&req, &rem);
    }

    //reset terminal
    printf("\e[m"); // reset color changes
    printf("\e[?25h"); // show cursor
    printf("\x1B[1;1H");
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);

    return 0;
}

i32 mod_mas(i32 x, i32 y){
    return  (x + y)%(WIDTH*HEIGHT);
}
i32 mod_menos(i32 x, i32 y){
    return  (x - y + WIDTH*HEIGHT)%(WIDTH*HEIGHT);
}


Input read_input(){
    char buf[4096]; //maximum input buffer
    i32 n = read(STDIN_FILENO, buf, sizeof(buf));
    i32 final_key = 0;
    for(int k=0; k < n; k+=3){
        i32 key;
        if (buf[k] == 'q') key = QUIT;
        else if ( k<=n-3 && buf[k] == '\033' && buf[k+1] == '[') {
            switch (buf[k+2]) {
                case 'A': key = UP; break;
                case 'B': key = DOWN; break;
                case 'C': key = RIGHT; break;
                case 'D': key = LEFT; break;
            }
        }
        if (key == 0) continue;
        final_key = key;
    }

    return final_key;
}
