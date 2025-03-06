#include "main.h"

// Directions for BFS: up, down, left, right
int DIRECTION[4][2] = { {0,-1}, {0,1}, {-1,0}, {1,0} };

/**
 * "Normalise" le tile en fonction de l'origine BFS (xOrig, yOrig).
 * - tile 4 (une origine) est passable uniquement si c'est précisément la case (xOrig, yOrig).
 * - tile 5 (une destination) est traité comme 3 (croisement) pour autoriser le passage.
 * - tile 0 = bloqué
 * - tile 1 = vertical
 * - tile 2 = horizontal
 * - tile 3 = croisement
 */
static int getTileForBFS(int originalTile, int x, int y, int xOrig, int yOrig)
{
    if (originalTile == MAP_TILE_ORIG) {
        // Passable seulement s'il s'agit de l'origine BFS
        if (x == xOrig && y == yOrig) {
            return MAP_TILE_CROS;  // on assimile l'origine à un croisement pour pouvoir partir dans n'importe quelle direction
        } else {
            return MAP_TILE_EMPTY;  // bloqué
        }
    }
    else if (originalTile == MAP_TILE_DEST) {
        // Les destinations sont traitées comme des croisements
        return MAP_TILE_CROS;
    }
    // Sinon, on renvoie tel quel
    return originalTile;
}

/**
 * Vérifie si on peut passer du tile 'currentTile' vers 'nextTile'
 * en allant dans la direction (dx, dy).
 *
 * Comme on a "converti" tile 4 -> 3 ou 0, tile 5 -> 3,
 * les seules valeurs possibles ici sont 0,1,2,3.
 */
static bool isValidTransition(int currentTile, int nextTile, int dx, int dy)
{
    if (nextTile == MAP_TILE_EMPTY) return false; // bloqué

    // 1 = vertical => ne peut bouger que haut/bas
    if (currentTile == MAP_TILE_VERT) {
        // dx=0, dy=-1 (haut) ou dy=1 (bas)
        if (dx == 0) {
            // next doit être 1 ou 3 ou tunnel
            if (dy == -1){
                return (nextTile == MAP_TILE_VERT || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_S);
            } else if (dy == 1){
                return (nextTile == MAP_TILE_VERT || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_N);
            }
        }
        return false;
    }
    // 2 = horizontal => ne peut bouger que gauche/droite
    if (currentTile == MAP_TILE_HORI) {
        if (dy == 0) {
            if (dx == -1){
                return (nextTile == MAP_TILE_HORI || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_E);
            } else if (dx == 1){
                return (nextTile == MAP_TILE_HORI || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_W);
            }
        }
        return false;
    }
    // 3 = croisement => peut aller dans toutes les directions (mais "logiquement")
    //   - si on va haut/bas => la case suivante doit être 1 ou 3
    //   - si on va gauche/droite => la case suivante doit être 2 ou 3
    if (currentTile == MAP_TILE_CROS) {
        if (dx == 0) {
            if (dy == -1){
                return (nextTile == MAP_TILE_VERT || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_S);
            } else if (dy == 1){
                return (nextTile == MAP_TILE_VERT || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_N);
            }
        }
        else if (dy == 0) {
            if (dx == -1){
                return (nextTile == MAP_TILE_HORI || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_E);
            } else if (dx == 1){
                return (nextTile == MAP_TILE_HORI || nextTile == MAP_TILE_CROS || nextTile == MAP_TILE_TUNNEL_W);
            }
        }
        return false;
    }

    if (currentTile == MAP_TILE_TUNNEL_S){
        if (dx == 0 && dy == 1){
            return (nextTile == MAP_TILE_VERT || nextTile == MAP_TILE_CROS);
        }
        return false;
    }

    if (currentTile == MAP_TILE_TUNNEL_N){
        if (dx == 0 && dy == -1){
            return (nextTile == MAP_TILE_VERT || nextTile == MAP_TILE_CROS);
        }
        return false;
    }

    if (currentTile == MAP_TILE_TUNNEL_E){
        if (dx == 1 && dy == 0){
            return (nextTile == MAP_TILE_HORI || nextTile == MAP_TILE_CROS);
        }
        return false;
    }

    if (currentTile == MAP_TILE_TUNNEL_W){
        if (dx == -1 && dy == 0){
            return (nextTile == MAP_TILE_HORI || nextTile == MAP_TILE_CROS);
        }
        return false;
    }

    return false; // 0 = bloqué
}

bool isTunnelEntrance(int type){
    return type >= MAP_TILE_TUNNEL_S && type <= MAP_TILE_TUNNEL_W;
}

/**
 * BFS global "une seule fois" depuis l'origine (xOrig,yOrig).
 *   - Remplit `visited[y][x]` et `parent[y][x]` pour TOUTE la grille.
 *   - Ne s'arrête pas dès qu'on trouve une destination ; on explore tout.
 */
void bfsFromOrigin(t_main *main, t_map_tile mapt[MAX_H][MAX_W],
                   int mapH, int mapW,
                   int xOrig, int yOrig,
                   bool visited[MAX_H][MAX_W],
                   t_pos parent[MAX_H][MAX_W])
{
    // Initialisation
    for (int r = 0; r < mapH; r++) {
        for (int c = 0; c < mapW; c++) {
            visited[r][c] = false;
            parent[r][c].x = -1;
            parent[r][c].y = -1;
        }
    }

    // On regarde le "tile effectif" de l'origine
    int tileOrig = getTileForBFS(mapt[yOrig][xOrig].type, xOrig, yOrig, xOrig, yOrig);
    if (tileOrig == MAP_TILE_EMPTY) {
        // Si l'origine est bloquée quand ce n'est pas la bonne case, BFS = vide
        return;
    }

    // File FIFO
    t_pos queue[MAX_W * MAX_H];
    int front = 0, back = 0;

    // Enfile l'origine
    queue[back].x = xOrig;
    queue[back].y = yOrig;
    back++;
    visited[yOrig][xOrig] = true;

    // Parcours BFS
    while (front < back) {
        // Défile
        t_pos current = queue[front++];
        int cx = current.x;
        int cy = current.y;

        // Si la case courante est une destination (type=5) et que 
        // ce n'est pas la case d'origine, on n'explore pas les voisins.
        if (mapt[cy][cx].type == MAP_TILE_DEST && !(cx == xOrig && cy == yOrig)) {
            // On a "atteint" cette destination, donc on ne la traverse pas
            continue;
        }

        // Tile courant
        int cTile = getTileForBFS(mapt[cy][cx].type, cx, cy, xOrig, yOrig);

        // On parcourt les 4 voisins
        for (int i = 0; i < 4; i++) {
            int dx = DIRECTION[i][0];
            int dy = DIRECTION[i][1];
            int nx = cx + dx;
            int ny = cy + dy;

            // bornes
            if (nx < 0 || nx >= mapW || ny < 0 || ny >= mapH) {
                continue;
            }
            if (!visited[ny][nx]) {
                int nTile = getTileForBFS(mapt[ny][nx].type, nx, ny, xOrig, yOrig);
                if (isValidTransition(cTile, nTile, dx, dy)) {
                    visited[ny][nx] = true;
                    parent[ny][nx].x = cx;
                    parent[ny][nx].y = cy;
                    queue[back].x = nx;
                    queue[back].y = ny;
                    back++;
                }
            }
        }

        // 2) Puis, si c'est une entrée de tunnel, on "saute" directement
        //    à l'autre extrémité en l'ajoutant dans la file BFS.
        if (isTunnelEntrance(mapt[cy][cx].type)) {
            // Supposons qu'on retrouve la case "paire" (pairedX, pairedY)
            // en cherchant dans un tableau/structure qui décrit les paires.
            int tunnel_id = mapt[cy][cx].tunnel_id;
            int pairedX, pairedY;
            int x0 = main->tunnels[tunnel_id].x[0];
            int y0 = main->tunnels[tunnel_id].y[0];
            int x1 = main->tunnels[tunnel_id].x[1];
            int y1 = main->tunnels[tunnel_id].y[1];

            if (cx == x0 && cy == y0) {
                pairedX = x1;
                pairedY = y1;
            } else {
                pairedX = x0;
                pairedY = y0;
            }

            if (!visited[pairedY][pairedX]) {
                visited[pairedY][pairedX] = true;
                parent[pairedY][pairedX].x = cx;
                parent[pairedY][pairedX].y = cy;
                queue[back].x = pairedX;
                queue[back].y = pairedY;
                back++;
            }
        }
    }
}

/**
 * Reconstruit le chemin depuis (xDest, yDest) vers l'origine,
 * en se basant sur le tableau `parent[][]` rempli par BFS,
 * puis le renverse pour obtenir l'ordre Start→...→Dest.
 *
 * Retourne la longueur du chemin construit dans `outPath[]`.
 * Retourne 0 si (xDest,yDest) n'a pas de parent (non visité).
 */
int reconstructPath(int xDest, int yDest,
                    t_pos parent[MAX_H][MAX_W],
                    t_pos outPath[])
{
    int length = 0;
    int rx = xDest;
    int ry = yDest;

    // remonte jusqu'à parent = {-1, -1}
    while (rx != -1 && ry != -1) {
        outPath[length].x = rx;
        outPath[length].y = ry;
        length++;
        t_pos p = parent[ry][rx];
        rx = p.x;
        ry = p.y;
    }

    // on a le chemin à l'envers, on le renverse
    for (int i = 0; i < length / 2; i++) {
        t_pos tmp = outPath[i];
        outPath[i] = outPath[length - 1 - i];
        outPath[length - 1 - i] = tmp;
    }

    return length;
}

void swap_paths(t_path *p1, t_path *p2)
{
    t_path tmp;
    memcpy(&tmp, p1, sizeof(t_path));
    memcpy(p1, p2, sizeof(t_path));
    memcpy(p2, &tmp, sizeof(t_path));
}

void stop_pf()
{
}

void check_path_awaiting_removal(t_main *main){
    // Check si on a des chemins en attente de suppression
    // Si aucune voiture n'utilise le chemin, on peut le supprimer

    // On parcourt toutes les origines actives
    int id_orig = 0;
    t_elem *orig = NULL;

    while ((orig = elem_get_next_active(main->origs, &id_orig)) != NULL) {
        // Maintenant, on parcourt toutes les destinations
        int id_dest = 0;
        t_elem *dest = NULL;

        while ((dest = elem_get_next_active(main->dests, &id_dest)) != NULL) {

            t_path *pinfo = &main->paths[orig->index][dest->index][1];
            if (pinfo->steps != NULL && pinfo->nb_cars_using == 0){
                remove_path_from_all_tiles(main, pinfo);
            }
            id_dest++;
        }
        id_orig++;
    }       
}

void update_paths(t_main *main)
{
    check_path_awaiting_removal(main);


    if (main->need_path_update == false){
        return;
    }

    // On parcourt toutes les origines actives
    int id_orig = 0;
    t_elem *orig = NULL;

    int new_paths = 0;

    while ((orig = elem_get_next_active(main->origs, &id_orig)) != NULL) {
        // Prépare les tableaux BFS
        static bool visited[MAX_H][MAX_W];
        static t_pos parent[MAX_H][MAX_W];

        // Lance le BFS depuis cette origine
        bfsFromOrigin(main, main->mapt, MAX_H, MAX_W,
                      orig->pos.x, orig->pos.y,
                      visited, parent);

        // Maintenant, on parcourt toutes les destinations
        int id_dest = 0;
        t_elem *dest = NULL;

        while ((dest = elem_get_next_active(main->dests, &id_dest)) != NULL) {


            // Vérifie si on n'a deja un chemin en attente de suppression
            t_path *pMain   = &main->paths[orig->index][dest->index][0];
            t_path *pOld    = &main->paths[orig->index][dest->index][1];

            if (pOld->steps == NULL) { 
                // => aucun chemin stocké pour ce O->D
                // => Vérifie si BFS a atteint la destination
                if (visited[dest->pos.y][dest->pos.x]) {
                    // On reconstruit le chemin
                    t_pos bufferPath[MAX_W * MAX_H];
                    int len = reconstructPath(dest->pos.x, dest->pos.y, parent, bufferPath);

                    // Check si le nouveau chemin est plus court que l'ancien
                    if (len > 2 && (pMain->steps == NULL || len < pMain->length)) {

                        bool must_remap_ptrs = false;

                        if (pMain->steps != NULL){
                            // On check si le chemin actuel est utilisé par des voitures
                            if (pMain->nb_cars_using == 0){
                                // On ne garde que pMain
                                remove_path_from_all_tiles(main, pMain);
                            
                            } else {
                                // On ne peut pas supprimer pMain tout de suite
                                // On doit tag toutes les voitures qui utilisaient pMain pour utiliser pOld
                                t_car_list *carl = pMain->cars;
                                while (carl){
                                    carl->car->path_index = 1;
                                    carl = carl->next;
                                }

                                // On retire le pointeur de pMain de toutes les tiles pour y mettre pOld
                                for (int i = 0; i < pMain->length; i++) {
                                    t_pos *tile_pos = &pMain->steps[i];
                                    t_map_tile *tile = &main->mapt[tile_pos->y][tile_pos->x];
                            
                                    path_remove_from_list(&tile->paths, pMain);
                                }
                                must_remap_ptrs = true;
                            }
                        }


                        // On passe l'ancien chemin en "old"
                        swap_paths(pOld, pMain);

                        // Ok on ajoute les donnees du nouveau chemin qui est maintenant pMain
                        t_pos *allocatedPath = malloc(len * sizeof(t_pos));
                        if (allocatedPath) {
                            memcpy(allocatedPath, bufferPath, len * sizeof(t_pos));

                            // Remplir pMain
                            pMain->steps  = allocatedPath;
                            pMain->length = len;
                            pMain->active = true;
                            pMain->nb_cars_using = 0;
                            pMain->orig = orig;
                            pMain->dest = dest;

                            // Mise à jour
                            orig->nb_paths++;
                            dest->nb_paths++;

                            path_add_to_list(&orig->paths, pMain);
                            path_add_to_map(main, pMain);

                            new_paths++;
                        }

                        if (must_remap_ptrs){
                            path_add_to_map(main, pOld);
                        }

                    }
                }
            }
            id_dest++;
        }

        id_orig++;
    }

    // Debug
    // dprtxy(90, 3, "New paths: %d/%d", new_paths, main->nb_paths);

    // On considère qu'on a fini le recalcul complet des chemins
    main->need_path_update = false;
}
