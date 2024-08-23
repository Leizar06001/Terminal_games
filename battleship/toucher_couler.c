#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>
#include <pthread.h>
#include <sys/select.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include "terminal_colors.h"
#include <signal.h>

#include "globals.h"
#include "toucher_utils.h"
#include "toucher_comms.h"
#include "toucher_anims.h"


int socket_port = DEFAULT_PORT;
int new_socket;
int server_fd;

char msg[BUFFER_SIZE];

char vsteps = 2;
char hsteps = 4;

char map[MAP_SIDE * MAP_SIDE];
char map_opp[MAP_SIDE * MAP_SIDE];
int xCur = 2;
int yCur = 2;

char current_player = 1;
char self_player = 1;
char opponent_player = 2;
char winner = 0;
char opponent_ready = 0;

int nb_self_touched = 0;
int nb_opp_touched = 0;

int exit_flags = 0;

char got_answer = 0;
char answer_touched = 0;

pthread_t thread1;
pthread_mutex_t mut_exit_flags;
pthread_mutex_t mut_new_socket;

char server_mode = 0;
int game_steps = 0;
char game_draw = 0;

// static int score_self = 0;
// static int score_opponent = 0;
const int max_score = 5 + 4 + 3 + 3 + 2;

int nb_boats = 5;
int boats_size[5] = {5, 4, 3, 3, 2};
int hits_per_boat[5] = {0, 0, 0, 0, 0};



void init_new_game();
int check_args(int argc, char **argv);
int init_network(char **argv);
int init_vars();
void exit_game();
void *thread_receive(void *arg);
char get_input();

void signal_handler(int sig) {
    (void)sig;
    exit_game();
    exit(0);
}

void display_resize_win(){
    const int h = 50;
    const int w = 90;

    char line[128];
    char walls[128];
    memset(line, '-', w);
    line[w] = 0;
    memset(walls, ' ', w);
    walls[0] = '|';
    walls[w - 1] = '|';
    walls[w] = 0;
    system("clear");
    printf("%s\n", line);
    for(int i = 1; i < h - 1; i++){
        if (i % 10 == 0){
            printf("            RESIZE THE TERMINAL TO FIT THE WHOLE FRAME, THEN PRESS ONE KEY\n");
        } else {
            printf("%s\n", walls);
        }
    }
    printf("%s\n", line);
    getchar();
    system("clear");
}

int main(int argc, char **argv) {
    display_resize_win();
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
        choose_and_send_first_player(0);
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

    struct timeval tv;
    fd_set readfds;
    int retval;

    

    while (read_exit_flags() == 0) {
        FD_ZERO(&readfds);
        FD_SET(new_socket, &readfds);

        // Set timeout to 1 second
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        retval = select(new_socket + 1, &readfds, NULL, NULL, &tv);

        if (retval == -1){
            mvCursor(1, 1);
            perror("select()");
            write_exit_flags(1);
            break;
        } else if (retval == 0){
            continue;
        }

        if (!FD_ISSET(new_socket, &readfds)){
            continue;
        }
        
        if (read(new_socket, buffer, BUFFER_SIZE) <= 0){
            mvCursor(1, 1);
            printf("Client disconnected");
            write_exit_flags(1);
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

                check_win();

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

    mvCursor(xBoats, yBoats + 10);
    printf("Move  : arrows");
    mvCursor(xBoats, yBoats + 11);
    printf("Rotate: 'r'");
    mvCursor(xBoats, yBoats + 12);
    printf("Place : space");

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
    for(int i = 0; i < 3; i++){
        mvCursor(xBoats, yBoats + 10 + i);
        printf("                             ");
    }

    xCur = MAP_SIDE / 2;
    yCur = MAP_SIDE / 2;
    resetCursor();
    msg[0] = 'r';
    msg[1] = 0;
    send_message(msg);
}









