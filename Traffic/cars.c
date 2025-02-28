#include "main.h"

int get_first_inactive_car(t_main *main){
    int i = 0;
    while (i < MAX_CARS){
        if (main->cars[i].state == CAR_INACTIVE){
            return i;
        }
        i++;
    }
    return -1;
}

char get_next_dir(t_pos *p1, t_pos *p2){
    if (p1->x == p2->x){
        if (p1->y < p2->y){
            return 'S';
        } else {
            return 'N';
        }
    } else {
        if (p1->x < p2->x){
            return 'E';
        } else {
            return 'W';
        }
    }
}

void prt_nb_cars_orig(t_main *main, int orig_id){
    t_elem *orig = &main->origs[orig_id];
    int mx, my;
    mapToMousePos(orig->pos.x, orig->pos.y, &mx, &my);
    prtxy(mx + 2, my + 2, "%d", orig->nb_cars_in);
}

void prt_nb_cars_dest(t_main *main, int dest_id){
    t_elem *dest = &main->dests[dest_id];
    int mx, my;
    mapToMousePos(dest->pos.x, dest->pos.y, &mx, &my);
    prtxy(mx + 1, my + 3, "%d/%d", dest->nb_cars_in, dest->max_cars);
}

void car_update_tile(t_main *main, t_cars *car){
    // Get the tile the car is on and the next tile
    t_path *path = &main->paths[car->origin][car->dest][car->path_index];
    t_pos tile_pos = path->steps[car->index_in_path];
    car->actual_tile = main->mapt[tile_pos.y][tile_pos.x].type - 1;

    if (car->travel_dir == 1){
        if (car->index_in_path < path->length - 1){
            t_pos next_tile_pos = path->steps[car->index_in_path + 1];
            car->next_tile = main->mapt[next_tile_pos.y][next_tile_pos.x].type - 1;
        } else {
            car->next_tile = -1;
        }
    } else {
        if (car->index_in_path > 0){
            t_pos next_tile_pos = path->steps[car->index_in_path - 1];
            car->next_tile = main->mapt[next_tile_pos.y][next_tile_pos.x].type - 1;
        } else {
            car->next_tile = -1;
        }
    }
}

// Get the entrance (x, y) of the tile2 from tile1
char get_tile_entrance(t_pos *tile1, t_pos *tile2, int *x, int *y){
    int x_tile, y_tile;
    mapToMousePos(tile2->x, tile2->y, &x_tile, &y_tile);
    
    if (tile1->x == tile2->x){
        if (tile1->y < tile2->y){
            *x = x_tile + 1;
            *y = y_tile;
            return 'S';
        } else {
            *x = x_tile + 3;
            *y = y_tile + 3;
            return 'N';
        }
    } else {
        if (tile1->x < tile2->x){
            *x = x_tile;
            *y = y_tile + 2;
            return 'E';
        } else {
            *x = x_tile + 4;
            *y = y_tile + 1;
            return 'W';
        }
    }
}

int get_next_tile_entrance_coord(t_main *main, t_cars *car){
    t_path *path = &main->paths[car->origin][car->dest][car->path_index];
    if (car->travel_dir == 1){
        if (car->index_in_path >= path->length - 1){
            return 0;
        }
        get_tile_entrance(&path->steps[car->index_in_path], &path->steps[car->index_in_path + 1], &car->next_tile_x, &car->next_tile_y);
    } else {
        if (car->index_in_path <= 0){
            return 0;
        }
        get_tile_entrance(&path->steps[car->index_in_path], &path->steps[car->index_in_path - 1], &car->next_tile_x, &car->next_tile_y);
    }
    return 1;
}

int place_new_car(t_main *main, int car_id){
    t_cars *car = &main->cars[car_id];
    t_path *path = &main->paths[car->origin][car->dest][car->path_index];

    car_update_tile(main, car);
    car->dir = get_tile_entrance(&path->steps[0], &path->steps[1], &car->x, &car->y);
    car->next_tile_dir = get_next_dir(&path->steps[1], &path->steps[2]);
    if (get_next_tile_entrance_coord(main, car) == 0){
        return 0;
    }
    // Car can exit, we place it on the map
    car->state = CAR_DRIVING;
    car->can_move = true;
    car->path_index = 0;
    car->color = main->dests[car->dest].color;
    car->stay_at_dest_time = get_random_range(CAR_STAY_AT_DEST_MIN_TIME, CAR_STAY_AT_DEST_MAX_TIME);
    main->nb_cars_out++;
    main->origs[car->origin].nb_cars_in--;
    main->paths[car->origin][car->dest][0].nb_cars_using++;
    add_car_to_path(&main->paths[car->origin][car->dest][0], car);
    return 1;
}

int place_return_car(t_main *main, int car_id){
    t_cars *car = &main->cars[car_id];
    t_path *path = &main->paths[car->origin][car->dest][car->path_index];

    car_update_tile(main, car);
    int last_tile = path->length - 1;
    car->dir = get_tile_entrance(&path->steps[last_tile], &path->steps[last_tile - 1], &car->x, &car->y);
    car->next_tile_dir = get_next_dir(&path->steps[last_tile - 1], &path->steps[last_tile - 2]);
    if (get_next_tile_entrance_coord(main, car) == 0){
        return 0;
    }
    // Car can exit, we place it on the map
    main->nb_cars_out++;
    car->state = CAR_DRIVING;
    main->paths[car->origin][car->dest][car->path_index].dest->nb_cars_in--;

    // Remove the car from dest requests
    for(int i = 0; i < NB_CARS_PER_DEST; i++){
        if (main->dests[car->dest].cars_requests[i] == car){
            main->dests[car->dest].cars_requests[i] = NULL;

            // ADDIND EXTRA TIME TO DEST
            // Extra time depends of nb of origins and dests
            int color = main->dests[car->dest].color_index;
            float ratio = 1.0;
            int extra_time = DEST_EXTRA_TIME_PER_CAR;
            if (main->logic.nb_orig_colors[color] != 0){
                ratio = (float)main->logic.nb_dest_colors[color] / (float)main->logic.nb_orig_colors[color];
                ratio += 0.3;
                if (ratio < 0.4) ratio = 0.4;
                else if (ratio > 1.4) ratio = 1.4;
                extra_time = DEST_EXTRA_TIME_PER_CAR * ratio;
            }

            main->dests[car->dest].time_left += extra_time;
            if (main->dests[car->dest].time_left > DEST_DEFAULT_TIME){
                main->dests[car->dest].time_left = DEST_DEFAULT_TIME;
            }
            break;
        }
    }
    return 1;
}

int get_new_car(t_main *main){
    if (main->nb_cars_out >= MAX_CARS) return 0;

    int new_car = 0;
    // We check all origins if there is a car that can go out
    for(int i = 0; i < MAX_ORIGS; i++){

        t_elem *orig = &main->origs[i];
        if (orig->active == 0 || orig->nb_paths == 0 || orig->nb_cars_in <= 0) continue;

        // We get a random number to determine if we add a car
        if (get_random(1000) > main->chance_new_car) continue;

        // We must determine its destination
        int orig_id = orig->index;
        int path_id = get_random(orig->nb_paths);
        
        t_tile_paths *tps = path_get_by_index_in_list(orig->paths, path_id);
        if (tps == NULL) continue;
        if (tps->path->dest->active == 0 || tps->path->active == 0) continue;

        int dest_id = tps->path->dest->index;

        // Ok we can go to this destination
        int car_id = get_first_inactive_car(main);
        if (car_id == -1){
            return 0;
        }
        t_cars *car = &main->cars[car_id];
        car->index_in_path = 1;
        car->origin = orig_id;
        car->dest = dest_id;

        // We get its position and check if it can go out
        if (place_new_car(main, car_id)){
            // We draw the car
            prtxy(car->x, car->y, "%s©%s", colors[car->color], RESET);
            
            prt_nb_cars_orig(main, orig_id);
            
            new_car = 1;
            new_car++;
        }
    }
    return new_car;
}

void remove_car(t_main *main, t_cars *car){
    rm_car_from_path(&main->paths[car->origin][car->dest][car->path_index], car);
    main->smap[car->y][car->x].state = 0;
    main->paths[car->origin][car->dest][car->path_index].nb_cars_using--;
    prtxy(car->x, car->y, " ");
    main->origs[car->origin].nb_cars_in++;
    main->nb_cars_out--;
    prt_nb_cars_orig(main, car->origin);
    reset_car(car);
}

bool free_room_in_dest(t_main *main, int dest_id){
    t_elem *dest = &main->dests[dest_id];
    if (dest->nb_cars_in >= dest->max_cars){
        return false;
    }
    return true;
}



void stop_cars(){

}

int check_path_free(t_main *main, t_cars *car){

    if (main->smap[car->new_y][car->new_x].state != 0){
        return false;
    }

    return true;
}

int check_crossroad(t_main *main, t_cars *car, int tile_x, int tile_y, t_pos *tile_pos){

    // On check si la voiture est deja sur une case reservee, elle ne doit pas s'arreter dans le croisement
    // 1 > reserved, 2 > exits, must check new pos
    const char reserved[4][5] = {"00020",
                                 "21110",
                                 "01112",
                                 "02000"};
    int x_in_tile = car->x - tile_x;
    int y_in_tile = car->y - tile_y;
    if (x_in_tile < 0 || x_in_tile > 4 || y_in_tile < 0 || y_in_tile > 3){
        return true;    // Out of bounds, ignoring
    }
    if (reserved[y_in_tile][x_in_tile] == '1'){
        return true;    // Already reserved, car can move
    }

    // On check si la voiture est sur une case de sortie, elle doit verifier la prochaine tile
    if (reserved[y_in_tile][x_in_tile] == '2'){
        return check_path_free(main, car);
    }

    // On recupere l'etat du feu s'il y en a un
    t_light *light = &main->mapt[tile_pos->y][tile_pos->x].light;
    int light_state = -1;
    if (light->enabled){
        light_state = light->state;
    }

    // On check si la voiture peut s'engager et reserver un chemin. Là ca devient plus compliqué..
    // Tout depend d'ou la voiture se trouve et ou elle veut aller
    // 4 positions possibles

    // {y, x}
    const char SToN[4][2]   = {{2, 3}, {1, 3}, {0, 3},                  {-1, -1}};
    const char StoE[3][2]   = {{2, 3}, {2, 4},                          {-1, -1}};
    const char SToW[6][2]   = {{2, 3}, {1, 3}, {1, 2}, {1, 1}, {1, 0},  {-1, -1}};

    const char NToS[4][2]   = {{1, 1}, {2, 1}, {3, 1},                  {-1, -1}};
    const char NtoW[3][2]   = {{1, 1}, {1, 0},                          {-1, -1}};
    const char NToE[6][2]   = {{1, 1}, {2, 1}, {2, 2}, {2, 3}, {2, 4},  {-1, -1}};

    const char WtoS[3][2]   = {{2, 1}, {3, 1},                          {-1, -1}};
    const char WToE[5][2]   = {{2, 1}, {2, 2}, {2, 3}, {2, 4},          {-1, -1}};
    const char WToN[6][2]   = {{2, 1}, {2, 2}, {2, 3}, {1, 3}, {0, 3},  {-1, -1}};

    const char EtoN[3][2]   = {{1, 3}, {0, 3},                          {-1, -1}};
    const char EToW[5][2]   = {{1, 3}, {1, 2}, {1, 1}, {1, 0},          {-1, -1}};
    const char EToS[6][2]   = {{1, 3}, {1, 2}, {1, 1}, {2, 1}, {3, 1},  {-1, -1}};

    const char (*tiles_to_check)[2] = NULL;

    if (x_in_tile == 3 && y_in_tile == 3){      // Entrée SUD,  3 cas possibles

        if (light_state != -1 && light_state != 1){
            return false;
        }
        if (car->next_tile_dir == 'N'){       
            tiles_to_check = SToN;
        } else if (car->next_tile_dir == 'E'){
            tiles_to_check = StoE;
        } else if (car->next_tile_dir == 'W'){
            tiles_to_check = SToW;
        }

    } else if (x_in_tile == 1 && y_in_tile == 0){   // Entrée NORD, 3 cas possibles

        if (light_state != -1 && light_state != 1){
            return false;
        }

        if (car->next_tile_dir == 'S'){   
            tiles_to_check = NToS;
        } else if (car->next_tile_dir == 'W'){ 
            tiles_to_check = NtoW;
        } else if (car->next_tile_dir == 'E'){
            tiles_to_check = NToE;
        }

    } else if (x_in_tile == 0 && y_in_tile == 2){   // Entrée OUEST,3 cas possibles

        if (light_state != -1 && light_state != 4){
            return false;
        }

        if (car->next_tile_dir == 'S'){        
            tiles_to_check = WtoS;
        } else if (car->next_tile_dir == 'N'){  
            tiles_to_check = WToN;
        } else if (car->next_tile_dir == 'E'){ 
            tiles_to_check = WToE;
        }

    } else if (x_in_tile == 4 && y_in_tile == 1){   // Entrée EST,  3 cas possibles

        if (light_state != -1 && light_state != 4){
            return false;
        }

        if (car->next_tile_dir == 'N'){        
            tiles_to_check = EtoN;
        } else if (car->next_tile_dir == 'S'){  
            tiles_to_check = EToS;
        } else if (car->next_tile_dir == 'W'){ 
            tiles_to_check = EToW;
        }

    } 

    // Juste au cas ou il y a un probleme
    if (tiles_to_check == NULL){
        return true;
    }
 
    // On check si les cases sont libres
    int i = 0;
    while (tiles_to_check[i][0] != -1){
        if (main->smap[tile_y + tiles_to_check[i][0]][tile_x + tiles_to_check[i][1]].state != 0){
            return false;
        }
        i++;
    }

    // Ok c'est libre, reservation des cases
    i = 0;
    while (tiles_to_check[i][0] != -1){
        main->smap[tile_y + tiles_to_check[i][0]][tile_x + tiles_to_check[i][1]].state = 2;
        i++;
    }

    return true;
}

int move_car(t_main *main, t_cars *car){
    int can_move = 0;
    char dir = car->dir;
    t_path *path = &main->paths[car->origin][car->dest][car->path_index];
    t_pos tile_pos = path->steps[car->index_in_path];
    int tile = main->mapt[tile_pos.y][tile_pos.x].type - 1;

    int x_tile, y_tile;
    mapToMousePos(tile_pos.x, tile_pos.y, &x_tile, &y_tile);

    car->new_x = car->x;
    car->new_y = car->y;
    char car_new_dir = dir;

    bool changed_tile = false;

    // Calcul de la nouvelle position
    switch (dir){
        case 'E':
            car->new_x++;
            break;
        case 'W':
            car->new_x--;
            break;
        case 'N':
            car->new_y--;
            break;
        case 'S':
            car->new_y++;
            break;
    }

    // On check si on a changé de tile, ou si on change de direction (crossroad)
    switch (dir){
        case 'E': case 'W':
            if (tile == 1){ // Horizontal
                if (car->new_x == car->next_tile_x){
                    changed_tile = true;
                }
                can_move = 1;
            } else if (tile == 2){  // Crossroad
                if (car->new_x == car->next_tile_x){    // Car must change direction, must go up or down now
                    if (car->new_y == car->next_tile_y){
                        changed_tile = true;
                    } else if (car->new_y < car->next_tile_y){
                        car_new_dir = 'S';
                    } else {
                        car_new_dir = 'N';
                    }
                }
                can_move = 1;
            }
            break;
        case 'N': case 'S':
            if (tile == 0){ // Vertical
                if (car->new_y == car->next_tile_y){
                    changed_tile = true;
                }
                can_move = 1;
            } else if (tile == 2){  // Crossroad
                if (car->new_y == car->next_tile_y){    // Car must change direction, must go left or right now
                    if (car->new_x == car->next_tile_x){
                        changed_tile = true;
                    } else if (car->new_x < car->next_tile_x){
                        car_new_dir = 'E';
                    } else {
                        car_new_dir = 'W';
                    }
                }
                can_move = 1;
            }
            break;
    }

    // On check si on peut avancer
    if (tile == 2){ // Crossroad
        // This also checks for traffic lights
        if (check_crossroad(main, car, x_tile, y_tile, &tile_pos) == 0) return 0;
    } else {
        if (check_path_free(main, car) == 0) return 0;
    }

    if (changed_tile){

        if (car->travel_dir == 1){  

                            // Going to the destination
            if (car->index_in_path + 1 < path->length - 1){
                car->index_in_path++;
                car->next_tile_dir = get_next_dir(&path->steps[car->index_in_path], &path->steps[car->index_in_path + 1]);
                car_update_tile(main, car);
                get_next_tile_entrance_coord(main, car);

            } else {
                            // We are at the destination
                if (free_room_in_dest(main, car->dest) == false){
                    return 0;
                }
                car->index_in_path++;
                prtxy(car->x, car->y, " ");
                main->smap[car->y][car->x].state = 0;
                main->nb_cars_out--;
                car->frames_at_dest = main->game_frame;
                car->state = CAR_AT_DEST;
                path->dest->nb_cars_in++;
                // prt_nb_cars_dest(main, car->dest);
                print_dest_infos(main, &main->dests[car->dest]);
                main->player.cash += CASH_PER_CAR;
                main->player.score += SCORE_PER_CAR;
                can_move = 0;
            }

        } else {            // Going back to the origin
            car->index_in_path--;
            if (car->index_in_path > 0){
                car->next_tile_dir = get_next_dir(&path->steps[car->index_in_path], &path->steps[car->index_in_path - 1]);
                car_update_tile(main, car);
                get_next_tile_entrance_coord(main, car);
            } else {
                // Update origin cooldown
                t_elem *car_orig = &main->origs[car->origin];
                stop_cars();
                for(int i = 0; i < NB_CARS_PER_ORIG; i++){
                    if (car_orig->cars_cooldown[i] == -1){
                        car_orig->cars_cooldown[i] = ORIG_CAR_COOLDOWN;
                        break;
                    }
                }
                // We are at the origin
                remove_car(main, car);
                can_move = 0;
            }
        }
    }

    car->dir = car_new_dir;

    return can_move;
}

void draw_car(t_cars *car){
    char *unicode = 0;
    if (car->can_move){
        if (car->travel_dir == 1){
            if (car->dir == 'N'){
                unicode = "⮝";
            } else if (car->dir == 'S'){
                unicode = "⮟";
            } else if (car->dir == 'E'){
                unicode = "⮞";
            } else if (car->dir == 'W'){
                unicode = "⮜";
            }
        } else {
            if (car->dir == 'N'){
                unicode = "⮙";
            } else if (car->dir == 'S'){
                unicode = "⮛";
            } else if (car->dir == 'E'){
                unicode = "⮚";
            } else if (car->dir == 'W'){
                unicode = "⮘";
            }
        }
    } else {
        if (car->dir == 'N'){
            unicode = "△";
        } else if (car->dir == 'S'){
            unicode = "▽";
        } else if (car->dir == 'E'){
            unicode = "▷";
        } else if (car->dir == 'W'){
            unicode = "◁";
        }
    }
    prtxy(car->x, car->y, "%s%s%s", colors[car->color], unicode, RESET);
}

void update_cars(t_main *main){
    
    if (disable_new_cars == false){
        get_new_car(main);
    }

    for(int c = 0; c < MAX_CARS; c++){
        if (main->cars[c].state){
            t_cars *car = &main->cars[c];

            // Check if the car is at dest and can go back
            if (car->state == CAR_AT_DEST){
                if (main->game_frame - car->frames_at_dest > car->stay_at_dest_time){
                    car->travel_dir = -1;
                    car->index_in_path = main->paths[car->origin][car->dest][car->path_index].length - 2;
                    
                    if (place_return_car(main, c)){
                        // prt_nb_cars_dest(main, car->dest);
                        print_dest_infos(main, &main->dests[car->dest]);
                    }
                }
            }

            // Check if it can move
            if ((car->can_move = move_car(main, &main->cars[c]))){

                // Clear the old position
                prtxy(car->x, car->y, " ");
                main->smap[car->y][car->x].state = 0;

                // Update the position
                car->x = car->new_x;
                car->y = car->new_y;
                main->smap[car->y][car->x].state = 1;

                
            }
            // Draw the car (if it's not at the origin or destination)
            if (car->index_in_path > 0 && car->index_in_path < main->paths[car->origin][car->dest][car->path_index].length - 1){
                draw_car(car);
            }
        }

    }
    dprtxy(180, 1, "Frame: %d", main->game_frame);
    fflush(stdout);
}