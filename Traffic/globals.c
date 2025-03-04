#include "main.h"

bool running = true;
bool prt_debug = 0;
bool disable_new_cars = false;

const uint16_t speeds[NB_SPEEDS] = {250, 60, 30, 8, 1};

const char *colors[NB_COLORS] = {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    B_RED,
    B_GREEN,
    B_YELLOW,
    B_BLUE,
    B_MAGENTA,
    B_CYAN,
    B_WHITE,
    B_BLACK,
};

const int dests_color_index[NB_DEST_COLORS] = {
    C_YELLOW,
    C_B_RED,
    C_B_GREEN,
    C_B_MAGENTA,
    C_B_WHITE,
    C_B_BLUE,
};


// Charsets for cars
char *cars_charset_extended[3][4] = {
    {
        "△",
        "▽",
        "▷",
        "◁",
    },
    {
        "⮝",
        "⮟",
        "⮞",
        "⮜",
    },
    {
        "⮙",
        "⮛",
        "⮚",
        "⮘",
    }
};

char *cars_charset[3][4] = {
    {
        "△",
        "▽",
        "▷",
        "◁",
    },
    {
        "△",
        "▽",
        "▷",
        "◁",
    },
    {
        "△",
        "▽",
        "▷",
        "◁",
    }
};