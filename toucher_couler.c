#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include "terminal_colors.h"
#include <signal.h>

#define DEFAULT_PORT 9468
#define BUFFER_SIZE 1024

#define DEBUG_ON 0
#define PRT_IN_OUT 1

#define MAP_SIDE    10
#define yTOP        3
#define xLEFT       1

static int socket_port = DEFAULT_PORT;

const char vsteps = 2;
const char hsteps = 4;

static char map[MAP_SIDE * MAP_SIDE];
static char map_opp[MAP_SIDE * MAP_SIDE];
static int xCur = 2;
static int yCur = 2;

char current_player = 1;
char self_player = 1;
char opponent_player = 2;
char winner = 0;
char opponent_ready = 0;

int nb_self_touched = 0;
int nb_opp_touched = 0;

int exit_flags = 0;
int new_socket;
int server_fd;
char got_answer = 0;
char answer_touched = 0;

pthread_t thread1;
pthread_mutex_t mut_exit_flags;
pthread_mutex_t mut_new_socket;

static char server_mode = 0;
static int end_game = 0;
static int game_steps = 0;
static char game_draw = 0;

static int score_self = 0;
static int score_opponent = 0;


static int nb_boats = 5;
static int boats_size[5] = {5, 4, 3, 3, 2};
static int hits_per_boat[5] = {0, 0, 0, 0, 0};

char msg[32];


int client_init(char *server_ip);
int server_init();
void enableRawMode();
void disableRawMode();
void moveCursor(char direction, char steps, int map);
void resetCursor();
void mvCursor(int x, int y);
void prt_debug();
int read_exit_flags();
void write_exit_flags(int flags);
int send_message(char *message);
void update_whos_turn();
int check_win(char player);
void init_new_game();
void choose_and_send_first_player(int player);
int check_draw();
int check_args(int argc, char **argv);
int init_network(char **argv);
int init_vars();
void exit_game();
void *thread_receive(void *arg);
char get_input();
int fire(int x, int y);
void anim_shoot_received(int x, int y);
void anim_boat_sinking(int player);

void signal_handler(int sig) {
    (void)sig;
    exit_game();
    exit(0);
}

int64_t millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

int min(int a, int b){
    return a < b ? a : b;
}
int max(int a, int b){
    return a > b ? a : b;
}


int main(int argc, char **argv) {
    // anim_boat_sinking(1);
    signal(SIGINT, signal_handler);
    
    if (check_args(argc, argv) != 0){
        return EXIT_FAILURE;
    }
    printf("%s", HIDE_CURSOR);
    if (init_network(argv) != 0){
        return EXIT_FAILURE;
    }
    if (init_vars() != 0){
        return EXIT_FAILURE;
    }

    prt_debug();

    int id1 = 1;
    if (pthread_create(&thread1, NULL, thread_receive, &id1) != 0) {
        fprintf(stderr, "Error creating thread 1\n");
        return EXIT_FAILURE;
    }

    init_new_game();
    if (server_mode){
        choose_and_send_first_player(2);
        update_whos_turn();
    }

    resetCursor();
    printf("%s>%s<%s", BOLD_YELLOW, MOVE_CURSOR_RIGHT, RESET);
    char c;
    while (read_exit_flags() == 0) {
        c = get_input();
        if (c == 'q'){
            write_exit_flags(1);
            break;
        }
        resetCursor();
        printf(" %s ", MOVE_CURSOR_RIGHT);
        switch (c){
            case 'A': moveCursor('A', vsteps, 0); break;
            case 'B': moveCursor('B', vsteps, 0); break;
            case 'C': moveCursor('C', hsteps, 0); break;
            case 'D': moveCursor('D', hsteps, 0); break;
            case ' ':
                if (current_player == self_player && opponent_ready){
                    fire(xCur, yCur);
                }
                break;
        }
        resetCursor();
        printf("%s>%s<%s", BOLD_YELLOW, MOVE_CURSOR_RIGHT, RESET);
    }

    exit_game();

    return 0;
}

void *thread_receive(void *arg) {
    (void)arg;
    char buffer[BUFFER_SIZE] = {0};

    while (read_exit_flags() == 0) {

        if (read(new_socket, buffer, BUFFER_SIZE) <= 0){
            printf("Client disconnected");
            pthread_mutex_lock(&mut_exit_flags);
            exit_flags = 1;
            pthread_mutex_unlock(&mut_exit_flags);
            break;
        }

#if PRT_IN_OUT == 1
        mvCursor(60, 6);
        printf("Received: %s         ", buffer);
        resetCursor();
#endif

        int len = strlen(buffer);
        int pos = 0;

        while (pos < len){
            if (buffer[pos] == 'S'){
                current_player = buffer[pos + 1] - '0';
                mvCursor(5, 51);
                update_whos_turn();
                fflush(stdout);
                pos += 2;

            } else if (buffer[pos] == 'f'){   // RECU NOUVEAU TIR
                int x = buffer[pos + 1] - '0' + 1;
                int y = buffer[pos + 3] - '0' + 1;

                if (map[(y - 1) * MAP_SIDE + x - 1] > 0){
                    sprintf(msg, "t1");
                } else {
                    sprintf(msg, "t0");
                }
                send_message(msg);
                anim_shoot_received(x, y);

                prt_debug();
                game_steps++;
                current_player = self_player;
                update_whos_turn();
                fflush(stdout);
                pos += 4;

            } else if (buffer[pos] == 't'){   // RECU TOUCHE OR NOT
                if (buffer[pos + 1] == '1'){
                    answer_touched = 1;
                } else {
                    answer_touched = 0;
                }
                got_answer = 1;
                pos += 2;

            } else if (buffer[pos] == 'r'){
                opponent_ready = 1;
                mvCursor(30, 25);
                printf("Opponent ready !");
                fflush(stdout);
                pos++;

            } else if (buffer[pos] == 'k'){
                mvCursor(5, 25);
                printf("YOU'VE SUNK A BOAT!");
                fflush(stdout);
                pos++;
                anim_boat_sinking(opponent_player);

            } else {
                pos++;
            }
        }
    }
    return NULL;
}

int fire(int x, int y){
    if (map_opp[(y - 1) * MAP_SIDE + x - 1] == 0){
        sprintf(msg, "f%d,%d", x - 1, y - 1);
        map_opp[(y - 1) * MAP_SIDE + x - 1] = 1;
        send_message(msg);

        int steps_left = 50;
        int64_t start = millis();
        int64_t last_step = 0;
        const int yLow = 35;
        const int min_sleep = 25;
        int cur_y = yLow;
        int anim_sleep = 400;
        const int target_sleep = 50;
        int64_t last_target = 0;
        int target_step = 0;
        int xBomb = 50;
        while (millis() - start < 3000 || got_answer == 0){
            if (millis() - last_step > anim_sleep && cur_y > -9){
                if (cur_y > 0){
                    mvCursor(xBomb, cur_y + 0); printf("  /\\");
                    mvCursor(xBomb, cur_y + 1); printf(" /  \\");
                    mvCursor(xBomb, cur_y + 2); printf(" |  |");
                    mvCursor(xBomb, cur_y + 3); printf(" |  |");
                    mvCursor(xBomb, cur_y + 4); printf("/ == \\");
                    mvCursor(xBomb, cur_y + 5); printf("|/**\\|");
                    if (steps_left % 2 == 0){
                        mvCursor(xBomb, cur_y + 6); printf(" %s*%s* %s* ", BOLD_HI_RED, BOLD_YELLOW, BOLD_HI_RED);
                        mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_YELLOW, BOLD_HI_RED);
                        mvCursor(xBomb, cur_y + 8); printf("   %s*  ", BOLD_YELLOW);
                    } else {
                        mvCursor(xBomb, cur_y + 6); printf(" %s* %s*%s* ", BOLD_HI_YELLOW, BOLD_RED, BOLD_HI_YELLOW);
                        mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_RED, BOLD_HI_YELLOW);
                        mvCursor(xBomb, cur_y + 8); printf("  %s*   ", BOLD_RED);
                    }
                    mvCursor(xBomb, cur_y + 9); printf("       %s", RESET);
                } else if (cur_y > -9){
                    int y = 0;
                    if (cur_y > -1) {mvCursor(xBomb, y++); printf("  /\\");}
                    if (cur_y > -2) {mvCursor(xBomb, y++); printf(" /  \\");}
                    if (cur_y > -3) {mvCursor(xBomb, y++); printf(" |  |");}
                    if (cur_y > -4) {mvCursor(xBomb, y++); printf(" |  |");}
                    if (cur_y > -5) {mvCursor(xBomb, y++); printf("/ == \\");}
                    if (cur_y > -6) {mvCursor(xBomb, y++); printf("|/**\\|");}
                    if (steps_left % 2 == 0){
                        if (cur_y > -7) {mvCursor(xBomb, cur_y + 6); printf(" %s*%s* %s* ", BOLD_HI_RED, BOLD_YELLOW, BOLD_HI_RED);}
                        if (cur_y > -8) {mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_YELLOW, BOLD_HI_RED);}
                        if (cur_y > -9) {mvCursor(xBomb, cur_y + 8); printf("   %s*  ", BOLD_YELLOW);}
                    } else {
                        if (cur_y > -7) {mvCursor(xBomb, cur_y + 6); printf(" %s* %s*%s* ", BOLD_HI_YELLOW, BOLD_RED, BOLD_HI_YELLOW);}
                        if (cur_y > -8) {mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_RED, BOLD_HI_YELLOW);}
                        if (cur_y > -9) {mvCursor(xBomb, cur_y + 8); printf("  %s*   ", BOLD_RED);}
                    }
                    mvCursor(xBomb, cur_y + 9); printf("       %s", RESET);
                }
                anim_sleep = max(400 - ((yLow - cur_y) * 3000 / yLow), min_sleep);
                cur_y--;
                steps_left--;
                last_step = millis();
                fflush(stdout);
            }
            if (millis() - last_target > target_sleep){
                target_step++;
                last_target = millis();
                if (target_step > 3){
                    target_step = 0;
                }
                resetCursor();
                switch (target_step){
                    case 0: printf("%s |%s", BOLD_HI_RED, RESET); break;
                    case 1: printf("%s /%s", BOLD_HI_MAGENTA, RESET); break;
                    case 2: printf("%s -%s", BOLD_HI_RED, RESET); break;
                    case 3: printf("%s \\%s", BOLD_HI_YELLOW, RESET); break;
                }
                printf("%s", RESET);
                fflush(stdout);
            }
        }
        if (answer_touched){
            map_opp[(y - 1) * MAP_SIDE + x - 1] = 3;
            for(int i = 0; i < 5; ++i){
                resetCursor();
                printf("%s( )%s", BOLD_HI_RED, RESET);
                fflush(stdout);
                usleep(300000);
                resetCursor();
                printf("%s(@)%s", BOLD_HI_RED, RESET);
                fflush(stdout);
                usleep(300000);
            }
            nb_opp_touched++;
        } else {
            map_opp[(y - 1) * MAP_SIDE + x - 1] = 2;
            for(int i = 0; i < 5; ++i){
                resetCursor();
                printf("%s( )%s", BOLD_CYAN, RESET);
                fflush(stdout);
                usleep(300000);
                resetCursor();
                printf("%s(@)%s", BOLD_CYAN, RESET);
                fflush(stdout);
                usleep(300000);
            }
        }
        current_player = opponent_player;
        update_whos_turn();
        return 1;
    }
    return 0;
}

char get_input(){
    char c = getchar();
    if (c == '\033') { // If the first value is esc
        getchar(); // Skip the [
        char cc = getchar(); // The real value
        return cc;
    } else if (c == 'q') {
        write_exit_flags(1);
        return -1;
    } else {
        return c;
    }
}

void anim_boat_sinking(int player){
    (void)player;
    int xBoat = 45;
    int yBoat = 14;
    for(int i = 0; i < 10; ++i){
        mvCursor(xBoat, yBoat + i - 1);                     printf("                                       ");
        mvCursor(xBoat, yBoat + 5);                         printf("----------------------------------------");
        if (i < 6) {mvCursor(xBoat + 13, yBoat + 0 + i);                 printf(" |    |    | ");}
        if (i < 5) {mvCursor(xBoat + 12, yBoat + 1 + i);                printf(" )_)  )_)  )_)  ");}
        if (i < 4) {mvCursor(xBoat + 11, yBoat + 2 + i);               printf(" )___))___))___)\\  ");}
        if (i < 3) {mvCursor(xBoat + 9, yBoat + 3 + i);             printf("  )____)____)_____)\\\\    ");}
        if (i < 2) {mvCursor(xBoat + 8, yBoat + 4 + i);             printf(" _____|____|____|____\\\\\\__ ");}
        if (i < 1) {mvCursor(xBoat, yBoat + 5 + i);         printf("---------\\                   /---------");}
        if (i % 2) {mvCursor(xBoat, yBoat + 6);             printf("  ^^^^^ ^^^^^^^^^^^^^^^^^^^^^          ");}
        else       {mvCursor(xBoat, yBoat + 6);             printf("        ^^^^^^^^^^^^^^^^^^^^^ ^^^^^    ");}
                    mvCursor(xBoat, yBoat + 7 + i % 2);     printf("    ^^^^      ^^^^     ^^^    ^^       ");
                    mvCursor(xBoat, yBoat + 8 - i % 2);     printf("         ^^^^      ^^^                 ");
                    mvCursor(xBoat, yBoat + 9);             printf("");
        fflush(stdout);
        usleep(500000);
    }
    for(int i = 0; i < 5; ++i){
        mvCursor(xBoat, yBoat + 5 + i);                     printf("                                       ");
    }
}

void anim_shoot_received(int x, int y){
    int cur_x = 60;
    const int yTarget = y * vsteps + MAP_SIDE * vsteps + 1 + yTOP + 3;
    const int xTarget = x * hsteps;
    // const int xTarget = MAP_SIDE * hsteps + 3;

    char color = 0;
    while (cur_x >= xTarget){
    
        // Redraw the map where the shot was fired
        for(int i = 0; i < MAP_SIDE; ++i){
            mvCursor(i * hsteps + 3, yTarget);
            if (map[(y - 1) * MAP_SIDE + i] == 0){
                printf("|   |");
            } else if (map[(y - 1) * MAP_SIDE + i] > 10){
                printf("|%s[x]%s|", BOLD_HI_RED, RESET);
            } else if (map[(y - 1) * MAP_SIDE + i] > 0){
                printf("|%s[x]%s|", BOLD_YELLOW, RESET);
            } else if (map[(y - 1) * MAP_SIDE + i] == -1){
                printf("|%s @ %s|", BOLD_CYAN, RESET);
            }
        }
        printf("    ");

        // Draw the bomb
        if (cur_x != xTarget){
            mvCursor(cur_x, yTarget);
            if (color == 0){
                printf("%s<=%s<<%s ", BOLD_HI_RED, BOLD_YELLOW, RESET);
                color = 1;
            } else {
                printf("%s<=%s<<%s ", BOLD_HI_YELLOW, BOLD_RED, RESET);
                color = 0;
            }
            fflush(stdout);
            usleep(100000);
        }
        cur_x--;
    }

    int map_pos = (y - 1) * MAP_SIDE + x - 1;
    if (map[map_pos] > 0){
        int boat_type = map[map_pos] - 1;
        hits_per_boat[boat_type]++;
        if (hits_per_boat[boat_type] >= boats_size[boat_type]){
            mvCursor(60, 8);
            printf("You've lost a boat!");
            send_message("k");
        }

        map[map_pos] += 10;
        mvCursor(x * hsteps, yTarget);
        printf("%s(@)%s", BOLD_HI_RED, RESET);
        nb_self_touched++;

        for(int i = 0; i < 10; ++i){
            mvCursor(x * hsteps, yTarget);
            printf("%s>%s@%s<%s", BOLD_HI_RED, BOLD_YELLOW, BOLD_HI_RED, RESET);
            fflush(stdout);
            usleep(100000);
            mvCursor(x * hsteps, yTarget);
            printf("%s @ %s", BOLD_HI_RED, RESET);
            fflush(stdout);
            usleep(100000);
        }
        mvCursor(x * hsteps, yTarget);
        printf("%s[x]%s", BOLD_HI_RED, RESET);

    } else {
        map[map_pos] = -1;
        for(int i = 0; i < 10; ++i){
            mvCursor(x * hsteps, yTarget);
            printf("%s(%s@%s)%s", BOLD_CYAN, BOLD_BLUE, BOLD_CYAN, RESET);
            fflush(stdout);
            usleep(100000);
            mvCursor(x * hsteps, yTarget);
            printf("%s @ %s", BOLD_CYAN, RESET);
            fflush(stdout);
            usleep(100000);
        }
    }
}


void exit_game(){
    printf("%s", SHOW_CURSOR);
    disableRawMode();
    pthread_join(thread1, NULL);
    close(new_socket);
    if (server_mode){
        close(server_fd);
    }
    pthread_mutex_destroy(&mut_exit_flags);
    pthread_mutex_destroy(&mut_new_socket);
}

int init_vars(){
    pthread_mutex_init(&mut_exit_flags, NULL);
    pthread_mutex_init(&mut_new_socket, NULL);
    pthread_mutex_unlock(&mut_exit_flags);
    pthread_mutex_unlock(&mut_new_socket);
    return 0;
}

int init_network(char **argv){
    int status = 0;
    if (server_mode){
        self_player = 1;
        opponent_player = 2;
        status = server_init();
        srand(time(NULL));
    } else {
        self_player = 2;
        opponent_player = 1;
        status = client_init(argv[1]);
    }
    if (status != 0){
        return -1;
    }
    return 0;
}

int check_args(int argc, char **argv){
    if (argc < 2){
        printf("Usage: Server : 0 <port>. Client : <server_ip> <port>.\n       If no port is specified, the default port is 9468.\n");
        return -1;
    }
    if (strcmp(argv[1], "0") == 0){
        server_mode = 1;
    }
    if (argc > 2){
        socket_port = atoi(argv[2]);
        if (socket_port <= 0 || socket_port > 65535){
            printf("Error: Wrong port number\n");
            return -1;
        }
    }
    return 0;
}

int check_win(char player){
    int ret = 0;
    for (int i = 0; i < MAP_SIDE; i++){
        if (map[i * MAP_SIDE] == player && map[i * MAP_SIDE + 1] == player && map[i * MAP_SIDE + 2] == player){
            ret = 1;
            break;
        }
        if (map[i] == player && map[i + MAP_SIDE] == player && map[i + 2 * MAP_SIDE] == player){
            ret = 1;
            break;
        }
    }
    if (map[0] == player && map[4] == player && map[8] == player){
        ret = 1;
    }
    if (map[2] == player && map[4] == player && map[6] == player){
        ret = 1;
    }
    if (ret == 1){
        end_game = 1;
        winner = player;
        printf("\033[15;1H");
        if (player == self_player){
            printf("You win!\n");
            score_self++;
        } else {
            printf("You loose!");
            score_opponent++;
        }
        printf("\033[17;1H");
        printf("Score: %d - %d", score_self, score_opponent);
        if (server_mode == 1){
            printf(" Press 'r' to restart");
        }
        resetCursor();
        fflush(stdout);
    }
    return ret;
}

char get_map_val(char *map, int x, int y){
    return map[y * MAP_SIDE + x];
}

void init_new_game(){
    memset(map, 0, sizeof(map));
    memset(map_opp, 0, sizeof(map_opp));
    game_steps = 0;
    game_draw = 0;

    system("clear");
    enableRawMode();

    printf("<Arrow keys> move | <Space> fire | <q> quit.\n");
    printf("\033[%d;1H", yTOP);

    char horiz[128];
    char vert[128];

    // Draw the grid
    memset(horiz, '-', MAP_SIDE * hsteps + 1);
    horiz[MAP_SIDE * hsteps + 1] = 0;
    for(int i = 0; i < MAP_SIDE + 1; i++){
        vert[i * hsteps] = '|';
        vert[i * hsteps + 1] = ' ';
        vert[i * hsteps + 2] = ' ';
        vert[i * hsteps + 3] = ' ';
    }
    vert[MAP_SIDE * hsteps + 1] = 0;

    for (int grid = 0; grid < 2; grid++){
        int top = grid * (MAP_SIDE * vsteps + 4) + yTOP;

        mvCursor(3, top);
        for(int i = 0; i < MAP_SIDE; i++){
            printf("  %d ", i + 1);
        }
        for(int i = 0; i < MAP_SIDE; i++){
            mvCursor(1, top + 1 + i * vsteps);
            printf("  %s", horiz);
            mvCursor(1, top + 1 + i * vsteps + 1);
            printf("%c %s", 'A' + i, vert);
        }
        mvCursor(1, top + 1 + MAP_SIDE * vsteps);
        printf("  %s", horiz);
    }

    // Draw the boats
    int xBoats = MAP_SIDE * hsteps + 8;
    int yBoats = MAP_SIDE * vsteps * 1.5 + yTOP + 1;
    mvCursor(xBoats, yBoats);
    printf("<xx][xx][xx][xx][xx>");
    mvCursor(xBoats, yBoats + 2);
    printf("<xx][xx][xx][xx>");
    mvCursor(xBoats, yBoats + 4);
    printf("<xx][xx][xx>");
    mvCursor(xBoats, yBoats + 6);
    printf("<xx][xx][xx>");
    mvCursor(xBoats, yBoats + 8);
    printf("<xx][xx>");

    fflush(stdout);

    int placed_boats = 0;
    int x = 0;
    int y = 0;
    int newX = 0;
    int newY = 0;
    int dir = 0;    // 0: horizontal, 1: vertical
    while (placed_boats < nb_boats){
        if (placed_boats > 0){
            mvCursor(xBoats - 3, yBoats + 2 * placed_boats - 2);
            printf("                                ");
        }
        mvCursor(xBoats - 3, yBoats + 2 * placed_boats);
        printf("@>");
        xCur = 1;
        yCur = 1;
        int this_boat_size = boats_size[placed_boats];
        int next_boat = 0;
        int spot_taken = 0;
        while (next_boat == 0){
            if (dir == 0){
                if (xCur + this_boat_size > MAP_SIDE + 1){
                    xCur = MAP_SIDE - this_boat_size + 1;
                }
            } else {
                if (yCur + this_boat_size > MAP_SIDE + 1){
                    yCur = MAP_SIDE - this_boat_size + 1;
                }
            }

            x = xCur * hsteps;
            y = yCur * vsteps + MAP_SIDE * vsteps + 1 + yTOP + 3;

            // Check if map already has a boat (for color)
            spot_taken = 0;
            for(int i = 0; i < this_boat_size; i++){
                if (dir == 0){
                    if (map[(yCur - 1) * MAP_SIDE + xCur + i - 1] != 0){
                        spot_taken = 1;
                        break;
                    }
                } else {
                    if (map[(yCur + i - 1) * MAP_SIDE + xCur - 1] != 0){
                        spot_taken = 1;
                        break;
                    }
                }
            }
            if (spot_taken == 1){
                printf("%s", BOLD_RED);
            } else {
                printf("%s", BOLD_GREEN);
            }
            
            // Move the boat
            for(int i = 0; i < this_boat_size; i++){
                if (dir == 0){
                    if (map[(yCur - 1) * MAP_SIDE + xCur + i - 1] == 0){
                        mvCursor(x + i * hsteps, y);
                        if (i == 0){
                            printf("<x]");
                        } else if (i == this_boat_size - 1){
                            printf("[x>");
                        } else {
                            printf("[x]");
                        }
                    }
                } else {
                    if (map[(yCur + i - 1) * MAP_SIDE + xCur - 1] == 0){
                        mvCursor(x, y + i * vsteps);
                        if (i == 0){
                            printf(" ^");
                        } else if (i == this_boat_size - 1){
                            printf(" v");
                        } else {
                            printf("[x]");
                        }
                    }
                }
            }
            printf("%s", RESET);
            fflush(stdout);

            char c = get_input();
            if ((c >= 'A' && c <= 'D') || c == 'r'){
                for(int i = 0; i < this_boat_size; i++){
                    if (dir == 0){
                        newX = xCur + i;
                        newY = yCur;
                    } else {
                        newX = xCur;
                        newY = yCur + i;
                    }
                    if (get_map_val(map, newX - 1, newY - 1) == 0){
                        x = newX * hsteps;
                        y = newY * vsteps + MAP_SIDE * vsteps + 1 + yTOP + 3;
                        mvCursor(x, y);
                        printf("   ");
                    }
                }
            }
            switch (c){
                case 'A':  // Up
                    moveCursor(c, vsteps, 1); 
                    break;
                case 'B':  // Down
                    moveCursor(c, vsteps, 1); 
                    break; 
                case 'C':  // Right
                    moveCursor(c, hsteps, 1); 
                    break;
                case 'D':  // Left
                    moveCursor(c, hsteps, 1); 
                    break;
                case 'r':  // Rotate
                    dir = 1 - dir;
                    break;
                case ' ':  // Place
                    if (spot_taken == 1){
                        break;
                    }
                    printf("%s", BOLD_YELLOW);
                    if (dir == 0){
                        for(int i = 0; i < this_boat_size; i++){
                            map[(yCur - 1) * MAP_SIDE + xCur + i - 1] = placed_boats + 1;
                            mvCursor(x + i * hsteps, y);
                            if (i == 0){
                                printf("<x]");
                            } else if (i == this_boat_size - 1){
                                printf("[x>");
                            } else {
                                printf("[x]");
                            }
                        }
                    } else {
                        for(int i = 0; i < this_boat_size; i++){
                            map[(yCur + i - 1) * MAP_SIDE + xCur - 1] = placed_boats + 1;
                            mvCursor(x, y + i * vsteps);
                            if (i == 0){
                                printf(" ^");
                            } else if (i == this_boat_size - 1){
                                printf(" v");
                            } else {
                                printf("[x]");
                            }
                        }
                    }
                    printf("%s", RESET);
                    placed_boats++;
                    next_boat = 1;
                    break;
            }
        }
    }
    mvCursor(xBoats - 3, yBoats + 2 * placed_boats - 2);
    printf("                                ");

    xCur = MAP_SIDE / 2;
    yCur = MAP_SIDE / 2;
    resetCursor();
    msg[0] = 'r';
    msg[1] = 0;
    send_message(msg);
}

void choose_and_send_first_player(int player){
    if (player == 0){
        current_player = rand() % 2 + 1;
    } else {
        current_player = player;
    }
    sprintf(msg, "S%d", current_player);
    send_message(msg);
}

void update_whos_turn(){
    mvCursor(5, 25);
    if (self_player == current_player){
        printf("Your turn                                ");
    } else {
        printf("Opponent's turn                          ");
    }
    resetCursor();
}

int check_draw(){
    if (game_steps >= MAP_SIDE * MAP_SIDE){
        printf("\033[15;1H");
        printf("The match ends in a draw");
        game_draw = 1;
        if (server_mode == 1){
            printf(" Press 'r' to restart");
        }
        return 1;
    }
    return 0;
}


void resetCursor() {
    printf("\033[%d;%dH", vsteps * yCur + yTOP + 1 - 1, hsteps * xCur + xLEFT + 2 - 3);
}

void mvCursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

void prt_debug(){
    if (DEBUG_ON == 0){
        return;
    }
    char horiz[128];
    char vert[128];

    // Draw the grid
    memset(horiz, '-', MAP_SIDE * hsteps + 1);
    horiz[MAP_SIDE * hsteps + 1] = 0;
    for(int i = 0; i < MAP_SIDE + 1; i++){
        vert[i * hsteps] = '|';
        vert[i * hsteps + 1] = ' ';
        vert[i * hsteps + 2] = ' ';
        vert[i * hsteps + 3] = ' ';
    }
    vert[MAP_SIDE * hsteps + 1] = 0;

    for (int grid = 0; grid < 2; grid++){
        int top = grid * (MAP_SIDE * vsteps + 4) + yTOP;

        mvCursor(63, top);
        for(int i = 0; i < MAP_SIDE; i++){
            printf("  %d ", i + 1);
        }
        for(int i = 0; i < MAP_SIDE; i++){
            mvCursor(61, top + 1 + i * vsteps);
            printf("  %s", horiz);
            mvCursor(61, top + 1 + i * vsteps + 1);
            printf("%c %s", 'A' + i, vert);
            for(int j = 0; j < MAP_SIDE; j++){
                mvCursor(61 + 4 + 4 * j, top + 1 + i * vsteps + 1);
                if (grid == 0)
                    printf("%c", (map_opp[i * MAP_SIDE + j] == 0 ? ' ' : map_opp[i * MAP_SIDE + j] + '0'));
                else
                    printf("%c", (map[i * MAP_SIDE + j] == 0 ? ' ' : map[i * MAP_SIDE + j] + '0'));
            }
        }
        mvCursor(61, top + 1 + MAP_SIDE * vsteps);
        printf("  %s", horiz);
    }
    resetCursor();
}

void enableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag |= (ICANON | ECHO); // Enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void moveCursor(char direction, char steps, int map) {
    (void)map;
    (void)steps;
    switch (direction) {
        case 'A': if(yCur > 1)         yCur--; break; // Up
        case 'B': if(yCur < MAP_SIDE)  yCur++; break; // Down
        case 'C': if(xCur < MAP_SIDE)  xCur++; break; // Right
        case 'D': if(xCur > 1)         xCur--; break; // Left
    }
    // mvCursor(xCur * hsteps, yCur * vsteps + map * MAP_SIDE * vsteps + 1 + yTOP + 3);
    resetCursor();
}

int read_exit_flags() {
    int flags;
    pthread_mutex_lock(&mut_exit_flags);
    flags = exit_flags;
    pthread_mutex_unlock(&mut_exit_flags);
    return flags;
}

void write_exit_flags(int flags) {
    pthread_mutex_lock(&mut_exit_flags);
    exit_flags = flags;
    pthread_mutex_unlock(&mut_exit_flags);
}

int send_message(char *message) {
#if PRT_IN_OUT == 1
    mvCursor(60, 5);
    printf("Sending : %s         ", message);
    resetCursor();
#endif
    return send(new_socket, message, strlen(msg), 0);
}




int server_init(){
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Créer le socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attacher le socket au port 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(socket_port);

    // Lier le socket à l'adresse et au port spécifiés
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de connexions...\n");

    // Accepter une nouvelle connexion
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int client_init(char *server_ip){
    struct sockaddr_in serv_addr;

    // Créer le socket
    if ((new_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Erreur de création du socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(socket_port);

    // Convertir l'adresse IPv4 de texte en binaire
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nAdresse invalide / Adresse non supportée \n");
        return -1;
    }

    // Connecter au serveur
    if (connect(new_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nLa connexion a échoué \n");
        return -1;
    }
    return 0;
}



