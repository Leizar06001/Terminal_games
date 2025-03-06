#include "main.h"

bool is_tunnel(int type){
    return type >= TILE_TUNNEL_S && type <= TILE_TUNNEL_W;
}

int remove_tunnel(t_main *main, int x, int y){
    int tunnel_id = main->mapt[y][x].tunnel_id;
    if (tunnel_id == -1) return 0;

    // Clear the tunnel
    int x0 = main->tunnels[tunnel_id].x[0];
    int y0 = main->tunnels[tunnel_id].y[0];
    int x1 = main->tunnels[tunnel_id].x[1];
    int y1 = main->tunnels[tunnel_id].y[1];
    main->mapt[y0][x0].tunnel_id = -1;
    main->mapt[y1][x1].tunnel_id = -1;
    main->mapt[y0][x0].type = 0;
    main->mapt[y1][x1].type = 0;
    main->mapt[y0][x0].action = 0;
    main->mapt[y1][x1].action = 0;
    main->tunnels[tunnel_id].enabled = 0;
    main->nb_tunnels--;

    // Clear the tiles
    // clear_tile_map(x0, y0);
    // clear_tile_map(x1, y1);

    return 1;
}

void convert_tunnel_pos_to_car_pos(int tunnel_x, int tunnel_y, char tunnel_dir, int *x, int *y){
    int mx, my;
    mapToMousePos(tunnel_x, tunnel_y, &mx, &my);
    switch (tunnel_dir){
        case 0:
            *x = mx + 1;
            *y = my + 3;
            break;
        case 2:
            *x = mx + 3;
            *y = my;
            break;
        case 1:
            *x = mx + 4;
            *y = my + 2;
            break;
        case 3:
            *x = mx;
            *y = my + 1;
            break;
    }
}

void get_tunnel_exit_pos(t_tunnel *tunnel, int x_in, int y_in, int *x_out, int *y_out){
    if (x_in == tunnel->x[0] && y_in == tunnel->y[0]){
        convert_tunnel_pos_to_car_pos(tunnel->x[1], tunnel->y[1], tunnel->rot[1], x_out, y_out);
    } else {
        convert_tunnel_pos_to_car_pos(tunnel->x[0], tunnel->y[0], tunnel->rot[0], x_out, y_out);
    }
}

void get_tunnel_linked_tile(t_tunnel *tunnel, int x, int y, int *x_out, int *y_out){
    if (x == tunnel->x[0] && y == tunnel->y[0]){
        *x_out = tunnel->x[1];
        *y_out = tunnel->y[1];
    } else {
        *x_out = tunnel->x[0];
        *y_out = tunnel->y[0];
    }
}