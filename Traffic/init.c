#include "main.h"

void reset_car(t_cars *car){
    car->state = CAR_INACTIVE;
    car->x = -1;
    car->y = -1;
    car->new_x = -1;
    car->new_y = -1;
    car->next_tile_x = -1;
    car->next_tile_y = -1;
    car->next_tile_dir = 0;
    car->actual_tile = -1;
    car->next_tile = -1;
    car->dir = 0;
    car->color = 0;
    car->index_in_path = 0;
    car->path_index = 0;
    car->speed = 0;
    car->origin = -1;
    car->dest = -1;
    car->travel_dir = 1;
    car->stay_at_dest_time = 0;
    car->frames_at_dest = 0;
}

void clear_map(t_map_tile map[MAX_H][MAX_W]){
    for(int i = 0; i < MAX_H; i++){
        for(int j = 0; j < MAX_W; j++){
            map[i][j].type = 0;
            map[i][j].action = 0;
            map[i][j].paths = NULL;

            map[i][j].light.enabled = false;
            map[i][j].light.state = 0;
            map[i][j].light.elapsed = 0;
            map[i][j].light.x = j;
            map[i][j].light.y = i;
        }
    }
}

void reset_ui(t_ui *ui){
    ui->tile_over = -1;
    ui->tile_selected = -1;
    ui->prev_mouse_map.x = -1;
    ui->prev_mouse_map.y = -1;
    ui->mouse_map.x = -1;
    ui->mouse_map.y = -1;
    ui->infos_prev_x = -1;
    ui->infos_prev_y = -1;
    ui->info_prev_type = 0;
}

void reset_player(t_player *player){
    player->cash = CASH_INITIAL;
    player->score = 0;
}

void reset_logic(t_logic *logic){
    logic->day = 0;
    logic->weekDay = 0;
    logic->hour = 0;
    logic->minute = 0;
    logic->prev_day = 0;
    logic->prev_weekDay = 0;
    logic->prev_hour = 0;
    logic->trig_new_day = false;
    logic->trig_new_hour = false;
    logic->trig_new_week = true;
    logic->colors_cnt = 0;
    logic->max_colors = NB_DEST_COLORS;
    logic->need_new_origin_color = false;
    logic->need_new_dest_color = false;
    logic->warning_will_loose = false;
    logic->warning_will_loose_prev = false;

    // random pick colors and check that they are different
    for(int i = 0; i < logic->max_colors; i++){
        logic->colors[i] = dests_color_index[get_random(NB_DEST_COLORS - 1)];
        for(int j = 0; j < i; j++){
            if (logic->colors[i] == logic->colors[j]){
                i--;
                break;
            }
        }
    }

    for(int i = 0; i < NB_DEST_COLORS; i++){
        logic->nb_orig_colors[i] = 0;
        logic->nb_dest_colors[i] = 0;
    }
}

void reset_dest(t_elem *dest, int index){
    dest->active = 0;
    dest->index = index;
    dest->nb_cars_in = 0;
    dest->max_cars = 0;
    dest->nb_paths = 0;
    dest->paths = NULL;
    dest->time_left = DEST_DEFAULT_TIME;
    dest->cooldown = DEST_DEFAULT_COOLDOWN;
    dest->nb_bars = 0;
    dest->prev_nb_bars = 0;
    for(int i = 0; i < NB_CARS_PER_DEST; i++){
        dest->cars_requests[i] = NULL;
    }
}

void reset_orig(t_elem *orig, int index){
    orig->active = 0;
    orig->index = index;
    orig->nb_cars_in = NB_CARS_PER_ORIG;
    orig->max_cars = 0;
    orig->nb_paths = 0;
    orig->paths = NULL;
    for(int i = 0; i < NB_CARS_PER_ORIG; i++){
        orig->cars_cooldown[i] = 0;
    }
}

int init_main(t_main *main){
    // init random with time
    srand(time(NULL)*9);

    main->paused = false;
    main->speed_index = DEFAULT_SPEED;
    main->game_speed = speeds[main->speed_index];
    main->last_update_ui = 0;
    main->mouse_x = -1;
    main->mouse_y = -1;
    main->mouse_btn = 0;
    main->fps = 0;
    main->board_w = 0;
    main->board_h = 0;
    main->screen_w = 0;
    main->screen_h = 0;
    main->game_over = false;

    // LOGIC
    reset_logic(&main->logic);

    // PLAYER
    reset_player(&main->player);

    // UI
    reset_ui(&main->ui);

    // MAPS
    clear_map(main->mapt);
    for(int i = 0; i < S_MAP_H; i++){
        for(int j = 0; j < S_MAP_W; j++){
            main->smap[i][j].state = 0;
        }
    }

    // ORIGS AND DESTS

    main->nb_origs = 0;
    main->nb_dests = 0;
    main->need_path_update = false;

    for(int i = 0; i < MAX_ORIGS; i++){
        reset_orig(&main->origs[i], i);
    }
    for(int i = 0; i < MAX_DESTS; i++){
        reset_dest(&main->dests[i], i);
    }
    main->origs[MAX_ORIGS - 1].active = -1; // Last element is the end of the list
    main->dests[MAX_DESTS - 1].active = -1; // Last element is the end of the list

    // PATHS

    main->nb_paths = 0;

    for(int i = 0; i < MAX_ORIGS; i++){
        for(int j = 0; j < MAX_DESTS; j++){
            for(int k = 0; k < 2; k++){
                main->paths[i][j][k].steps = NULL;
                main->paths[i][j][k].length = -1;
                main->paths[i][j][k].active = false;
                main->paths[i][j][k].nb_cars_using = 0;
                main->paths[i][j][k].orig = NULL;
                main->paths[i][j][k].dest = NULL;
                main->paths[i][j][k].cars = NULL;
            }
        }
    }
    main->tiles_to_be_removed = NULL;

    // CARS
    main->last_car_out = 0;
    main->last_cars_update = 0;
    main->game_frame = 0;
    main->nb_cars_out = 0;
    main->chance_new_car = CHANCES_TO_ADD_CAR;
    main->nb_cars_per_origins = NB_CARS_PER_ORIG;

    for(int i =0; i < MAX_CARS; i++){
        reset_car(&main->cars[i]);
        main->cars[i].id = i;
    }

    // LIGHTS
    main->lights = NULL;

    // init random with time
    srand(time(NULL));

    return 0;
}

int free_all(t_main *main){
    // Free lights list
    t_light_list *light = main->lights;
    while (light != NULL){
        t_light_list *next = light->next;
        free(light);
        light = next;
    }

    // Free all paths
    for(int i = 0; i < MAX_ORIGS; i++){
        t_tile_paths *tps = main->origs[i].paths;
        while (tps != NULL){
            t_tile_paths *next = tps->next;
            free(tps);
            tps = next;
        }

        for(int j = 0; j < MAX_DESTS; j++){
            for(int k = 0; k < 2; k++){
                if (main->paths[i][j][k].steps != NULL){

                    // Free cars in path
                    t_car_list *car = main->paths[i][j][k].cars;
                    while (car != NULL){
                        t_car_list *next = car->next;
                        free(car);
                        car = next;
                    }

                    free(main->paths[i][j][k].steps);
                }
            }
        }
    }

    // Free dests paths
    for(int j = 0; j < MAX_DESTS; j++){
        t_tile_paths *tps = main->dests[j].paths;
        while (tps != NULL){
            t_tile_paths *next = tps->next;
            free(tps);
            tps = next;
        }
    }

    // Free path_ptrs
    for(int i = 0; i < MAX_H; i++){
        for(int j = 0; j < MAX_W; j++){
            // printf("[%d, %d] ", i, j);
            t_tile_paths *path = main->mapt[i][j].paths;
            while (path != NULL){
                t_tile_paths *next = path->next;
                free(path);
                path = next;
            }
        }
    }

    // Free to be removed
    t_pos *p = main->tiles_to_be_removed;
    while (p != NULL){
        t_pos *next = p->next;
        free(p);
        p = next;
    }
    return 0;
}

int quit_main(t_main *main){
    restore_terminal(&main->original);

    free_all(main);

    return 0;
}