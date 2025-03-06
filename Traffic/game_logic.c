#include "main.h"

#define TIME_CONST          1.5f
#define CHANCE_NEW_ORIGIN   70
#define CHANCE_NEW_DEST     27

void stop_logic(){
}

const int week_dests[20] = {3, 5, 8, 11, 14, 17, 20, 24, 28, 31, 35, 40, 45, 50};

void add_new_orig(t_main *main){
    int x, y;
    bool valid = false;
    int tries = 0;
    do {
        x = get_random(main->board_w);
        y = get_random(main->board_h);
        // We dont want to be right beside a destination
        if (main->mapt[y][x].type == 0){ 
            if (main->mapt[y][x + 1].type != TILE_DEST + 1){
                if (main->mapt[y + 1][x].type != TILE_DEST + 1){ 
                    if (y <= 1 || main->mapt[y - 1][x].type != TILE_DEST + 1){
                        valid = true;
                    }
                }
            }
        }
        tries++;
        if (tries > 100) return;
    } while (!valid);
    
    int color_index = -1;
    int color;
    if (main->logic.need_new_origin_color){
        // color = main->logic.colors[main->logic.colors_cnt];
        color_index = main->logic.colors_cnt;
    } else {
        for(int i = 0; i <= main->logic.colors_cnt; i++){
            if (main->logic.nb_orig_colors[i] < 2 * main->logic.nb_dest_colors[i]){
                // color = main->logic.colors[i];
                color_index = i;
                break;
            }
        }
        if (color_index == -1){
            // color = main->logic.colors[get_random(main->logic.colors_cnt)];
            color_index = get_random(main->logic.colors_cnt);
        }
    }
    color = main->logic.colors[color_index];
    
    main->logic.need_new_origin_color = false;
    main->logic.nb_orig_colors[color_index]++;
    
    int ori_id = orig_add_by_pos(main, x, y, color);
    main->origs[ori_id].color_index = color_index;
    map_add_tile(main, x, y, TILE_ORIG);
    update_crossroads(main, x, y);
}

void add_new_dest(t_main *main){
    int x, y;
    bool valid = false;
    int tries = 0;
    do {
        x = get_random(main->board_w - 2);
        y = get_random(main->board_h);
        // We dont want to be right beside an origin
        if (main->mapt[y][x].type == 0 && main->mapt[y][x + 1].type == 0){
            if (main->mapt[y][x - 1].type != TILE_ORIG + 1){
                if (main->mapt[y + 1][x].type != TILE_ORIG + 1){
                    if (y <= 1 || main->mapt[y - 1][x].type != TILE_ORIG + 1){
                        valid = true;
                    }
                }
            }
        }
        tries++;
        if (tries > 100) return;
    } while (!valid);

    int color_index = -1;
    int color;

    if (main->logic.need_new_dest_color){
        // We need a dest of the new color
        color_index = main->logic.colors_cnt;

    } else {
        // We pick a random color
        // We must check if we have enough origins of the same color
        for(int i = 0; i <= 20; i++){
            int rnd_color = get_random(main->logic.colors_cnt);
            if (main->logic.nb_orig_colors[rnd_color] >= main->logic.nb_dest_colors[rnd_color] * 1.7){
                color_index = rnd_color;
                break;
            }
        }
        // We didnt find a color with enough origins
        if (color_index == -1){
            return;
        }
    }

    color = main->logic.colors[color_index];
    
    main->logic.need_new_dest_color = false;
    main->logic.nb_dest_colors[color_index]++;
    
    int dest_id = dest_add_by_pos(main, x, y, color);
    main->dests[dest_id].color_index = color_index;
    map_add_tile(main, x, y, TILE_DEST);
    update_crossroads(main, x, y);
}

bool request_new_car(t_main *main, t_elem *dest){
    
    // Calling for cars
    int request_index = -1;
    for(int i = 0; i < NB_CARS_PER_DEST; i++){
        if (dest->cars_requests[i] == NULL){
            request_index = i;
            break;
        }
    }
    
    // Search for a an origin with a free car
    if (request_index == -1) 
        return false;

    int path_len = 9999;
    t_elem *closest_orig = NULL;

    // We must check all paths connected to this dest to find the closest 
    // origin of the same color with a free car
    t_tile_paths *tps = dest->paths;
    while (tps != NULL){
        if (tps->path->active != 0){

            int orig_id = tps->path->orig->index;
            t_elem *orig = &main->origs[orig_id];
            if (orig->active != 0 && orig->nb_cars_in > 0){

                // We must check if the origin has a car of the same color
                if (orig->color == dest->color){

                    // Is path shorter ?
                    if (tps->path->length < path_len){

                        // Check if one car has no cooldown
                        bool car_found = false;
                        for(int i = 0; i < NB_CARS_PER_ORIG; i++){
                            if (orig->cars_cooldown[i] == 0){
                                car_found = true;
                                break;
                            }
                        }
                        if (car_found){
                            path_len = tps->path->length;
                            closest_orig = orig;
                        }
                    }
                }
            }
        }
        tps = tps->next;
    }

    // We found an origin with a free car
    if (closest_orig == NULL) 
        return false;

    int car_id = get_first_inactive_car(main);
    if (car_id == -1)
        return false;
    
    t_cars *car = &main->cars[car_id];
    car->index_in_path = 1;
    car->origin = closest_orig->index;
    car->dest = dest->index;
    
    // We get its position and check if it can go out
    if (place_new_car(main, car_id) == 0) 
        return false;

    // Set cooldown to -1
    for(int i = 0; i < NB_CARS_PER_ORIG; i++){
        if (closest_orig->cars_cooldown[i] == 0){
            closest_orig->cars_cooldown[i] = -1;
            break;
        }
    }

    // We draw the car
    prtxy(car->x, car->y, "%s©%s", colors[car->color], RESET);
    
    dest->cars_requests[request_index] = car;
    prt_nb_cars_orig(main, closest_orig->index);

    return true;
}


// UPDATE DESTINATIONS TIMERS and REQUEST NEW CARS

void update_dests_timers(t_main *main){
    int index = 0;
    bool must_update_dest_infos = false;
    t_elem *dest = elem_get_next_active(main->dests, &index);

    const float bar_time = DEST_NB_BARS / DEST_DEFAULT_TIME;

    main->logic.warning_will_loose = false;

    while (dest != NULL){
        if (dest->cooldown > 0){
            dest->cooldown--;
        } else {
            if (dest->time_left > 0){
                dest->time_left--;
                
            } else if (main->god_mode == false){
                main->game_over = true;
                
            }
            dest->nb_bars = (DEST_DEFAULT_TIME - dest->time_left) * bar_time;
            if (dest->nb_bars != dest->prev_nb_bars){
                dest->prev_nb_bars = dest->nb_bars;
                must_update_dest_infos = true;
            }
        }
        
    // GET NEW CARS
        int nb_car_requested = 0;
        for(int i = 0; i < NB_CARS_PER_DEST; i++){
            if (dest->cars_requests[i] != NULL){
                nb_car_requested++;
            }
        }
        if (dest->nb_bars > 2 || nb_car_requested < 4){
            must_update_dest_infos |= request_new_car(main, dest);
        }
        if (dest->nb_bars > 5){
            main->logic.warning_will_loose = true;
        }
        
        if (must_update_dest_infos){
            update_dest_infos(dest);
            must_update_dest_infos = false;
        }


        index++;
        dest = elem_get_next_active(main->dests, &index);
    }
}

void update_origin_timers(t_main *main){
    int index = 0;
    t_elem *orig = elem_get_next_active(main->origs, &index);

    while (orig != NULL){
        for(int i = 0; i < NB_CARS_PER_ORIG; i++){
            if (orig->cars_cooldown[i] > 0){
                orig->cars_cooldown[i]--;
            }
        }
        index++;
        orig = elem_get_next_active(main->origs, &index);
    }
}


// UPDATE TIME and TRIGGER EVENTS

void update_time(t_main *main){
    t_logic *logic = &main->logic;
    int elapsed_min = main->game_frame * TIME_CONST;

    // Process time
    logic->day     = elapsed_min / (1440);  // Convert to full days (24h * 60min)
    logic->weekDay = logic->day % 7;
    logic->hour    = (elapsed_min % (1440)) / 60;
    logic->minute  = elapsed_min % 60;

    // Entering a new day
    if (logic->day != logic->prev_day) logic->trig_new_day = true;
    // Entering a new hour
    if (logic->hour != logic->prev_hour) logic->trig_new_hour = true;
    // Entering a new week
    if (logic->prev_weekDay != 0 && logic->weekDay == 0) logic->trig_new_week = true;

    logic->prev_day = logic->day;
    logic->prev_weekDay = logic->weekDay;
    logic->prev_hour = logic->hour;
}


// **** MAIN GAME LOGIC LOOP ****

void game_logic_loop(t_main *main){
    t_logic *logic = &main->logic;

    update_time(main);
    update_origin_timers(main);
    update_dests_timers(main);

    int week = logic->day / 7;
    int max_week_dests = (week < 20 ? week_dests[week] : week_dests[19]);

    // Every week logic
    if (logic->trig_new_week && week < 20){
        
        // Get a new color
        if (logic->day > 1){    // We don't want to change the color on the first week
            if (logic->colors_cnt < logic->max_colors - 1){
                logic->colors_cnt++;
            }
        }
        logic->need_new_origin_color = true;
        logic->need_new_dest_color = true;
    }

    t_elem *ori = &main->origs[0];
    dprtxy(220, 1, "O1: %d/%d/%d/%d  ", ori->cars_cooldown[0], ori->cars_cooldown[1], ori->cars_cooldown[2], ori->cars_cooldown[3]);

    // Every hour logic
    if (logic->trig_new_hour){

        // FOR DEBUG ****************************************************
        if (prt_debug){
            prtxy(160, 1, "Max dests: %d/%d  ", main->nb_dests, max_week_dests);
            prtxy(160, 2, "Max origs: %d/%d  ", main->nb_origs, (int)(max_week_dests * 2.5));
            prtxy(160, 3, "Colors: %d/%d  ", logic->colors_cnt, logic->max_colors);
            prtxy(180, 1, "O|  %02d |  %02d |  %02d |  %02d |  %02d |  %02d |", logic->nb_orig_colors[0], logic->nb_orig_colors[1], logic->nb_orig_colors[2],
                                        logic->nb_orig_colors[3], logic->nb_orig_colors[4], logic->nb_orig_colors[5]);
            prtxy(180, 2, "D|  %02d |  %02d |  %02d |  %02d |  %02d |  %02d |", logic->nb_dest_colors[0], logic->nb_dest_colors[1], logic->nb_dest_colors[2],
                                        logic->nb_dest_colors[3], logic->nb_dest_colors[4], logic->nb_dest_colors[5]);
        }
        
        // Add new origin
        if (main->nb_origs < MAX_ORIGS){
            if (main->nb_origs < max_week_dests * 2.7){
                int r = get_random(1000);
                int chance = (logic->day % 7) * 10 + 50;
                dprtxy(220, 2, "R: %d/%d  ", r, chance);
                if (r < chance || main->nb_origs == 0 || logic->need_new_origin_color){
                    add_new_orig(main);
                    main->need_path_update = true;
                }
            }
        }

        // Add new destination
        if (main->nb_dests < MAX_DESTS){
            if (main->nb_dests < max_week_dests && main->nb_dests < main->nb_origs / 2){
                int r = get_random(1000);
                int chance = (logic->day % 7) * 10 + 10;
                dprtxy(220, 3, "R: %d/%d  ", r, chance);
                if (r < chance || main->nb_dests == 0 || logic->need_new_dest_color){
                    add_new_dest(main);
                    main->need_path_update = true;
                }
            }
        }
    }
    
    logic->trig_new_hour = false;
    logic->trig_new_week = false;
    logic->trig_new_day = false;
}

void update_high_scores(t_main *main){
    stop_logic();
    for(int i = 0; i < 10; i++){
        if (main->player.score > main->reccord.high_scores[i]){
            for(int j = 9; j >= i; j--){
                main->reccord.high_scores[j] = main->reccord.high_scores[j - 1];
            }
            main->reccord.high_scores[i] = main->player.score;
            break;
        }
    }
}

void switch_game_to_normal(t_main *main){
    main->game_mode = 1;
    disable_new_cars = true;
    tiles_en[TILE_ORIG] = 0;
    tiles_en[TILE_DEST] = 0;
    reset_game(main);
}

void switch_game_to_sandbox(t_main *main){
    main->game_mode = 0;
    disable_new_cars = false;
    tiles_en[TILE_ORIG] = 1;
    tiles_en[TILE_DEST] = 1;
    reset_game(main);
}