#include "main.h"

#define NB_SPEEDS   5
#define NB_COLORS   15
#define NB_DEST_COLORS 6

#define NB_TILES    7

extern bool running;
extern const uint16_t speeds[NB_SPEEDS];
extern const char *colors[NB_COLORS];
extern const int dests_color_index[NB_DEST_COLORS];
extern bool prt_debug;
extern bool disable_new_cars;

extern char tiles_en[NB_TILES];

extern char *cars_charset_extended[3][4];
extern char *cars_charset[3][4];

#define C_RED       0
#define C_GREEN     1
#define C_YELLOW    2
#define C_BLUE      3
#define C_MAGENTA   4
#define C_CYAN      5
#define C_WHITE     6
#define C_B_RED     7
#define C_B_GREEN   8
#define C_B_YELLOW  9
#define C_B_BLUE    10
#define C_B_MAGENTA 11
#define C_B_CYAN    12
#define C_B_WHITE   13
#define C_B_BLACK   14
