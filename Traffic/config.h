#ifndef CONFIG_H
#define CONFIG_H

#define Y_TOP 6     // Bottom of the top bar, top of map
#define MAX_H 35
#define MAX_W 100

#define DEFAULT_SPEED   1
#define UI_UPDATE_DELAY 100     // UI refresh delay in ms

#define CASH_INITIAL    4000
#define CASH_PER_CAR    1
#define CASH_PER_ROAD   40
#define CASH_PER_CROSS  60

#define SCORE_PER_CAR   1

#define CAR_STAY_AT_DEST_MIN_TIME 15        // Frames
#define CAR_STAY_AT_DEST_MAX_TIME 70       // Frames

#define MAX_ORIGS           100
#define MAX_DESTS           50
#define NB_CARS_PER_ORIG    4
#define MAX_CARS_PER_ORIG   9
#define NB_CARS_PER_DEST    8

#define MAX_CARS MAX_ORIGS * MAX_CARS_PER_ORIG

#define CHANCES_TO_ADD_CAR 300        // % 1000
#define MAX_CHANCE_ADD_CAR 1000


// ****** GAME LOGIC ******
#define DEST_DEFAULT_COOLDOWN   1000     // Frames
#define DEST_DEFAULT_TIME       1000.0        // Frames
#define DEST_NB_BARS            9
#define DEST_NB_BARS_TIME       DEST_DEFAULT_TIME / DEST_NB_BARS
#define DEST_EXTRA_TIME_PER_CAR 50

#define ORIG_CAR_COOLDOWN       50
// ****** GAME LOGIC ******


#define MAX_PATHS_UPDATE_LOOP 10        // Max number of paths calculated per frame, reduce freezes

#define TILE_H      4
#define TILE_W      5

#endif