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

#define NOT_ENOUGH_CASH_COLOR   C_B_MAGENTA
#define REMOVAL_COLOR           C_RED
#define AWAITING_RM_COLOR       C_B_BLACK

void stop_ui(){

}

char h_line[512] = "══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════";
char spaces[512] = "                                                                                                                                ";

// Enable or disable tiles
char tiles_en[NB_TILES] = {
    1,  // Vertical
    1,  // Horizontal
    1,  // Crossroad
    1,  // Traffic light
    1,  // Origins
    1,  // Destinations
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
    {   "🚦 🚥",
        "     ",
        "     ",
        "🚥 🚦"
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
    }
};

const char *tile_dest[5] = {
    "╝╔╔╔╚════╗",
    "┌╩╩╩┐ ___║",
    "░░░░░....║",
    "╗   ╔════╝"
};

int tiles_color[NB_TILES] = {
    C_B_CYAN,
    C_B_CYAN,
    C_B_BLUE,
    C_B_BLUE,
    C_B_CYAN,
    C_B_CYAN,
};

const int tiles_price[NB_TILES] = {
    CASH_PER_ROAD,
    CASH_PER_ROAD,
    CASH_PER_CROSS,
    CASH_PER_ROAD,
    0,
    0,
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

void draw_one_tile(int mx, int my, int type, const char* color){
    printf("%s", color);
    if (type != TILE_DEST){
        for (int j = 0; j < TILE_H; j++) {
            if (tiles[type][j] == NULL) {
                break;
            }
            prtxy(mx, my + j, "%s", tiles[type][j]);
        }
    } else {
        for (int j = 0; j < TILE_H; j++) {
            if (tiles[type][j] == NULL) {
                break;
            }
            prtxy(mx, my + j, "%s", tile_dest[j]);
        }
    }
    printf("%s", RESET);
}

void map_add_tile(t_main *main, int x, int y, int type){
    if (x < 0 || x >= MAX_W || y < 0 || y >= MAX_H){
        return;
    }
    main->mapt[y][x].type = type + 1;
    if (type == TILE_DEST){
        main->mapt[y][x + 1].type = TILE_DEST + 11;
    }
}

void ui_draw_menu_tile(int ind, const char *color){
    if (ind < 0 || ind >= NB_TILES) {
        return;
    }
    if (tiles[ind][0] == NULL) {
        return;
    }
    int y = ind * (TILE_H + 2);
    draw_one_tile(1, Y_TOP + y, ind, color);

}

void ui_mouse_over_menu(t_main *main) {
    int tile_over = -1;
    int tile_selected = -1;
    if (main->mouse_x < TILE_W + 1){
        tile_over = (main->mouse_y - Y_TOP) / (TILE_H + 2);
        
        if (tile_over < NB_TILES) {
            if (tiles_en[tile_over] == 0){
                return;
            }

            // Clear previous over
            if (main->ui.tile_over != tile_over && main->ui.tile_over != -1) {
                ui_draw_menu_tile(main->ui.tile_over, colors[tiles_color[main->ui.tile_over]]);
            }

            if (main->mouse_btn == 1) { // Select tile
                if (tiles_en[tile_over] == 0){
                    return;
                }
                tile_selected = tile_over;

                // Clear previous selected
                if (main->ui.tile_selected != -1 && main->ui.tile_selected != tile_selected) {
                    // Clear menu selection
                    ui_draw_menu_tile(main->ui.tile_selected, colors[tiles_color[main->ui.tile_over]]);
                    // Clear map selection
                    int x = main->ui.mouse_map.x;
                    int y = main->ui.mouse_map.y;
                    draw_tile_map(main, x, y, main->mapt[y][x].type - 1, -1);
                    if (main->ui.tile_selected == TILE_DEST){
                        draw_tile_map(main, x + 1, y, main->mapt[y][x + 1].type - 1, -1);
                    }
                }
                ui_draw_menu_tile(tile_over, SELECTED_COLOR);
                main->ui.tile_selected = tile_selected;
                main->ui.tile_over = -1;

            } else if (main->ui.tile_selected != tile_over) {
                main->ui.tile_over = tile_over;
                ui_draw_menu_tile(tile_over, OVER_COLOR);
            }
        }
    } else if (main->ui.tile_over != -1) {  // Clear over
        ui_draw_menu_tile(main->ui.tile_over, colors[tiles_color[main->ui.tile_over]]);
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


void print_dest_infos(int mx, int my, int nb_bars, int nb_cars){
    // Time left (bar graph)
    if (nb_bars < 3){
        printf("%s", B_GREEN);
    } else if (nb_bars < 6){
        printf("%s", B_YELLOW);
    } else {
        printf("%s", B_RED);
    }
    prtxy(mx + 6, my + 1, dest_bars[nb_bars]);
    printf("%s", RESET);

    // Cars in    
    prtxy(mx + 5, my + 2, "    ");
    prtxy(mx + 5, my + 2, "%.*s", nb_cars, "vvvv");
    if (nb_cars > 4){
        prtxy(mx + 5, my + 2, "%.*s", nb_cars - 4, "VVVV");
    }
}

void update_dest_infos(t_main *main, t_elem *dest){
    (void)main;
    int mx, my;
    mapToMousePos(dest->pos.x, dest->pos.y, &mx, &my);

    print_dest_infos(mx, my, dest->nb_bars, dest->nb_cars_in);
}

void draw_tile_map(t_main *main, int x, int y, int type, int color) {
    if (type >= NB_TILES) {
        return;
    }

    if (type < 0){
        clear_tile_map(x, y);
        return;
    }

    // We want to place a tile but the tile on the map is marked for removal
    if (color == C_B_WHITE && main->mapt[y][x].action == 'r'){
        return;
    }

    int mx, my;
    mapToMousePos(x, y, &mx, &my);
    
    if (color == -1){   // Pick assigned color
        color = tiles_color[type];

        // Will be removed ?
        if (main->mapt[y][x].action == 'r') {
            color = AWAITING_RM_COLOR;

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

    if (color == REMOVAL_COLOR && main->mapt[y][x].action == 'r') {
        color = AWAITING_RM_COLOR;
    }

    draw_one_tile(mx, my, type, colors[color]);

    if (type == TILE_CROS && color != NOT_ENOUGH_CASH_COLOR){     // Drawing crossroads depends of connected roads

        char roads[4] = {0, 0, 0, 0};   // W, E, N, S
        char nb_roads = 0;

        if (crossroad_can_connect(main, x, y, 'W')) {roads[0] = 1; nb_roads++;}
        if (crossroad_can_connect(main, x, y, 'E')) {roads[1] = 1; nb_roads++;}
        if (crossroad_can_connect(main, x, y, 'N')) {roads[2] = 1; nb_roads++;}
        if (crossroad_can_connect(main, x, y, 'S')) {roads[3] = 1; nb_roads++;}

        if (nb_roads > 1){  // We only close crossroad if at least 2 roads connected
            // We draw in blue if not overed and not marked for removal
            if (main->mapt[y][x].action != 'r' && color != REMOVAL_COLOR) color = C_B_BLUE;

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
    if (main->mapt[y][x].type - 1              == TILE_CROS) draw_tile_map(main, x, y, TILE_CROS, -1);
    if (y > 0 && main->mapt[y - 1][x].type - 1 == TILE_CROS) draw_tile_map(main, x, y - 1, TILE_CROS, -1);
    if (main->mapt[y + 1][x].type - 1          == TILE_CROS) draw_tile_map(main, x, y + 1, TILE_CROS, -1);
    if (x > 0 && main->mapt[y][x - 1].type - 1 == TILE_CROS) draw_tile_map(main, x - 1, y, TILE_CROS, -1);
    if (main->mapt[y][x + 1].type - 1          == TILE_CROS) draw_tile_map(main, x + 1, y, TILE_CROS, -1);
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
                    draw_tile_map(main, x, y, type, REMOVAL_COLOR);
                    dprtxy(1, 52, "Can't remove tile at %d, %d", x, y);
                }
            }
        }

    } else if (main->mouse_btn == DESELECT_BTN) {   // DESELECT TILE
        ui_draw_menu_tile(ui->tile_selected, TILE_COLOR);
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
                            draw_tile_map(main, x, y, ui->tile_selected, NOT_ENOUGH_CASH_COLOR);
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
                draw_tile_map(main, x, y, main->mapt[y][x].type - 1, REMOVAL_COLOR);
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

        // Clear maps
        main->mapt[y][x].type = 0;
        main->mapt[y][x].action = 0;

        // Remove the tile or draw the tile mouse is placing
        if (main->ui.mouse_map.x == x && main->ui.mouse_map.y == y){
            draw_tile_map(main, x, y, main->ui.tile_selected, C_B_WHITE);
        } else {
            clear_tile_map(x, y);
        }

        // Update crossroads
        update_crossroads(main, x, y);

        main->need_path_update = true;
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
        ui_draw_menu_tile(prev_selected, TILE_COLOR);
        ui_draw_menu_tile(main->ui.tile_selected, SELECTED_COLOR);
        draw_tile_map(main, main->ui.mouse_map.x, main->ui.mouse_map.y, main->ui.tile_selected, C_B_WHITE);
        if (prev_selected == TILE_DEST){
            draw_tile_map(main, main->ui.mouse_map.x + 1, main->ui.mouse_map.y, 
                main->mapt[main->ui.mouse_map.y][main->ui.mouse_map.x + 1].type - 1, -1);
        }
    }
}

void ui_manage_mouse(t_main *main) {
    if (main->ui.show_help){
        return;
    }
    ui_mouse_wheel(main);
    ui_mouse_over_menu(main);
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

void check_cash(t_main *main){
    static bool prev_cannot_buy = 0;

    if (prev_cannot_buy == 0){
        if (main->player.cash < tiles_price[main->ui.tile_selected]){
            draw_tile_map(main, main->ui.mouse_map.x, main->ui.mouse_map.y, main->ui.tile_selected, NOT_ENOUGH_CASH_COLOR);
            prev_cannot_buy = 1;
        }
    } else {
        if (main->player.cash >= tiles_price[main->ui.tile_selected]){
            draw_tile_map(main, main->ui.mouse_map.x, main->ui.mouse_map.y, main->ui.tile_selected, tiles_color[main->ui.tile_selected]);
            prev_cannot_buy = 0;
        }
    }
}

void update_ui(t_main *main) {
    uint64_t now = millis();

    if (now - main->last_update_ui < UI_UPDATE_DELAY) {
        return;
    }

    check_tiles_to_be_removed(main);
    check_cash(main);

    int center = main->screen_w / 2;

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
    prtxy(35, 4, "%s   H   : %sHelp", B_YELLOW, YELLOW);
    
    prtxy(center + 35, 1, "%s+ / - : %sMusic volume (%s%d%s)  ", B_YELLOW, YELLOW, B_GREEN, main->volume, YELLOW);
    prtxy(center + 35, 2, "%s  M   : %sMusic (%s%s%s)  ", B_YELLOW, YELLOW, (main->music) ? B_GREEN : B_RED, (main->music) ? "ON" : "OFF", YELLOW);
    prtxy(center + 35, 3, "%s  F   : %sChange Font", B_YELLOW, YELLOW);

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
        if (tiles_en[i] == 0) {
            break;
        }
        int y = Y_TOP + i * (TILE_H + 2);
        if (tiles[i][0] == NULL) {
            break;
        }
        // Draw boxes
        for(int j = 0; j < TILE_H + 2; ++j){
            prtxy(TILE_W + 1, y + j, "│");
        }
        // Price
        if (i == TILE_VERT || i == TILE_HORI){
            prtxy(1, y + TILE_H, " %s%d $%s", B_YELLOW, CASH_PER_ROAD, ui_color);
        } else if (i == TILE_CROS){
            prtxy(1, y + TILE_H, " %s%d $%s", B_YELLOW, CASH_PER_CROSS, ui_color);
        }

        if (i >= NB_TILES - 1 || tiles_en[i + 1] == 0){ 
            prtxy(1, y + TILE_H + 1, "─────┘");
        } else {
            prtxy(1, y + TILE_H + 1, "─────┤");
        }

        // Draw tiles
        ui_draw_menu_tile(i, colors[tiles_color[i]]);
        printf("%s", ui_color);
    }

    draw_frame(main);

    printf("%s", RESET);
}

void print_game_over(t_main *main){
    static uint64_t last_time = 0;
    static int color = 0;

    if (millis() - last_time > 800){
        color++;

        printf("%s", colors[dests_color_index[color % NB_DEST_COLORS]]);

        int x = main->screen_w / 2 - 15;
        int y = main->screen_h / 2;
        prtxy(x, y - 7, "╔═══════════════════════════════════════════╗");
        prtxy(x, y - 6, "║                                           ║");
        prtxy(x, y - 5, "║                 GAME OVER                 ║");
        prtxy(x, y - 4, "║                                           ║");
        prtxy(x, y - 3, "║      Press 'r' to retry, ESC to Quit      ║");
        prtxy(x, y - 2, "║                                           ║");
        prtxy(x, y - 1, "║                                           ║");
        prtxy(x, y + 0, "║          .*.*  HIGH SCORES  *.*.          ║");
        prtxy(x, y + 1, "║                                           ║");
        prtxy(x, y + 2, "║                1:                         ║");
        prtxy(x, y + 3, "║                2:                         ║");
        prtxy(x, y + 4, "║                3:                         ║");
        prtxy(x, y + 5, "║                4:                         ║");
        prtxy(x, y + 6, "║                5:                         ║");
        prtxy(x, y + 7, "║                                           ║");
        prtxy(x, y + 8, "╚═══════════════════════════════════════════╝");

        for(int i = 0; i < 5; i++){
            prtxy(x + 20, y + 2 + i, "%03ld %03ld", main->reccord.high_scores[i] / 1000, main->reccord.high_scores[i] % 1000);
            if (main->player.score == main->reccord.high_scores[i]){
                prtxy(x + 15, y + 2 + i, ">");
                prtxy(x + 28, y + 2 + i, "<");
            }
        }

        printf("%s", RESET);

        last_time = millis();
    }
}

void print_help(t_main *main){
    int w = 100;
    int h = 36;
    int x = (main->screen_w - w) / 2;
    int y = (main->screen_h - h) / 2;

    if (y < Y_TOP + 1) y = Y_TOP + 1;

    printf("%s", B_YELLOW);

    if (main->ui.show_help){
        prtxy(x, y, "╔%.*s╗", (w - 2) * 3, h_line);
        for(int i = 1; i < h - 1; i++){
            prtxy(x, y + i, "║%.*s║", w - 2, spaces);
        }
        prtxy(x, y + h - 1, "╚%.*s╝", (w - 2) * 3, h_line);

        // Tiles 
        prtxy(x + (w - 17) / 2, y + 2, "%sWELCOME TO ASCARS", B_YELLOW);
        prtxy(x + (w - 85) / 2, y + 4, "%sThe goal of this game is to allow cars to travel from houses to factories using roads", YELLOW);

        draw_one_tile(x + 15, y + 6,    TILE_HORI, colors[tiles_color[TILE_HORI]]);
        draw_one_tile(x + 35, y + 6,    TILE_VERT, colors[tiles_color[TILE_VERT]]);
        draw_one_tile(x + 70, y + 6,    TILE_CROS, colors[tiles_color[TILE_CROS]]);

        printf("%s", YELLOW);
        prtxy(x + 10, y + 11, "Those are %sregular roads%s | Cost %s%d%s $", B_YELLOW, YELLOW, B_YELLOW, CASH_PER_ROAD, YELLOW);
        prtxy(x + 4, y + 12, "They can be placed horizontally or vertically");

        prtxy(x + 57, y + 11, "This is a %scrossroad%s | Cost %s%d%s $", B_YELLOW, YELLOW, B_YELLOW, CASH_PER_CROSS, YELLOW);
        prtxy(x + 59, y + 12, "Cars can go in any direction");
        prtxy(x + 53, y + 13, "But it takes %smore time%s for them to travel", B_YELLOW, YELLOW);
        
        // Origins and Destinations
        draw_one_tile(x + 10, y + 15,   TILE_ORIG, B_GREEN);
        printf("%s", YELLOW);
        prtxy(x + 17, y + 15, "This is a %shouse%s", B_YELLOW, YELLOW);
        prtxy(x + 17, y + 16, "Cars will spawn here");
        prtxy(x + 17, y + 17, "There is a limit of %s%d cars%s per house", B_YELLOW, NB_CARS_PER_ORIG, YELLOW);

        draw_one_tile(x + 6, y + 20,   TILE_DEST, B_GREEN);
        printf("%s", YELLOW);
        prtxy(x + 17, y + 20, "This is a %sfactory%s", B_YELLOW, YELLOW);
        prtxy(x + 17, y + 21, "Cars will have to go here");
        prtxy(x + 17, y + 22, "There is a limit of %s%d cars%s per factory", B_YELLOW, NB_CARS_PER_DEST, YELLOW);
        prtxy(x + 17, y + 23, "Factories must be connected to houses of the %ssame color%s", B_YELLOW, YELLOW);

        draw_one_tile(x + 6, y + 25,   TILE_DEST, B_BLACK);
        print_dest_infos(x + 6, y + 25, 3, 3);
        printf("%s", YELLOW);
        prtxy(x + 17, y + 25, "Number of cars inside is shown by %sv%s = 1 car, %sV%s = 2 cars", B_YELLOW, YELLOW, B_YELLOW, YELLOW);
        prtxy(x + 17, y + 27, "The %sbar graph%s represent the factory's %sdemand%s for cars", B_YELLOW, YELLOW, B_YELLOW, YELLOW);
        prtxy(x + 17, y + 28, "%sIf the bar fills up in %sred%s, you will %sloose !", YELLOW, B_RED, YELLOW, B_RED);

        prtxy(x + 7, y + 31, "%sAll available %sinformations%s and %skeys%s are shown on the top. Press %sH%s to hide this help", YELLOW, B_YELLOW, YELLOW, B_YELLOW, YELLOW, B_YELLOW, YELLOW);

        prtxy(x + (w - 11) / 2, y + 33, "%sGOOD LUCK !", B_YELLOW);


    } else {
        int tx_left, tx_right, ty_top, ty_bottom;
        mousePosToMap(x, y, &tx_left, &ty_top);
        mousePosToMap(x + w, y + h, &tx_right, &ty_bottom);

        // Clear
        for(int i = 0; i < h; i++){
            prtxy(x, y + i, "%.*s", w, spaces);
        }
        // Draw the map
        for(int i = ty_top - 1; i <= ty_bottom + 1; i++){
            for(int j = tx_left - 1; j <= tx_right + 1; j++){
                draw_tile_map(main, j, i, main->mapt[i][j].type - 1, -1);
            }
        }

    }

}