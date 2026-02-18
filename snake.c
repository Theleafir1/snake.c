// Изучаю язык программирования C
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define CLEARVIEW printf("\e[1;1H\e[2J");
#define DEFAULT_HEIGHT 9
#define DEFAULT_WIDTH 17
#define FRAMOUNT 1

#define SPACE " . "
#define FRUIT "\e[31m @ \e[90m"
#define BONUS_L "\e[31m @@"
#define BONUS_R "@@ \e[90m"
#define HEAD "\e[92m 0 \e[90m"
#define TAIL "\e[32m o \e[90m"

void set_nonblocking() {
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &t);
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
}
void unset_nonblocking() {
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(0, TCSANOW, &t);
    int flags = fcntl(0, F_GETFL);
    fcntl(0, F_SETFL, flags & ~O_NONBLOCK);
}
int get_input() {
    char dir;
    return read(0, &dir, 1) == 1 ? dir : 0;
}

int end_screen(int cause, int score, int snake_len)
{
    unset_nonblocking();
    switch (cause)
    {
    case 0:
        CLEARVIEW;
        printf("Cannibalism detected! Self-collision\nScore: %d\nSnake length: %d\n", score, snake_len);
        break;
    case 1:
        CLEARVIEW;
        printf("Solid object detected\nScore: %d\nSnake length: %d\n", score, snake_len);
        break;
    case 2:
        CLEARVIEW;
        printf("Nokia Legend: Memory Full.\nScore: %d\nSnake length: %d\n", score, snake_len);
        break;
    default:
        printf("wtf man\n");
        break;
    }
    printf("Press Enter to exit\n");
    unset_nonblocking();
    usleep(2000*1000);
    tcflush(0, TCIFLUSH);
    while (getchar() != '\n');
    return 0;

}

int play(int delay, int skillChoice, int Height, int Width, int fruitsAmount) 
{
    int score = 0;
    srand(time(NULL));  //  randomization
    int dir = 'd';  //  default direction

    char bonus_progressbar_cell_size = (Width * 3 - 6) / 15;

    char *framebuffer = malloc((5 + ((Width * 13 + 1 ) * Height) + (1 + 3 * Width - 2 + 1) + (2 + 5 + 15 * bonus_progressbar_cell_size + 4) + 1));
    if (framebuffer == NULL) return 1;
    
    //  fruit
    int *fruitX = malloc(fruitsAmount * sizeof(int));
    int *fruitY = malloc(fruitsAmount * sizeof(int));
    if (fruitX == NULL || fruitY == NULL) {
        free(framebuffer);
        framebuffer = NULL;
        if (fruitX == NULL) free(fruitX);
        if (fruitY == NULL) free(fruitY);
        return 1;
    }

    for (int i = 0; i < fruitsAmount; i++) {
        fruitX[i] = rand() % Width;
        fruitY[i] = rand() % Height;
    }

    int bonusX = rand() % (Width - 1);
    int bonusY = rand() % (Height - 1);

    char ate = 0;
    char bonus_time = 0;



    //  snake
    int snake_len = 3;

    int *snakeX = malloc(Height * Width * sizeof(int));
    int *snakeY = malloc(Height * Width * sizeof(int));
    if (snakeX == NULL || snakeY == NULL) {
        free(framebuffer);
        framebuffer = NULL;
        free(fruitX);
        fruitX = NULL;
        free(fruitY);
        fruitY = NULL;
        if (snakeX != NULL) {
            free(snakeX);
            snakeX = NULL;
        }
        if (snakeY != NULL) {
            free(snakeY);
            snakeY = NULL;

        }
        return 1;
    }
    
    snakeX[2] = 0, snakeY[2] = 0;
    snakeX[1] = 1, snakeY[1] = 0;
    snakeX[0] = 2, snakeY[0] = 0;
    
    //  game cycle starts here 
    set_nonblocking();
    while (1){
        CLEARVIEW;

        int input = get_input();
    
        if (input != 0){
            if ((input == 'w' || input == 'W') && (dir != 's')) dir = 'w';
            else if ((input == 's' || input == 'S') && (dir != 'w')) dir = 's';
            else if ((input == 'a' || input == 'A') && (dir != 'd')) dir = 'a';
            else if ((input == 'd' || input == 'D') && (dir != 'a')) dir = 'd';
        }

        //  shift array
        for (int i = snake_len - 1; i > 0; i--){
            snakeX[i] = snakeX[i - 1];
            snakeY[i] = snakeY[i - 1];
        }

        //  moving
        if (dir == 'w') snakeY[0]--;
        else if (dir == 's') snakeY[0]++;
        else if (dir == 'a') snakeX[0]--;
        else if (dir == 'd') snakeX[0]++;

        //  portals
        if (snakeY[0] < 0) snakeY[0] = Height - 1 ;
        if (snakeY[0] >= Height) snakeY[0] = 0 ;
        if (snakeX[0] < 0) snakeX[0] = Width - 1 ;
        if (snakeX[0] >= Width) snakeX[0] = 0 ;

        //  collision with itself
        for (int i = 1; i < snake_len; i++){
            if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]){
                free(framebuffer);
                    framebuffer = NULL;
                free(snakeX);
                    snakeX = NULL;
                free(snakeY);
                    snakeY = NULL;
                free(fruitX);
                    fruitX = NULL;
                free(fruitY);
                    fruitY = NULL;
                return end_screen(0, score, snake_len);
            }
        }
        
        if (snake_len == Height * Width) {
            free(framebuffer);
                framebuffer = NULL;
            free(snakeX);
                snakeX = NULL;
            free(snakeY);
                snakeY = NULL;
            free(fruitX);
                    fruitX = NULL;
            free(fruitY);
                fruitY = NULL;
            return end_screen(2, score, snake_len);
        }

        //  fruit eating and generation
        for (int i = 0; i < fruitsAmount; i++){
            if (snakeX[0] == fruitX[i] && snakeY[0] == fruitY[i]){
                snakeX[snake_len] = snakeX[snake_len - 1];
                snakeY[snake_len] = snakeY[snake_len - 1];
                snake_len++;
                score += skillChoice;
                ate++;
                int onSnake;
                do {
                    onSnake = 0;
                    fruitX[i] = rand() % Width;
                    fruitY[i] = rand() % Height;
                    for (int o = 0; o < snake_len; o++){
                        if (snakeX[o] == fruitX[i] && snakeY[o] == fruitY[i]){
                            onSnake = 1;
                            break;
                        }
                    }
                } while (onSnake);
                if (ate == 5 && bonus_time == 0) {
                    bonus_time = 15;
                    ate = 0;
                    do {
                        bonusX = rand() % (Width - 1);
                        bonusY = rand() % (Height - 1);
                        int inUse = 0;
                        for (int o = 0; o < snake_len; o++) {
                            if ((bonusX == snakeX[o] || bonusX + 1 == snakeX[o]) &&
                                (bonusY == snakeY[o] || bonusY + 1 == snakeY[o])) {
                                    inUse = 1;
                                }
                        }
                        for (int o = 0; o < snake_len; o++) {
                            if ((bonusX == fruitX[o] || bonusX + 1 == fruitX[o]) &&
                                (bonusY == fruitY[o] || bonusY + 1 == fruitY[o])) {
                                    inUse = 1;
                                }
                        }
                        if (inUse == 0) break;
                    } while (1);
                }
                break;
            }
        }

        if (bonus_time > 0) {
            if ((snakeX[0] == bonusX || snakeX[0] == bonusX + 1) && (snakeY[0] == bonusY || snakeY[0] == bonusY + 1)) {
                score += bonus_time * skillChoice * 1.5;
                bonus_time = 0;
            }
        }

        //  filling framebuffer
        int p = 0;
        memcpy(&framebuffer[p], "\e[90m", 5);   //  gray color for free space
        p += 5;
        for (int y = 0; y < Height; y++){
            for (int x = 0; x < Width; x++){
                if (snakeX[0] == x && snakeY[0] == y) {            //  head
                    memcpy(&framebuffer[p], HEAD, 13);
                    p += 13;
                    goto next;
                }
                if ((bonusX == x || bonusX + 1 == x) && (bonusY == y || bonusY + 1 == y ) && bonus_time > 0) {
                    if (bonusX == x) {
                        memcpy(&framebuffer[p], BONUS_L, 8);
                        p += 8;
                    }
                    else {
                        memcpy(&framebuffer[p], BONUS_R, 8);
                        p += 8;
                    }
                    goto next;
                }
                                                              
                for (int i = 1; i < snake_len; i++) {              //  tail
                    if (snakeX[i] == x && snakeY[i] == y){
                        memcpy(&framebuffer[p], TAIL, 13);
                        p += 13;
                        goto next;
                    }
                }
                for (char j = 0; j < fruitsAmount ; j++) {
                    if (fruitX[j] == x && fruitY[j] == y) {        //  fruit
                        memcpy(&framebuffer[p], FRUIT, 13);
                        p += 13;
                        goto next;
                    }
                }                                  
                memcpy(&framebuffer[p], SPACE, 3);                 //  free space
                p += 3;

                next: //    end
            }
        framebuffer[p++] = '\n';
        }
        memcpy(&framebuffer[p++], " ", 1);
        for(char i = 0; i < Width; i++){
                memcpy(&framebuffer[p], "___", 3);
                p += 3;
        }
        p -= 2;  //  remove last 2 symbols ("__") and replace them with ...
        memcpy(&framebuffer[p], "\n  ", 3); //  new line and two spaces before score
        p += 3;

        char scoreformated[6];
        sprintf(scoreformated, "%04d ", score); //  formatting
        memcpy(&framebuffer[p], scoreformated, 5);
        p += 5;
        
        if (bonus_time != 0){
            for(char i = 1; i < bonus_time; i++){
                for (char j = 0; j < bonus_progressbar_cell_size; j++){
                    memcpy(&framebuffer[p], "=", 1);
                    p += 1;
                }
            }
            memcpy(&framebuffer[p], "\n", 1);
            p += 1;
        } else {
            memcpy(&framebuffer[p], "\n", 1);
            p += 1;
        }
        memcpy(&framebuffer[p], "\e[0m", 4);
        p += 4;                                                    //  reset color at the end of the frame
        framebuffer[p] = '\0';

        //  and finally, drawing
        printf("%s", framebuffer);

        //  cycle
        if (bonus_time > 0) bonus_time--;
        tcflush(0, TCIFLUSH);
        usleep(delay*1000);    
    }
    unset_nonblocking();
    free(framebuffer);
        framebuffer = NULL;
    free(snakeX);
        snakeX = NULL;
    free(snakeY);
        snakeY = NULL;
    free(fruitX);
        fruitX = NULL;
    free(fruitY);
        fruitY = NULL;
    return 0;
}

int main() {
    int delay = 250;
    int menuChoice;
    int skillChoice = 4;
    int running = 1;
    int Height = DEFAULT_HEIGHT;
    int Width = DEFAULT_WIDTH;
    int fruitsAmount = 1;
    
    while (running) {
        CLEARVIEW;
        printf("Snake, my first C project\n\n");
        printf("1. Play\n");
        printf("2. Settings\n");
        printf("3. Exit\n\n");
        int c;
        if (scanf("%d", &menuChoice) != 1) {
            while ((c = getchar()) != '\n' && c != EOF);  
            continue;
        }

        switch (menuChoice) {
            case 1:
                CLEARVIEW
                play(delay, skillChoice, Height, Width, fruitsAmount);
                unset_nonblocking();
                tcflush(0, TCIFLUSH);
                while (getchar() != '\n' && getchar() != EOF);
                break;
            case 2:
                CLEARVIEW
                int settingsChoiсe;
                printf("1. Select skill level\n2. Select amount of fruits\n3. Select map type\n4. Select map size\n5. Exit\n");
                scanf("%d", &settingsChoiсe);
                switch (settingsChoiсe){
                    case 1:{
                        CLEARVIEW
                        printf("Choose skill level from\n1. 500 ms\n 2. 400 ms\n 3. 320 ms\n 4. 250 ms\n 5. 200 ms\n 6. 170 ms\n 7. 140 ms\n 8. 110 ms\n 9. 90 ms\n 10. Custom\n");
                        scanf("%d", &skillChoice);
                        switch (skillChoice) {
                            case 1:
                                delay = 500;
                                break;
                            case 2:
                                delay = 400;
                                break;
                            case 3:
                                delay = 320;
                                break;
                            case 4:
                                delay = 250;
                                break;
                            case 5:
                                delay = 200;
                                break;
                            case 6:
                                delay = 170;
                                break;
                            case 7:
                                delay = 140;
                                break;
                            case 8:
                                delay = 110;
                                break;
                            case 9:
                                delay = 100;
                                break;
                            case 10:
                                printf("Enter delay [ms]: ");
                                scanf("%d", &delay);
                                break;
                            default:
                                printf("Invalid choice.\n");
                                usleep(1000*1000);
                                break;
                        }
                        break;
                    }
                    case 2:{
                        CLEARVIEW
                        printf("Choose amount of fruits\n");
                        if (scanf("%d", &fruitsAmount) != 1) {
                            CLEARVIEW
                            printf("ARE YOU FUCKING STUPID\n");
                            usleep(1000*1000);
                            return 9309706;
                        }
                        break;
                    }
                    case 3:{
                        CLEARVIEW
                        printf("Maps are'nt implemented yet\n");
                        usleep(1000*1000);
                        break;
                    }
                    case 4:{
                        CLEARVIEW
                        printf("Choose map size:\n");
                        printf("1. Default (%dx%d)\n2. Small (12x6)\n3. Large (22x11)\n4. Custom\n5. Exit\n", DEFAULT_WIDTH, DEFAULT_HEIGHT);
                        int sizeChoice;
                        scanf("%d", &sizeChoice);
                        switch (sizeChoice) {
                            case 1:
                                Height = DEFAULT_HEIGHT;
                                Width = DEFAULT_WIDTH;
                                break;
                            case 2:
                                Height = 6;
                                Width = 12;
                                break;
                            case 3:
                                Height = 11;
                                Width = 22;
                                break;
                            case 4:
                                printf("Enter {height}X{width} (e.g. 13x16)\n");
                                scanf("%dx%d", &Height, &Width);
                                break;
                            case 5:
                                break;
                            default:
                                printf("Invalid choice.\n");
                        }
                        break;
                    }
                    case 5:{
                        break;
                    }
                    
                }
            break;
            case 3:
                CLEARVIEW
                running = 0;
                break;
            default:
                printf("Invalid choice.\n");
                usleep(500*1000);
        }
    }
    return 0;
}