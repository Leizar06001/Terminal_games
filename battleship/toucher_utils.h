#ifndef TOUCHER_UTILS_H
#define TOUCHER_UTILS_H

#include <stdint.h>



void choose_and_send_first_player(int player);
void update_whos_turn();
int check_draw();
int check_win();

int64_t millis();

int min(int a, int b);
int max(int a, int b);

void resetCursor();
void mvCursor(int x, int y);

void enableRawMode();
void disableRawMode();
void moveCursor(char direction, char steps, int map);
int read_exit_flags();
void write_exit_flags(int flags);

void prt_debug();

#endif