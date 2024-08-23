#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>

#define DEFAULT_PORT    9468
#define BUFFER_SIZE     128

#define DEBUG_ON    0
#define PRT_IN_OUT  0

#define MAP_SIDE    10
#define yTOP        3
#define xLEFT       1

extern pthread_t thread1;
extern pthread_mutex_t mut_exit_flags;
extern pthread_mutex_t mut_new_socket;

extern int exit_flags;

extern int socket_port;
extern int server_fd;
extern int new_socket;

extern char msg[BUFFER_SIZE];

extern char got_answer;
extern char answer_touched;

extern char map[MAP_SIDE * MAP_SIDE];
extern char map_opp[MAP_SIDE * MAP_SIDE];

extern char current_player;
extern char opponent_player;
extern char self_player;
extern const int max_score;

extern int game_steps;
extern char game_draw;
extern char server_mode;

extern int hits_per_boat[5];
extern int boats_size[5];

extern int nb_self_touched;
extern int nb_opp_touched;

extern int yCur;
extern int xCur;

extern char vsteps;
extern char hsteps;

#endif