#include "main.h"

#define TILE_COLOR      BOLD_CYAN
#define OVER_COLOR      BOLD_HI_YELLOW
#define SELECTED_COLOR  BOLD_HI_GREEN

#define PLACE_BTN       1
#define DESELECT_BTN    2
#define REMOVE_BTN      3

#define T_LIGHT_G   🟢
#define T_LIGHT_Y   🟡
#define T_LIGHT_R   🔴

void stop_ui(){

}

// Enable or disable tiles
char tiles_en[NB_TILES] = {
    1,  // Vertical
    1,  // Horizontal
    1,  // Crossroad
    1,  // Origins
    1,  // Destinations
    1   // Traffic light
};

const char *tiles[NB_TILES][10] = {
    {   "║ ¦ ║",
        "║ ¦ ║",
        "║ ¦ ║",
        "║ ¦ ║"    
    },
    {   "═════",
        "     ",
        "     ",
        "═════"
    },
    {   "╝ | ╚",
        "     ",
        "     ",
        "╗ | ╔"
    },
    {   "╝ Á ╚",
        " / \\ ",
        " │ │ ",
        "╗¯¯¯╔"
    },
    {   "╝╔╔╔╚",
        "┌╩╩╩┐",
        "░░░░░",
        "╗   ╔"
    },
    {   "🚦 🚥",
        "     ",
        "     ",
        "🚥 🚦"
    }
};

const char *tile_dest[5] = {
    "╝╔╔╔╚════╗",
    "┌╩╩╩┐ ___║",
    "░░░░░....║",
    "╗   ╔════╝"
};

const char *dest_bars[DEST_NB_BARS] = {
    "‗__",
    "▄__",
    "█__",
    "█‗_",
    "█▄_",
    "██_",
    "██‗",
    "██▄",
    "███",
};

static const char *days[7] = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

void draw_tile_map(t_main *main, int x, int y, int type, int color);

void map_add_tile(t_main *main, int x, int y, int type){
    if (x < 0 || x >= MAX_W || y < 0 || y >= MAX_H){
        return;
    }
    main->mapt[y][x].type = type + 1;
    if (type == TILE_DEST){
        main->mapt[y][x + 1].type = TILE_DEST + 11;
    }
}

void draw_tile_ui(int ind, char *color){
    if (ind < 0 || ind >= NB_TILES) {
        return;
    }
    if (tiles[ind][0] == NULL) {
        return;
    }
    int y = ind * (TILE_H + 1);
    for (int j = 0; j < TILE_H; j++) {
        if (tiles[ind][j] == NULL) {
            break;
        }
        if (tiles_en[ind] == 0){
            prtxy(1, Y_TOP + y + j, "%s%s%s", colors[C_B_BLACK], tiles[ind][j], RESET);
        } else {
            prtxy(1, Y_TOP + y + j, "%s%s%s", color, tiles[ind][j], RESET);
        }
    }
}

void ui_over(t_main *main) {
    int tile_over = -1;
    int tile_selected = -1;
    if (main->mouse_x < TILE_W + 1){
        tile_over = (main->mouse_y - Y_TOP) / (TILE_H + 1);
        if (tile_over < NB_TILES) {
            // Clear previous over
            if (main->ui.tile_over != tile_over && main->ui.tile_over != -1) {
                draw_tile_ui(main->ui.tile_over, TILE_COLOR);
            }

            if (main->mouse_btn == 1) { // Select tile
                if (tiles_en[tile_over] == 0){
                    return;
                }
                tile_selected = tile_over;

                // Clear previous selected
                if (main->ui.tile_selected != -1 && main->ui.tile_selected != tile_selected) {
                    // Clear menu selection
                    draw_tile_ui(main->ui.tile_selected, TILE_COLOR);
                    // Clear map selection
                    int x = main->ui.mouse_map.x;
                    int y = main->ui.mouse_map.y;
                    draw_tile_map(main, x, y, main->mapt[y][x].type - 1, -1);
                    if (main->ui.tile_selected == TILE_DEST){
                        draw_tile_map(main, x + 1, y, main->mapt[y][x + 1].type - 1, -1);
                    }
                }
                draw_tile_ui(tile_over, SELECTED_COLOR);
                main->ui.tile_selected = tile_selected;
                main->ui.tile_over = -1;

            } else if (main->ui.tile_selected != tile_over) {
                main->ui.tile_over = tile_over;
                draw_tile_ui(tile_over, OVER_COLOR);
            }
        }
    } else if (main->ui.tile_over != -1) {  // Clear over
        draw_tile_ui(main->ui.tile_over, TILE_COLOR);
        main->ui.tile_over = -1;
    }
}

void mousePosToMap(int mx, int my, int *x, int *y) {
    *x = (mx - TILE_W - 2) / TILE_W;
    *y = (my - Y_TOP) / TILE_H;
}

void mapToMousePos(int x, int y, int *mx, int *my) {
    *mx = x * TILE_W + TILE_W + 2;
    *my = y * TILE_H + Y_TOP;
}

void draw_light(int x, int y){
    int mx, my;
    mapToMousePos(x, y, &mx, &my);
    prtxy(mx + 4, my,       "%s⬤", RED);
    prtxy(mx, my + 3,       "⬤");
    prtxy(mx, my,           "%s⬤", GREEN);
    prtxy(mx + 4, my + 3,   "⬤%s", RESET);
}

void clear_tile_map(int x, int y) {
    int mx, my;
    mapToMousePos(x, y, &mx, &my);
    for (int j = 0; j < TILE_H; j++) {
        prtxy(mx, my + j, "     ");
    }
}

bool crossroad_can_connect(t_main *main, int x, int y, char location){

    if (location == 'N'){
        if (y > 0 && main->mapt[y - 1][x].type != 0 && 
                main->mapt[y - 1][x].type != TILE_HORI + 1 && main->mapt[y - 1][x].type != TILE_DEST + 11){
            return true;
        }
    } else if (location == 'S'){
        if (y < MAX_H - 1 && main->mapt[y + 1][x].type != 0 && 
                main->mapt[y + 1][x].type != TILE_HORI + 1 && main->mapt[y + 1][x].type != TILE_DEST + 11){
            return true;
        }
    } else if (location == 'W'){
        if (x > 0 && main->mapt[y][x - 1].type != 0 && 
                main->mapt[y][x - 1].type != TILE_VERT + 1 && main->mapt[y][x - 1].type != TILE_DEST + 11){
            return true;
        }
    } else if (location == 'E'){
        if (x < MAX_W - 1 && main->mapt[y][x + 1].type != 0 && 
                main->mapt[y][x + 1].type != TILE_VERT + 1 && main->mapt[y][x + 1].type != TILE_DEST + 11){
            return true;
        }
    }
    return false;
}



void print_dest_infos(t_main *main, t_elem *dest){
    (void)main;
    int mx, my;
    mapToMousePos(dest->pos.x, dest->pos.y, &mx, &my);

    // Time left (bar graph)
    if (dest->nb_bars < 3){
        printf("%s", B_GREEN);
    } else if (dest->nb_bars < 6){
        printf("%s", B_YELLOW);
    } else {
        printf("%s", B_RED);
    }
    prtxy(mx + 6, my + 1, dest_bars[dest->nb_bars]);
    printf("%s", RESET);

    // Cars in    
    // if (main->game_mode == 0){
    prtxy(mx + 5, my + 2, "    ");
    prtxy(mx + 5, my + 2, "%.*s", dest->nb_cars_in, "vvvv");
    if (dest->nb_cars_in > 4){
        prtxy(mx + 5, my + 2, "%.*s", dest->nb_cars_in - 4, "VVVV");
    }
        
    // } else {
    //     char cars_info[5] = {0};
    //     memset(cars_info, ' ', 4);
    //     for(int i = 0; i < NB_CARS_PER_DEST; i++){
    //         if (dest->cars_requests[i]){
    //             if (dest->cars_requests[i]->state == CAR_AT_DEST){
    //                 cars_info[i] = 'v';
    //             } else {
    //                 cars_info[i] = '.';
    //             }
    //         }
    //     }
    //     prtxy(mx + 5, my + 2, "%s", cars_info);
    // }
}

void draw_tile_map(t_main *main, int x, int y, int type, int color) {
    if (type >= NB_TILES) {
        return;
    }

    if (type < 0){
        clear_tile_map(x, y);
        return;
    }

    int mx, my;
    mapToMousePos(x, y, &mx, &my);
    
    if (color == -1){
        color = C_B_CYAN; // Cyan

        // Will be removed ?
        if (main->mapt[y][x].action == 'r') {
            color = C_B_BLACK;

        } else {
            if (type == TILE_ORIG){
                t_elem *elem = elem_get_by_pos(main->origs, x, y);
                if (elem) color = elem->color;
            } else if (type == TILE_DEST){
                t_elem *elem = elem_get_by_pos(main->dests, x, y);
                if (elem) color = elem->color;
            }
        }
    }

    if (type == TILE_DEST){
        // More work to do for dests (2 tiles + infos)
        for (int j = 0; j < TILE_H; j++) {
            if (tiles[type][j] == NULL) {
                break;
            }
            prtxy(mx, my + j, "%s%s%s", colors[color], tile_dest[j], RESET);
        }


    } else {
        for (int j = 0; j < TILE_H; j++) {
            if (tiles[type][j] == NULL) {
                break;
            }
            prtxy(mx, my + j, "%s%s%s", colors[color], tiles[type][j], RESET);
        }
    }
    
    if (type == TILE_CROS){     // Drawing crossroads depends of connected roads
        char roads[4] = {0, 0, 0, 0};   // W, E, N, S
        char nb_roads = 0;

        if (crossroad_can_connect(main, x, y, 'W')) {roads[0] = 1; nb_roads++;}
        if (crossroad_can_connect(main, x, y, 'E')) {roads[1] = 1; nb_roads++;}
        if (crossroad_can_connect(main, x, y, 'N')) {roads[2] = 1; nb_roads++;}
        if (crossroad_can_connect(main, x, y, 'S')) {roads[3] = 1; nb_roads++;}

        if (nb_roads > 1){  // We only close crossroad if at least 2 roads connected
            // We draw in blue if not overed and not marked for removal
            if (main->mapt[y][x].action != 'r' && color != C_B_RED) color = C_B_BLUE;

            prtxy(1, 1, "%s", colors[color]);

            if (roads[2] == 0){
                prtxy(mx, my, "═════");
            }
            if (roads[3] == 0){
                prtxy(mx, my + 3, "═════");
            }
            if (roads[0] == 0 || roads[1] == 0){
                for(int i = 0; i < TILE_H; i++){
                    if (roads[0] == 0){
                        prtxy(mx, my + i, "║");
                    }
                    if (roads[1] == 0){
                        prtxy(mx + 4, my + i, "║");
                    }
                }
            }

            // Corners ? Only if 2 roads connected
            if (nb_roads == 2){
                if (roads[0] == 0 && roads[2] == 0){
                    prtxy(mx, my, "╔");
                }
                if (roads[0] == 0 && roads[3] == 0){
                    prtxy(mx, my + 3, "╚");
                }
                if (roads[1] == 0 && roads[2] == 0){
                    prtxy(mx + 4, my, "╗");
                }
                if (roads[1] == 0 && roads[3] == 0){
                    prtxy(mx + 4, my + 3, "╝");
                }
            }

            prtxy(1, 1, "%s", RESET);
        }
    }

    if (main->mapt[y][x].light.enabled){
        draw_light(x, y);
    }
}







bool remove_origin(t_main *main, int x, int y){
    bool can_remove = false;

    t_elem *orig = elem_get_by_pos(main->origs, x, y);
    if (orig && orig->active){
        can_remove = cancel_all_path_on_tile(main, x, y);
        if (can_remove){
            elem_remove_by_index(main->origs, orig->index);
            main->origs[orig->index].active = 0;
            main->nb_origs -= 1;
        }
    }

    return can_remove;
}

bool remove_dest(t_main *main, int x, int y){
    bool can_remove = false;

    t_elem *dest = elem_get_by_pos(main->dests, x, y);
    if (dest && dest->active){
        can_remove = cancel_all_path_on_tile(main, x, y);
        if (can_remove){
            elem_remove_by_index(main->dests, dest->index);
            main->dests[dest->index].active = 0;
            main->nb_dests--;
        }
    }

    return can_remove;
}

void update_crossroads(t_main *main, int x, int y){
    // We check if we should update a crossroad for the visual closing
    if (main->mapt[y][x].type - 1 == TILE_CROS)     draw_tile_map(main, x, y, TILE_CROS, -1);
    if (y > 1 && main->mapt[y - 1][x].type - 1 == TILE_CROS) draw_tile_map(main, x, y - 1, TILE_CROS, -1);
    if (main->mapt[y + 1][x].type - 1 == TILE_CROS) draw_tile_map(main, x, y + 1, TILE_CROS, -1);
    if (x > 1 && main->mapt[y][x - 1].type - 1 == TILE_CROS) draw_tile_map(main, x - 1, y, TILE_CROS, -1);
    if (main->mapt[y][x + 1].type - 1 == TILE_CROS) draw_tile_map(main, x + 1, y, TILE_CROS, -1);
}

void refund_player(t_main *main, int type){
    if (main->game_mode == 1){
        if (type == TILE_HORI || type == TILE_VERT){
            main->player.cash += CASH_PER_ROAD;
        } else if (type == TILE_CROS){
            main->player.cash += CASH_PER_CROSS;
        }
    }
}

void place_tile(t_main *main) {
    if (main->mouse_x < TILE_W + 1) return;

    t_ui *ui = &main->ui;

    // Check the mouse is on the map
    int x, y;
    mousePosToMap(main->mouse_x, main->mouse_y, &x, &y);
    if (x < 0 || y < 0 || x >= MAX_W || y >= MAX_H) return;

    ui->mouse_map.x = x;
    ui->mouse_map.y = y;

    // Place tile or remove tile
    if (main->mouse_btn == PLACE_BTN){              // PLACE TILE

        // Check if tile is empty
        if (main->mapt[y][x].type == 0) {             
            
            int index = 0;

            if (ui->tile_selected == TILE_HORI || ui->tile_selected == TILE_VERT){ // Horizontal or Vertical
                    // Player has cash ?
                if (main->game_mode == 1){
                    if (main->player.cash >= CASH_PER_ROAD || main->god_mode){
                        main->player.cash -= CASH_PER_ROAD;
                    } else {
                        index = -1;
                    }
                }
            } else if (ui->tile_selected == TILE_CROS){ // Crossroad
                    // Player has cash ?
                if (main->game_mode == 1){
                    if (main->player.cash >= CASH_PER_CROSS || main->god_mode){
                        main->player.cash -= CASH_PER_CROSS;
                    } else {
                        index = -1;
                    }
                }

            } else if (ui->tile_selected == TILE_ORIG){    // Origin
                index = orig_add_by_pos(main, x, y, C_CYAN);

            } else if (ui->tile_selected == TILE_DEST){ // Destination
                if (main->mapt[y][x + 1].type == 0){
                    index = dest_add_by_pos(main, x, y, dests_color_index[rand() % NB_DEST_COLORS]);
                } else {
                    index = -1;
                }

            } else if (ui->tile_selected == TILE_LIGHT){ // Traffic light
                index = -1;     // Traffic light must be placed on a crossroad
            }

            if (index != -1){
                map_add_tile(main, x, y, ui->tile_selected);
                draw_tile_map(main, x, y, ui->tile_selected, -1); // Draw new tile
                update_crossroads(main, x, y);
                main->need_path_update = true;
            }
            

        // TRAFFIC LIGHTS
        } else if (main->mapt[y][x].type - 1 == TILE_CROS){

            // TRAFFIC LIGHTS Must be placed on a crossroad
            if (ui->tile_selected == TILE_LIGHT){ // Traffic light
                main->mapt[y][x].light.enabled = true;
                main->mapt[y][x].light.state = 0;
                main->mapt[y][x].light.elapsed = 0;
                add_light_to_list(&main->lights, &main->mapt[y][x].light);
                    // draw_tile_map(main, x, y, ui->tile_selected, -1); // Draw new tile
            }
        }

    } else if (main->mouse_btn == REMOVE_BTN) {     // REMOVE TILE
        int type = main->mapt[y][x].type - 1;
        bool can_remove = false;

        // Remove traffic light if exists
        if (main->mapt[y][x].light.enabled){
            main->mapt[y][x].light.enabled = false;
            rm_light_from_list(&main->lights, &main->mapt[y][x].light);

        } else {
            if (main->mapt[y][x].action == 0){ // Check that no other action is being performed on the tile

                if (type == TILE_ORIG) {            // ORIGINS
                    if (main->game_mode == 0) can_remove = remove_origin(main, x, y);

                } else if (type == TILE_DEST) {     // DESTINATIONS
                    if (main->game_mode == 0) can_remove = remove_dest(main, x, y);         

                } else if (type >= 0){        // ROADS
                    can_remove = cancel_all_path_on_tile(main, x, y);
                }

                if (type != -1 && can_remove && main->mapt[y][x].action == 0) {
                    // Maybe refund player ?
                    refund_player(main, type);
                    main->mapt[y][x].type = 0;
                    clear_tile_map(x, y);
                    update_crossroads(main, x, y);
                    main->need_path_update = true;
                    dprtxy(1, 52, "Removed tile at %d, %d", x, y);
                } else {
                    dprtxy(1, 52, "Can't remove tile at %d, %d", x, y);
                }
            }
        }

    } else if (main->mouse_btn == DESELECT_BTN) {   // DESELECT TILE
        draw_tile_ui(ui->tile_selected, TILE_COLOR);
        draw_tile_map(main, x, y, main->mapt[y][x].type - 1, -1);
        ui->tile_selected = -1;
    }

    // Update previous tile. V this check only when starting program V
    if (ui->prev_mouse_map.x != -1 && ui->prev_mouse_map.y != -1) {

        // The mouse has moved to an other tile
        if (ui->prev_mouse_map.x != x || ui->prev_mouse_map.y != y) {

            // We redraw the tile on the map that was hovered before
            draw_tile_map(main, ui->prev_mouse_map.x, 
                                ui->prev_mouse_map.y, 
                                main->mapt[ui->prev_mouse_map.y][ui->prev_mouse_map.x].type - 1, -1);
            // if we are placing a dest, we need to draw the second tile
            if (ui->tile_selected == TILE_DEST){
                draw_tile_map(main, ui->prev_mouse_map.x + 1, 
                                    ui->prev_mouse_map.y, 
                                    main->mapt[ui->prev_mouse_map.y][ui->prev_mouse_map.x + 1].type - 1, -1);
            }
            
            // At the actual mouse position
            if (main->mapt[y][x].type == 0 || (ui->tile_selected == TILE_LIGHT && main->mapt[y][x].type - 1 == TILE_CROS)){
                // We draw the selected tile to be placed
                if (main->mouse_btn == 0){  
                    // If we are in game mode, we check the cash to change color
                    if (main->game_mode == 1){
                        int cash_needed = 0;
                        if (ui->tile_selected == TILE_HORI || ui->tile_selected == TILE_VERT){
                            cash_needed = CASH_PER_ROAD;
                        } else if (ui->tile_selected == TILE_CROS){
                            cash_needed = CASH_PER_CROSS;
                        }
                        if (main->player.cash < cash_needed){
                            draw_tile_map(main, x, y, ui->tile_selected, C_B_MAGENTA);
                        } else {
                            draw_tile_map(main, x, y, ui->tile_selected, C_B_WHITE);
                        }

                    // Sandbox, we dont care about cash
                    } else {
                        draw_tile_map(main, x, y, ui->tile_selected, C_B_WHITE);
                    }
                    
                }
                
            } else {
                // If we are over a placed tile, we draw it in red
                draw_tile_map(main, x, y, main->mapt[y][x].type - 1, C_B_RED);
            }
        }
    }

    ui->prev_mouse_map.x = x;
    ui->prev_mouse_map.y = y;
}

void check_tiles_to_be_removed(t_main *main) {
    int nb_tiles = 0;


    t_pos *tile = main->tiles_to_be_removed;
    
    if (tile) dprtxy(150, 20, "Tiles to be removed:");
    
    while (tile != NULL) {
        int x = tile->x;
        int y = tile->y;

        nb_tiles++;

        // get the next tile to be removed
        tile = tile->next;
        
        int tile_type = main->mapt[y][x].type - 1;

        dprtxy(150, 20 + nb_tiles, "Tile %d, %d   ", x, y);

        bool can_remove = false;

        if (tile_type == TILE_ORIG){            // Origin
            can_remove = remove_origin(main, x, y);
        } else if (tile_type == TILE_DEST){     // Destination
            can_remove = remove_dest(main, x, y);
        } else if (tile_type >= 0){     // Road
            can_remove = cancel_all_path_on_tile(main, x, y);
        }

        if (can_remove == false) {
            dprtxy(165, 20 + nb_tiles, "LOCKED    ");    
            continue;
        }
        dprtxy(165, 20 + nb_tiles, "REMOVING ");

        // Maybe refund player ?
        refund_player(main, tile_type);

        // Remove the tile from the list to be removed
        remove_tile_from_remove_list(main, x, y);
        // Remove the tile
        main->mapt[y][x].type = 0;
        clear_tile_map(x, y);
        // Clear over map
        main->mapt[y][x].action = 0;

        main->need_path_update = true;
        
    }
    if (main->tiles_to_be_removed || main->need_path_update){
        while (nb_tiles < 30){
            dprtxy(150, 21 + nb_tiles, "                                       ");
            nb_tiles++;
        }
    }
}

// Mouse wheel changes the selected tile
void ui_mouse_wheel(t_main *main){
    int prev_selected = main->ui.tile_selected;

    if (main->mouse_btn == 5){
        do {
            main->ui.tile_selected++;
            if (main->ui.tile_selected >= NB_TILES){
                main->ui.tile_selected = 0;
            }
        } while (tiles_en[main->ui.tile_selected] == 0);
    } else if (main->mouse_btn == 4){
        do {
            main->ui.tile_selected--;
            if (main->ui.tile_selected < 0){
                main->ui.tile_selected = NB_TILES - 1;
            }
        } while (tiles_en[main->ui.tile_selected] == 0);
    }
    // Update changes
    if (prev_selected != main->ui.tile_selected){
        draw_tile_ui(prev_selected, TILE_COLOR);
        draw_tile_ui(main->ui.tile_selected, SELECTED_COLOR);
        draw_tile_map(main, main->ui.mouse_map.x, main->ui.mouse_map.y, main->ui.tile_selected, C_B_WHITE);
        if (prev_selected == TILE_DEST){
            draw_tile_map(main, main->ui.mouse_map.x + 1, main->ui.mouse_map.y, 
                main->mapt[main->ui.mouse_map.y][main->ui.mouse_map.x + 1].type - 1, -1);
        }
    }
}

void ui_manage_mouse(t_main *main) {
    ui_mouse_wheel(main);
    ui_over(main);
    place_tile(main);
}

void ui_draw_new_tile(t_main *main, int x, int y){
    draw_tile_map(main, x, y, main->mapt[y][x].type - 1, -1);
}

void draw_frame(t_main *main){
    int color = C_B_YELLOW;

    if (main->logic.warning_will_loose){
        if (main->game_frame % 3 == 0){
            color = C_B_RED;
        }
    }

    int x_left = main->screen_w / 2 - 22;
    prtxy(x_left, 1,      "%s╔%s╔═══════════════════════════════════════╗%s╗", YELLOW, colors[color], YELLOW);
    prtxy(x_left, 2,      "%s║%s║                                       ║%s║", YELLOW, colors[color], YELLOW); 
    prtxy(x_left, 3,      "%s║%s║                                       ║%s║", YELLOW, colors[color], YELLOW); 
    prtxy(x_left, 4,      "%s║%s║              ╔═       ═╗              ║%s║", YELLOW, colors[color], YELLOW); 
    prtxy(x_left - 1, 5, " %s╚%s╚══════════════╩═════════╩══════════════╝%s╝ ", YELLOW, colors[color], YELLOW);

    main->logic.warning_will_loose_prev = main->logic.warning_will_loose;
}

void draw_rotating_week_clock(t_main *main){
    int pos_time = main->logic.minute + main->logic.hour * 60 + (main->logic.day % 7) * 1440;
    pos_time = (pos_time * 88) / 10080;

    printf("%s", RED);
    
    int x_left = main->screen_w / 2 - 22;
    x_left++;
    for(int i = 0; i < 2; i++){
        if (pos_time == 0 || pos_time == -1){
            prtxy(x_left,            1, "╔");
        } else if (pos_time < 40){
            prtxy(x_left + pos_time, 1, "═");
        } else if (pos_time == 40){
            prtxy(x_left + 40,       1, "╗");
        } else if (pos_time < 44){
            prtxy(x_left + 40, pos_time - 39, "║");
        } else if (pos_time == 44){
            prtxy(x_left + 40, 5, "╝");
        } else if (pos_time < 84){
            prtxy(x_left + 84 - pos_time, 5, "═");
        } else if (pos_time == 84){
            prtxy(x_left, 5, "╚");
        } else if (pos_time < 91){
            prtxy(x_left, 89 - pos_time, "║");
        }
        
        printf("%s", B_YELLOW);
        pos_time--;
    }
}

void update_ui(t_main *main) {
    uint64_t now = millis();

    check_tiles_to_be_removed(main);

    if (now - main->last_update_ui < UI_UPDATE_DELAY) {
        return;
    }

    // Positions
    if (prt_debug){
        prtxy(4, 1, "Mouse | x: %d, y: %d, btn: %d      ", main->mouse_x, main->mouse_y, main->mouse_btn);
        if (main->ui.mouse_map.x > 0 && main->ui.mouse_map.y > 0)
        prtxy(4, 2, "Map   | x: %d, y: %d, tile: %d      ", main->ui.mouse_map.x, main->ui.mouse_map.y, main->mapt[main->ui.mouse_map.y][main->ui.mouse_map.x].type);
    } else {
        prtxy(4, 1, "%sMouse Left  : %sPlace       ", B_YELLOW, YELLOW);
        prtxy(4, 2, "%sMouse Right : %sRemove      ", B_YELLOW, YELLOW);
        prtxy(4, 3, "%sMouse Wheel : %sDeselect    ", B_YELLOW, YELLOW);
        prtxy(4, 4, "%sESC: %sExit, %sr: %sReset game  ", B_RED, YELLOW, B_RED, YELLOW);
    }

    prtxy(35, 1, "%s1 2 3 4: %sSpeed ( %s%d%s )  ", B_YELLOW, YELLOW, B_YELLOW, main->speed_index + 1, YELLOW);
    prtxy(35, 2, "%sSpace  : %sPause (%s%s%s)  ", B_YELLOW, YELLOW,(main->paused) ? B_RED: B_GREEN, (main->paused) ? "STOP" : "RUN", YELLOW);
    prtxy(35, 3, "%s+ / -  : %sMusic volume (%s%d%s)  ", B_YELLOW, YELLOW, B_GREEN, main->volume, YELLOW);
    prtxy(35, 4, "%sM      : %sMusic (%s%s%s)  ", B_YELLOW, YELLOW, (main->music) ? B_GREEN : B_RED, (main->music) ? "ON" : "OFF", YELLOW);

    if (main->game_mode == 0){
        prtxy(35, 3, "c    : New cars (%s)  ", (disable_new_cars) ? "OFF" : "ON");
        prtxy(35, 4, "* / Chance new car (%d/%d)  ", main->chance_new_car, MAX_CHANCE_ADD_CAR);

        prtxy(60, 1, "8 : Less cars / house");
        prtxy(60, 2, "9 : More cars / house");
        prtxy(60, 3, "    Actual: %d", main->nb_cars_per_origins);

        prtxy(90, 1, "Origins: %d/%d, Dests: %d/%d       ", main->nb_origs, MAX_ORIGS, main->nb_dests, MAX_DESTS);
        prtxy(90, 2, "Nb cars: %d     ", main->nb_cars_out);

        prtxy(120, 1, "Score: %ld     ", main->player.score);
        prtxy(120, 2, "Cash : %ld $   ", main->player.cash);
        prtxy(120, 4, "FPS: %d    "    , main->fps);

        prtxy(160, 1, "Board: %d x %d", main->board_w, main->board_h);
    
    } else {

        if (main->logic.warning_will_loose || main->logic.warning_will_loose_prev){
            draw_frame(main);
        }

        // #### Rotating frame indicator for the week
        static int prev_week = 0;
        int week = main->logic.day / 7 + 1;
        if (week != prev_week){
            draw_frame(main);
            prev_week = week;
        }
        draw_rotating_week_clock(main);
        
        // #### Informations
        int center = main->screen_w / 2;
        int x_left = center - 22;

        x_left += 2;
        printf("%s", YELLOW);
        prtxy(x_left + 2, 4, "Week # %d", week);
        prtxy(center - 3, 4, "%02d:%02d", main->logic.hour, main->logic.minute);

        char day[16] = {0};
        int len = 11 - strlen(days[main->logic.weekDay]);
        sprintf(day, "%s%*.s", days[main->logic.weekDay], len, "          ");
        day[11] = 0;
        prtxy(center + 8, 4, "%s", day);

        prtxy(x_left + 3, 2, "Score: %ld ", main->player.score);
        prtxy(center + 5, 2, "%s$ %s%ld %s$ ", B_YELLOW, YELLOW, main->player.cash, B_YELLOW);
        prtxy(main->screen_w - 10, 1, "%sFPS: %s%d   ", YELLOW, B_YELLOW, main->fps);

        printf("%s", RESET);
    }

    // INFO BOX
    int tw = -1;
    int th = -1;
    get_terminal_size(&tw, &th);
    const int info_h = 7;
    const int info_w = 20;
    const int info_x = 4;
    const int info_y = th - info_h - 2;

    if (prt_debug){
        if (main->ui.mouse_map.x != main->ui.infos_prev_x || main->ui.mouse_map.y != main->ui.infos_prev_y) {
            main->ui.infos_prev_x = main->ui.mouse_map.x;
            main->ui.infos_prev_y = main->ui.mouse_map.y;

            if (main->ui.mouse_map.x >= 0 && main->ui.mouse_map.y >= 0 && main->ui.mouse_map.x < MAX_W && main->ui.mouse_map.y < MAX_H) {
                int x = main->ui.mouse_map.x;
                int y = main->ui.mouse_map.y;
                char type = main->mapt[y][x].type;
                int nb_paths;
                t_elem *elem;

                // Print a box
                if (type != 0){                         // We must print x3 cause lines are UTF-8 (3 bytes each char)
                    prtxy(info_x, info_y, "┌%.*s┐", (info_w-2) * 3, "─────────────────────────────────────────────────");
                    for(int i = 1; i < info_h - 1; i++){
                        prtxy(info_x, info_y + i, "│%.*s│", (info_w-2), "                                                  ");
                    }
                    prtxy(info_x, info_y + info_h - 1, "└%.*s┘", (info_w-2) * 3, "─────────────────────────────────────────────────");
                }

                switch (type){
                    case 0:     // Empty
                        if (main->ui.info_prev_type != 0){
                            for(int i = 0; i < info_h; i++){
                                prtxy(info_x, info_y + i, "%.*s", info_w, "                                                  ");
                            }
                        }
                        break;
                    
                    case 1:     // Vertical
                    case 2:     // Horizontal
                    case 3:     // Crossroad
                        nb_paths = path_count_in_list(main->mapt[y][x].paths);
                        prtxy(info_x + 2, info_y + 1, "Paths    : %d  ", nb_paths);
                        break;
                    
                    case 4:     // Origin
                        elem = elem_get_by_pos(main->origs, x, y);
                        if (elem == NULL) break;
                        prtxy(info_x + 2, info_y + 1, "Origin Id: %d ", elem->index);
                        prtxy(info_x + 2, info_y + 2, "Active   : %d ", elem->active);
                        prtxy(info_x + 2, info_y + 3, "Paths    : %d  ", elem->nb_paths);
                        prtxy(info_x + 2, info_y + 4, "Cars     : %d/%d ", elem->nb_cars_in, elem->max_cars);
                        break;

                    case 5:     // Destination
                        elem = elem_get_by_pos(main->dests, x, y);
                        if (elem == NULL) break;
                        prtxy(info_x + 2, info_y + 1, "Dest Id  : %d ", elem->index);
                        prtxy(info_x + 2, info_y + 2, "Active   : %d ", elem->active);
                        prtxy(info_x + 2, info_y + 3, "Paths    : %d  ", elem->nb_paths);
                        prtxy(info_x + 2, info_y + 4, "Cars     : %d/%d ", elem->nb_cars_in, elem->max_cars);
                        prtxy(info_x + 2, info_y + 5, "Bars     : %d  ", elem->nb_bars);
                        break;
                }

                main->ui.info_prev_type = type;
            }
        }
    }

    // static int deb_last_map_x = -1;
    // static int deb_last_map_y = -1;
    // static char deb_last_type = 0;
    // const int debug_x = 195;
    // const int debug_y = Y_TOP + 3;
    // const int debug_h = 50;
    // if (prt_debug) {
    //     if (main->ui.mouse_map.x != deb_last_map_x || main->ui.mouse_map.y != deb_last_map_y) {
    //         deb_last_map_x = main->ui.mouse_map.x;
    //         deb_last_map_y = main->ui.mouse_map.y;
            
    //         // Get information about the tile the mouse is over
    //         if (main->ui.mouse_map.x >= 0 && main->ui.mouse_map.y >= 0 && main->ui.mouse_map.x < MAX_W && main->ui.mouse_map.y < MAX_H) {
    //             int x = main->ui.mouse_map.x;
    //             int y = main->ui.mouse_map.y;
    //             char type = main->mapt[y][x].type;
    //             t_elem *elem;

    //             // Print a box
    //             if (type != 0){
    //                 prtxy(debug_x, debug_y, "┌─────────────────────────────────────┐");
    //                 for(int i = 1; i < debug_h - 1; i++){
    //                     prtxy(debug_x, debug_y + i, "│                                     │");
    //                 }
    //                 prtxy(debug_x, debug_y + debug_h - 1, "└─────────────────────────────────────┘");
    //             }

    //             switch (type){
    //                 case 0:     // Empty
    //                     if (deb_last_type != 0){
    //                         for(int i = 0; i < debug_h; i++){
    //                             prtxy(debug_x, debug_y + i, "                                       ");
    //                         }
    //                     }
    //                     break;

    //                 case 1:     // Vertical
    //                 case 2:     // Horizontal
    //                 case 3:     // Crossroad
    //                     prtxy(debug_x + 2, debug_y + 1, "Tile       %s ", (type == 1) ? "Vert  " : (type == 2) ? "Horiz " : "Cross ");
    //                     prtxy(debug_x + 2, debug_y + 2, "X: %d, Y: %d   ", x, y);

    //                     // On recupere les infos de chaque paths de cette tile
    //                     int nb_paths = path_count_in_list(main->mapt[y][x].paths);
    //                     prtxy(debug_x + 2, debug_y + 3, "Nb paths:  %d  ", nb_paths);
    //                     t_tile_paths *tps = main->mapt[y][x].paths;
    //                     int i = 0;
    //                     while (tps){
    //                         if (tps->path){
    //                             prtxy(debug_x + 2, debug_y + 4 + i, "%c%d, %d to %d, len: %d, en: %d, cars: %d", 
    //                                 (main->mapt[y][x].action == 'r') ? 'X' : '#', i, tps->path->orig->index, tps->path->dest->index,
    //                                 tps->path->length, tps->path->active, tps->path->nb_cars_using);
    //                             t_path *old = &main->paths[tps->path->orig->index][tps->path->dest->index][1];
    //                             if (old->steps){
    //                                 prtxy(debug_x + 2, debug_y + 5 + i, "OLD: len %d, cars %d", 
    //                                     old->length, 
    //                                     old->nb_cars_using);
    //                             }
    //                             i++;
    //                         }
    //                         tps = tps->next;
    //                     }
    //                     break;

    //                 case 4:     // Origin
    //                     elem = elem_get_by_pos(main->origs, x, y);
    //                     if (elem == NULL) break;

    //                     break;

    //                 case 5:     // Destination
    //                     elem = elem_get_by_pos(main->dests, x, y);
    //                     if (elem == NULL) break;

    //                     break;
    //             }
    //             deb_last_type = type;

    //         }
    //     }
    // }

    main->last_update_ui = now;
}

void draw_ui(t_main *main) {

    const char *ui_color = BOLD_HI_BLACK;

    printf("%s", ui_color);

    // Draw a line on top
    for (int i = 0; i < main->screen_w; i++) {
        prtxy(i, Y_TOP - 1, "─");
    }
    prtxy(TILE_W + 1, Y_TOP - 1, "┬");
    for(int i =0; i < NB_TILES; ++i){
        int y = Y_TOP + i * (TILE_H + 1);
        if (tiles[i][0] == NULL) {
            break;
        }
        // Draw boxes
        for(int j = 0; j < TILE_H + 1; ++j){
            prtxy(TILE_W + 1, y + j, "│");
        }
        if (i < NB_TILES - 1) {
            prtxy(1, y + TILE_H, "─────┤");
        } else {
            prtxy(1, y + TILE_H, "─────┘");
        }

        // Draw tiles
        draw_tile_ui(i, TILE_COLOR);
        printf("%s", ui_color);
    }

    draw_frame(main);

    // int center = main->screen_w / 2;
    // int x_left = center - 22;
    // prtxy(x_left, 1,      "%s╔%s╔═══════════════════════════════════════╗%s╗", YELLOW, B_YELLOW, YELLOW);
    // prtxy(x_left, 2,      "%s║%s║                                       ║%s║", YELLOW, B_YELLOW, YELLOW); 
    // prtxy(x_left, 3,      "%s║%s║                                       ║%s║", YELLOW, B_YELLOW, YELLOW); 
    // prtxy(x_left, 4,      "%s║%s║              ╔═       ═╗              ║%s║", YELLOW, B_YELLOW, YELLOW); 
    // prtxy(x_left - 1, 5, " %s╚%s╚══════════════╩═════════╩══════════════╝%s╝ ", YELLOW, B_YELLOW, YELLOW);

    printf("%s", RESET);
}

