#ifndef MAIN_H
#define MAIN_H

#define CAR_INACTIVE 0
#define CAR_DRIVING 1
#define CAR_AT_DEST 2

#define TILE_VERT   0    // Vertical road
#define TILE_HORI   1    // Horizontal road
#define TILE_CROS   2    // Crossroad
#define TILE_ORIG   3    // Origin
#define TILE_DEST   4    // Destination
#define TILE_LIGHT  5    // Traffic light


#include "config.h"
#include "includes.h"

#define S_MAP_W MAX_W * TILE_W
#define S_MAP_H MAX_H * TILE_H

typedef struct s_cars {
    int     id;             // Car id
    uint8_t state;          // 0: inactive, 1: driving, 2: at destination
    int     x;              // Position on the screen
    int     y;              // Position on the screen
    int     new_x;          // Next position on the screen
    int     new_y;          // Next position on the screen
    int     next_tile_x;    // Next tile entrance
    int     next_tile_y;    // Next tile entrance
    int     index_in_path;  // Position in the current path
    int     path_index;     // Index of the path (0: Main path, 1: Path with delete flag)
    char    dir;            // Direction (N, S, E, W)
    char    next_tile_dir;  // Direction to go to the next tile
    int     actual_tile;    // Actual tile type
    int     next_tile;      // Next tile type
    int     speed;          // Speed of the car (not used)
    int     origin;         // Origin index
    int     dest;           // Destination index
    uint8_t color;          // Color of the car
    uint16_t stay_at_dest_time; // Time to stay at destination
    int     frames_at_dest; // Time spent at destination
    int     travel_dir;     // 1: going to dest, -1: going back to origin
    bool    can_move;       // Can the car move
} t_cars;

typedef struct s_car_list {
    t_cars *car;
    struct s_car_list *next;
} t_car_list;

typedef struct s_pos {
    int x;
    int y;
    struct s_pos *next;
} t_pos;

typedef struct s_light {
    bool enabled;
    int x;
    int y;
    int state;
    int elapsed;
} t_light;

typedef struct s_light_list {
    t_light *light;
    struct s_light_list *next;
} t_light_list;

typedef struct s_elem {
    int     active;
    int     index;
    t_pos   pos;
    int     nb_cars_in;
    int     max_cars;
    uint8_t color;
    int     color_index;    // In logic->colors[]
    int     nb_paths;
    int     time_left;
    int     cooldown;
    int     nb_bars;
    int     prev_nb_bars;
    t_cars  *cars_requests[NB_CARS_PER_DEST];
    int     cars_cooldown[NB_CARS_PER_ORIG];
    struct s_tile_paths *paths;
} t_elem;

typedef struct s_ui {
    int     tile_over;
    int     tile_selected;
    t_pos   mouse_map;
    t_pos   prev_mouse_map;
    int     infos_prev_x;
    int     infos_prev_y;
    char    info_prev_type;
} t_ui;

typedef struct s_path {
    t_pos       *steps;
    int         length;
    bool        active;
    int         nb_cars_using;
    t_car_list  *cars;  
    t_elem      *orig;
    t_elem      *dest;
} t_path;

typedef struct s_tile_paths {
    t_path *path;
    struct s_tile_paths *next;
} t_tile_paths;

typedef struct s_map_tile {
    char        type;
    char        action;
    t_light     light;
    t_tile_paths *paths;
} t_map_tile;

typedef struct s_screen_map {
    char state; // 0: free, 1: has car, 2: reserved
} t_screen_map;

typedef struct s_player {
    long cash;
    long score;
} t_player;

typedef struct s_logic {
    int day;
    int weekDay;
    int hour;
    int minute;
    int prev_day;
    int prev_weekDay;
    int prev_hour;
    bool trig_new_day;
    bool trig_new_hour;
    bool trig_new_week;

    bool warning_will_loose;
    bool warning_will_loose_prev;

    int colors_cnt;
    int max_colors;
    int colors[8];
    bool need_new_origin_color;
    bool need_new_dest_color;
    int nb_orig_colors[NB_DEST_COLORS];
    int nb_dest_colors[NB_DEST_COLORS];
} t_logic;

typedef struct s_main {
    struct termios original;
    bool    game_started;
    int     mouse_x;
    int     mouse_y;
    int     mouse_btn;
    bool    paused;
    int     game_mode;
    uint64_t last_update_ui;
    int     fps;
    bool    god_mode;
    bool    game_over;
    int     volume;
    bool    music;

    int     board_w;
    int     board_h;
    int     screen_w;
    int     screen_h;

    int     nb_paths;

    int     speed_index;
    uint16_t game_speed;

    t_player player;

    t_ui    ui;

    t_logic logic;

    t_map_tile   mapt[MAX_H][MAX_W];
    t_screen_map smap[S_MAP_H][S_MAP_W];    // To store cars positions / reservations

    t_elem  origs[MAX_ORIGS];
    t_elem  dests[MAX_DESTS];
    int     nb_origs;
    int     nb_dests;
    t_path  paths[MAX_ORIGS][MAX_DESTS][2];   // We can have 50 origins and 50 destinations
                // Index 0: Chemin des nouvelles voitures
                // Index 1: Chemin des voitures retournant à l'origine si on a trouve un chemin plus court en index 0
                //          Il sera supprime lorsque plus aucune voiture ne l'utilise
    t_pos *tiles_to_be_removed;
    uint8_t nb_cars_per_origins;

    t_light_list *lights;

    bool    need_path_update;

    int     chance_new_car;
    uint64_t last_cars_update;
    uint64_t last_car_out;
    uint64_t game_frame;
    int     nb_cars_out;
    t_cars  cars[MAX_CARS];
} t_main;

// main.c
int reset_game(t_main *main);

// title_screen.c
void title_screen(t_main *main);
void exit_screen();

// init.c
void reset_dest(t_elem *dest, int index);
void reset_orig(t_elem *orig, int index);
void reset_car(t_cars *car);
int init_main(t_main *main);
int free_all(t_main *main);
int quit_main(t_main *main);

// cars.c
void prt_nb_cars_orig(t_main *main, int orig_id);
int get_first_inactive_car(t_main *main);
int place_new_car(t_main *main, int car_id);
void update_cars(t_main *main);

// path_find.c
void update_paths(t_main *main);

// terminal.c
int init_terminal(t_main *main);
void enable_mouse_tracking();
void enable_mouse_tracking_extended();
void disable_mouse_tracking();
void set_raw_mode(struct termios *original);
void restore_terminal(struct termios *original);
void mvCursor(int x, int y);
void prtxy(int x, int y, const char *format, ...);
void dprtxy(int x, int y, const char *format, ...);
void clear_screen();
void get_terminal_size(int *w, int *h);

// ui.c
void map_add_tile(t_main *main, int x, int y, int type);
void print_dest_infos(t_main *main, t_elem *dest);
void ui_manage_mouse(t_main *main);
void draw_ui(t_main *main);
void mapToMousePos(int x, int y, int *mx, int *my);
void mousePosToMap(int mx, int my, int *x, int *y);
void update_ui(t_main *main);
void ui_draw_new_tile(t_main *main, int x, int y);

// lights.c
void update_lights(t_main *main);

// inputs.c
void flush_input(void);
int read_input(t_main *main);

// game_logic.c
void game_logic_loop(t_main *main);
void switch_game_to_normal(t_main *main);
void switch_game_to_sandbox(t_main *main);

// utils.c
char **split(const char *str, char delim);
uint64_t millis();
int get_random(int max);
int get_random_range(int min, int max);

// music.c
void init_mpg123();
void play_mp3();
void stop_mp3();
void kill_audio_process();
void set_volume(int volume);

// chained.c
int     orig_add_by_pos(t_main *main, int x, int y, int color);
int     dest_add_by_pos(t_main *main, int x, int y, int color);
int     orig_remove_by_pos(t_main *main, int x, int y);
int     dest_remove_by_pos(t_main *main, int x, int y);
void    elem_remove_by_index(t_elem *lst, int index);

t_elem *elem_get_by_pos(t_elem *lst, int x, int y);
t_elem *elem_get_next_active(t_elem *lst, int *index);

void    remove_tile_from_remove_list(t_main *main, int x, int y);
int     nb_cars_using_path_on_tile(t_main *main, int x, int y);

t_tile_paths *path_get_by_index_in_list(t_tile_paths *list, int index);

bool cancel_all_path_on_tile(t_main *main, int x, int y);
void remove_path_from_all_tiles(t_main *main, t_path *path);
bool path_remove_from_list(t_tile_paths **list, t_path *path);
void path_add_to_list(t_tile_paths **list, t_path *path);
void path_add_to_map(t_main *main, t_path *path);
int  path_count_in_list(t_tile_paths *top);
void path_add_to_map(t_main *main, t_path *path);

void add_car_to_path(t_path *path, t_cars *car);
void rm_car_from_path(t_path *path, t_cars *car);

void add_light_to_list(t_light_list **list, t_light *light);
void rm_light_from_list(t_light_list **list, t_light *light);


#endif