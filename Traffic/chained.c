#include "main.h"

/* *********** ORIGINS & DESTS *********** */

// returns the id of the first empty element
static int elem_fill_next_empty(t_elem *lst, int x, int y, int max) {
    int i = 0;
    while (i < max){
        if (lst[i].active == 0){
            lst[i].pos.x = x;
            lst[i].pos.y = y;
            lst[i].active = 1;
            lst[i].index = i;
            dprtxy(170, 2, "Added Id:%d x:%d, y:%d            ", i, x, y);
            return i;
        }
        i++;
    }
    return -1;
}

int orig_add_by_pos(t_main *main, int x, int y, int color_index) {
    int index = elem_fill_next_empty(main->origs, x, y, MAX_ORIGS);
    main->origs[index].color = color_index;
    main->origs[index].max_cars = main->nb_cars_per_origins;
    main->origs[index].nb_cars_in = main->nb_cars_per_origins;
    main->nb_origs++;
    return index;
}

int dest_add_by_pos(t_main *main, int x, int y, int color_index) {
    int index = elem_fill_next_empty(main->dests, x, y, MAX_DESTS);
    main->dests[index].color        = color_index;
    main->dests[index].max_cars     = NB_CARS_PER_DEST;
    main->dests[index].nb_cars_in   = 0;
    main->dests[index].nb_paths     = 0;
    main->dests[index].paths        = NULL;
    main->dests[index].time_left    = DEST_DEFAULT_TIME;
    main->dests[index].cooldown     = DEST_DEFAULT_COOLDOWN;
    main->dests[index].nb_bars      = 0;
    main->dests[index].prev_nb_bars = 0;
    for(int i = 0; i < NB_CARS_PER_DEST; i++){
        main->dests[index].cars_requests[i] = NULL;
    }
    main->nb_dests++;
    return index;
}

// removes the element at position x, y
static int elem_remove_by_pos(t_elem *lst, int x, int y) {
    int i = 0;
    while (lst[i].active != -1){
        if (lst[i].active == 1 && lst[i].pos.x == x && lst[i].pos.y == y){
            lst[i].active = 0;
            dprtxy(170, 2, "Removed Id:%d x:%d, y:%d        ", i, x, y);
            return 0;
        }
        i++;
    }
    return -1;
}

int orig_remove_by_pos(t_main *main, int x, int y) {
    int ret = elem_remove_by_pos(main->origs, x, y);
    if (ret == 0) main->nb_origs--;
    return ret;
}

int dest_remove_by_pos(t_main *main, int x, int y) {
    int ret = elem_remove_by_pos(main->dests, x, y);
    if (ret == 0) main->nb_dests--;
    return ret;
}

// Remove one elem from the list and update the chain
void elem_remove_by_index(t_elem *lst, int index) {
    lst[index].active = 0;
    // dprtxy(170, 2, "Removed Id:%d x:%d, y:%d        ", index, lst[index].pos.x, lst[index].pos.y);
}

t_elem *elem_get_by_pos(t_elem *lst, int x, int y) {
    int i = 0;
    while (lst[i].active != -1){
        if (lst[i].pos.x == x && lst[i].pos.y == y){
            return &lst[i];
        }
        i++;
    }
    return NULL;
}

t_elem *elem_get_next_active(t_elem *lst, int *index) {
    int i = *index;
    while (lst[i].active != -1){
        if (lst[i].active == 1){
            *index = i;
            return &lst[i];
        }
        i++;
    }
    return NULL;
}

/* *********** ORIGINS & DESTS *********** */


/* *********** PATHS *********** */

t_tile_paths *path_get_by_index_in_list(t_tile_paths *list, int index) {
    t_tile_paths *p = list;
    int i = 0;
    while (p != NULL) {
        if (i == index) return p;
        p = p->next;
        i++;
    }
    return NULL;
}

void add_tile_to_be_removed(t_main *main, int x, int y) {
    if (main->mapt[y][x].action == 'r') return;

    t_pos *p = malloc(sizeof(t_pos));
    if (p == NULL) return;

    p->x = x;
    p->y = y;
    p->next = main->tiles_to_be_removed;
    main->tiles_to_be_removed = p;
    main->mapt[y][x].action = 'r';
}

void remove_tile_from_remove_list(t_main *main, int x, int y) {
    t_pos *p = main->tiles_to_be_removed;
    t_pos *prev = NULL;
    t_pos *next = NULL;
    while (p != NULL) {
        next = p->next;
        if (p->x == x && p->y == y) {
            if (prev == NULL) {
                main->tiles_to_be_removed = p->next;
            } else {
                prev->next = p->next;
            }
            free(p);
            return;
        }
        prev = p;
        p = next;
    }
}

int nb_cars_using_path_on_tile(t_main *main, int x, int y) {
    int nb = 0;
    t_tile_paths *tps = main->mapt[y][x].paths;
    while (tps != NULL) {
        nb += tps->path->nb_cars_using;
        tps = tps->next;
    }
    return nb;
}

bool tile_in_use(t_main *main, int x, int y) {
    t_tile_paths *tps = main->mapt[y][x].paths;
    while (tps != NULL) {
        if (tps->path->nb_cars_using > 0) return true;
        tps = tps->next;
    }
    return false;
}

static void free_path(t_path *path) {
    if (path->steps) {
        free(path->steps);
        path->steps = NULL;
        path->length = -1;
    }
}

void disable_all_path_on_tile(t_main *main, int x, int y) {
    t_tile_paths *tps = main->mapt[y][x].paths;
    while (tps != NULL) {
        t_path *path = tps->path;
        path->active = false;
        tps = tps->next;
    }
}

bool cancel_all_path_on_tile(t_main *main, int x, int y) {

    disable_all_path_on_tile(main, x, y);
    
    // If the tile is in use, we can't remove it, mark it for removal
    if (tile_in_use(main, x, y)){
        add_tile_to_be_removed(main, x, y);
        return false;
    }

    t_tile_paths *tps = main->mapt[y][x].paths;

    while (tps != NULL) {
        t_tile_paths *next_tps = tps->next;
        t_path *path = tps->path;
        
        remove_path_from_all_tiles(main, path);

        tps = next_tps;
    }
    
    return true;
}

// Parcours le path et le retire de toutes les tiles
void remove_path_from_all_tiles(t_main *main, t_path *path) {
    if (!path) return;
    int k = 0;

    for (int i = 0; i < path->length; i++) {
        t_pos *tile_pos = &path->steps[i];
        t_map_tile *tile = &main->mapt[tile_pos->y][tile_pos->x];

        if (path_remove_from_list(&tile->paths, path)) {
            k++;
        }
    }

    // On decremente le nombre de paths des origines et destinations
    if (path->orig) {
        path->orig->nb_paths--;
        path_remove_from_list(&path->orig->paths, path);
    }
    if (path->dest) {
        path->dest->nb_paths--;
        path_remove_from_list(&path->dest->paths, path);
    }

    free_path(path);
    main->nb_paths--;
    dprtxy(170, 3, "Path len %d removed from %d tiles", path->length, k);
}

bool path_remove_from_list(t_tile_paths **list, t_path *path)
{
    if (!list || !*list || !path)
        return false;

    // We get the top of the list of paths in the tile
    // We remove the path from the list
    // We update the top of the list
    // We update the next_in_tile of the path

    t_tile_paths *cur  = *list;
    t_tile_paths *prev = NULL;

    while (cur) {
        if (cur->path == path) {
            // On supprime le maillon 'cur'
            if (prev == NULL) {
                // Si c'est le premier maillon
                *list = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            return true; // on renvoie true si on l’a effectivement retiré
        }
        prev = cur;
        cur = cur->next;
    }
    return false; // le path n’était pas dans la liste
}

void path_add_to_list(t_tile_paths **list, t_path *path) {
    if (!list || !path) return;
    t_tile_paths *new_tile_path = malloc(sizeof(t_tile_paths));
    if (new_tile_path == NULL) return;
    new_tile_path->path = path;
    new_tile_path->next = *list;
    *list = new_tile_path;
}

void path_add_to_map(t_main *main, t_path *path) {
    if (path->length < 2) return;
    // On ajout le *path à la liste des paths de chaque tile

    for (int i = 0; i < path->length; i++) {
        t_pos *tile_pos = &path->steps[i];
        t_map_tile *tile = &main->mapt[tile_pos->y][tile_pos->x];

        t_tile_paths *new_tile_path = malloc(sizeof(t_tile_paths));
        if (new_tile_path == NULL) return;
        new_tile_path->path = path;
        new_tile_path->next = tile->paths;
        tile->paths = new_tile_path;
    }
    path_add_to_list(&path->orig->paths, path);
    path_add_to_list(&path->dest->paths, path);
}

int path_count_in_list(t_tile_paths *top) {
    int i = 0;
    while (top != NULL) {
        i++;
        top = top->next;
    }

    return i;
}

/* *********** PATHS *********** */

/* *********** CARS *********** */

void add_car_to_path(t_path *path, t_cars *car) {
    t_car_list *new_car = malloc(sizeof(t_car_list));
    if (new_car == NULL) return;
    new_car->car = car;
    new_car->next = path->cars;
    path->cars = new_car;
}

void rm_car_from_path(t_path *path, t_cars *car) {
    t_car_list *cur  = path->cars;
    t_car_list *prev = NULL;

    while (cur) {
        if (cur->car == car) {
            if (prev == NULL) {
                path->cars = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

/* *********** CARS *********** */

/* *********** LIGHTS *********** */

void add_light_to_list(t_light_list **list, t_light *light) {
    t_light_list *new_light = malloc(sizeof(t_light_list));
    if (new_light == NULL) return;
    new_light->light = light;
    new_light->next = *list;
    *list = new_light;
}

void rm_light_from_list(t_light_list **list, t_light *light) {
    if (!list || !*list || !light)
        return;

    t_light_list *cur  = *list;
    t_light_list *prev = NULL;

    while (cur) {
        if (cur->light == light) {
            if (prev == NULL) {
                *list = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}