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


u8 read_input();
void signal_handler(int signum) ;
i32 nanosleep(const struct timespec *req, struct timespec *rem);

i32 mod_mas(i32 x, i32 y);
i32 mod_menos(i32 x, i32 y);
void p1_draw(u8 x, u8 y, char* simbolo);
void p2_draw(u8 x, u8 y, char* simbolo);

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
    i32 largo1 =10, largo2 = 10;
    i32 direccion1, direccion2;;
    direccion1 = direccion2 = RIGHT;

    //variables del juego
    u16 puntaje = 0;
    i32 head1, tail1, manzana1[2];
    i32 head2, tail2, manzana2[2];
    i32 serpiente1[WIDTH*HEIGHT][2] = {0};
    i32 serpiente2[WIDTH*HEIGHT][2] = {0};
    head1 = head2 = 0;
    char forma_cabeza1, forma_cabeza2;
    serpiente1[head1][0] =  WIDTH/2;
    serpiente2[head2][0] = WIDTH/4;
    serpiente1[head1][1] =  HEIGHT/2;
    serpiente2[head2][1] = HEIGHT/4;

    //dibujo de marco
    printf("\x1B[1;1H");
    for(i32 i=0; i<WIDTH*HEIGHT; i++){
        int fila = i/WIDTH;
        int columna = i%WIDTH;
        if( columna==WIDTH-1){
            if( fila==HEIGHT-1) printf("┘");
            else printf("│");
        } else if( fila==HEIGHT-1){
            printf("─");
        } else {
            printf(" ");
        }
        if(columna==WIDTH-1){
            printf("\n");
        }
    }

    //dibujo de manzanas
    manzana2[0] = rand() % WIDTH;
    manzana2[1] = rand() % HEIGHT;
    manzana1[0] = rand() % WIDTH;
    manzana1[1] = rand() % HEIGHT;

    //ciclo principal
    while(!end){
        //dibujo
        printf("\x1B[1;1H Puntaje: %d", puntaje);
        p1_draw(manzana1[0], manzana1[1],"o");
        p2_draw(manzana2[0], manzana2[1],"o");

        p1_draw(serpiente1[head1][0], serpiente1[head1][1], "•");
        //p1_draw(serpiente1[tail1][0], serpiente1[tail1][1], " ");
        printf("\x1B[%d;%dH", serpiente1[tail1][1], serpiente1[tail1][0]);
        printf(" ");
        head1 = mod_mas(head1,1);
        serpiente1[head1][1] = serpiente1[mod_menos(head1,1)][1];
        serpiente1[head1][0] = serpiente1[mod_menos(head1,1)][0];
        tail1 = mod_menos(head1,largo1);

        p2_draw(serpiente2[head2][0], serpiente2[head2][1], "•");
        p2_draw(serpiente2[tail2][0], serpiente2[tail2][1], " ");
        head2 = mod_mas(head2,1);
        serpiente2[head2][1] = serpiente2[mod_menos(head2,1)][1];
        serpiente2[head2][0] = serpiente2[mod_menos(head2,1)][0];
        tail2 = mod_menos(head2,largo2);

        //lectura de teclado y cambio de dirección
        Input p1_key, p2_key;
        u8 inputs = read_input();
        p1_key = inputs%10;
        p2_key = inputs/10;
        switch (p1_key) {
            case UP:    if (direccion1 != DOWN)  direccion1 = p1_key; break;
            case DOWN:  if (direccion1 != UP)    direccion1 = p1_key; break;
            case LEFT:  if (direccion1 != RIGHT) direccion1 = p1_key; break;
            case RIGHT: if (direccion1 != LEFT)  direccion1 = p1_key; break;
            case QUIT:  end = 1; break;
            default: break;
        }

        switch (direccion1) {
            case UP:    serpiente1[head1][1] = (serpiente1[head1][1] - 1 + HEIGHT)% HEIGHT; forma_cabeza1 = '^'; break;
            case DOWN:  serpiente1[head1][1] = (serpiente1[head1][1] + 1 + HEIGHT)% HEIGHT; forma_cabeza1 = 'v'; break;
            case LEFT:  serpiente1[head1][0] = (serpiente1[head1][0] - 1 + WIDTH)% WIDTH;   forma_cabeza1 = '<'; break;
            case RIGHT: serpiente1[head1][0] = (serpiente1[head1][0] + 1 + WIDTH)% WIDTH;   forma_cabeza1 = '>'; break;
        }
        switch (p2_key) {
            case UP:    if (direccion2 != DOWN)  direccion2 = p2_key; break;
            case DOWN:  if (direccion2 != UP)    direccion2 = p2_key; break;
            case LEFT:  if (direccion2 != RIGHT) direccion2 = p2_key; break;
            case RIGHT: if (direccion2 != LEFT)  direccion2 = p2_key; break;
            default: break;
        }
        switch (direccion2) {
            case UP:    serpiente2[head2][1] = (serpiente2[head2][1] - 1 + HEIGHT)% HEIGHT; forma_cabeza2 = '^'; break;
            case DOWN:  serpiente2[head2][1] = (serpiente2[head2][1] + 1 + HEIGHT)% HEIGHT; forma_cabeza2 = 'v'; break;
            case LEFT:  serpiente2[head2][0] = (serpiente2[head2][0] - 1 + WIDTH)% WIDTH;   forma_cabeza2 = '<'; break;
            case RIGHT: serpiente2[head2][0] = (serpiente2[head2][0] + 1 + WIDTH)% WIDTH;   forma_cabeza2 = '>'; break;
        }

        printf("\x1B[%d;%dH", serpiente1[head1][1], serpiente1[head1][0]);
        printf("\x1B[38;2;102;102;255m"); //morado p1
        printf("%c", forma_cabeza1);
        printf("\x1B[%d;%dH", serpiente2[head2][1], serpiente2[head2][0]);
        printf("\x1B[38;2;0;204;205m"); //turquesa p2
        printf("%c", forma_cabeza2);
        printf("\x1B[m");

        //lógica de juego
        for (u16 i=0; i<largo1; i++){
            i32 cuerpo1 =  mod_menos(head1, i);
            if(serpiente2[head2][1] == serpiente1[cuerpo1][1] && serpiente2[head2][0] == serpiente1[cuerpo1][0]){
                end =1;
                break;
            }
            cuerpo1 =  mod_menos(head1, i+1);//posible bug
            if(serpiente1[head1][1] == serpiente1[cuerpo1][1] && serpiente1[head1][0] == serpiente1[cuerpo1][0]){
                end =1;
                break;
            }
        }

        for (u16 i=0; i<largo2; i++){
            i32 cuerpo2 =  mod_menos(head2, i);
            if(serpiente1[head1][1] == serpiente2[cuerpo2][1] && serpiente1[head1][0] == serpiente2[cuerpo2][0]){
                end =1;
                break;
            }
            cuerpo2 =  mod_menos(head2, i+1);//posible bug
            if(serpiente2[head2][1] == serpiente2[cuerpo2][1] && serpiente2[head2][0] == serpiente2[cuerpo2][0]){
                end =1;
                break;
            }
        }

        if(serpiente1[head1][1] == manzana1[1] && serpiente1[head1][0] == manzana1[0]){
            largo1 +=1;
            puntaje += 50;
            manzana1[0] = rand() % WIDTH;
            manzana1[1] = rand() % HEIGHT;
        }
        if(serpiente2[head2][1] == manzana2[1] && serpiente2[head2][0] == manzana2[0]){
            largo2 +=1;
            puntaje += 50;
            manzana2[0] = rand() % WIDTH;
            manzana2[1] = rand() % HEIGHT;
        }

        fflush(stdout);
        nanosleep(&req, &rem);
    }

    printf("\x1B[%d;1H ¡GAME OVER! \t Puntaje Final: %d", HEIGHT-1, puntaje);

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

u8 read_input(){
    char buf[4096]; //maximum input buffer
    i32 n = read(STDIN_FILENO, buf, sizeof(buf));
    i32 key1, key2;
    key1 = key2 = 0;
    for(int k=0; k < n; k+=3){
        switch (buf[k]) {
            case 'w': key1 = UP; break;
            case 's': key1 = DOWN; break;
            case 'd': key1 = RIGHT; break;
            case 'a': key1 = LEFT; break;
            case 'q': key1 = QUIT; break;
        }
         if (k <= n-3 && buf[k] == '\033' && buf[k+1] == '[') {
            switch (buf[k+2]) {
                case 'A': key2 = UP; break;
                case 'B': key2 = DOWN; break;
                case 'C': key2 = RIGHT; break;
                case 'D': key2 = LEFT; break;
            }
        }
        if (key1 == 0 && key2 ==0) continue;
        return key1 + 10*key2;
    }

    return 0;
}

void p1_draw(u8 x, u8 y, char* simbolo){
    char *R = "102";
    char *G = "102";
    char *B = "255";
    printf("\x1B[38;2;%s;%s;%sm\x1B[%d;%dH%s%s",R, G, B, y, x, simbolo, "\x1B[m");
}
void p2_draw(u8 x, u8 y, char* simbolo){
    char *R = "0";
    char *G = "204";
    char *B = "205";
    printf("\x1B[38;2;%s;%s;%sm\x1B[%d;%dH%s%s",R, G, B, y, x, simbolo, "\x1B[m");
}
