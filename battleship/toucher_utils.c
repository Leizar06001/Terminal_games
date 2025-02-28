#include "toucher_utils.h"
#include <time.h>
#include <sys/time.h>
#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include "toucher_comms.h"

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
        printf("  YOUR TURN !!!                          ");
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

int check_win(){
    int ret = 0;
    if (nb_self_touched == max_score){
        ret = self_player;
    }
    if (nb_opp_touched == max_score){
        ret = opponent_player;
    }
    mvCursor(55, 25);
    // printf("Lifes: You: %d, Ennemy: %d", max_score - nb_self_touched, max_score - nb_opp_touched);
    if (ret != 0){
        mvCursor(5, 25);
        if (ret == self_player){
            printf("  YOU WIN !!!");
        } else {
            printf(" GAME OVER !!!");
        }
        current_player = 0;
        fflush(stdout);
    }

    return ret;
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

void resetCursor() {
    printf("\033[%d;%dH", vsteps * yCur + yTOP + 1 - 1, hsteps * xCur + xLEFT + 2 - 3);
}

void mvCursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
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

