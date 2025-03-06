#include "main.h"
#include <poll.h>

void flush_input(void)
{
    struct pollfd fds;
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;
    
    char buf[256];

    while (1)
    {
        // Poll with zero timeout => non-blocking
        int ret = poll(&fds, 1, 0);
        if (ret <= 0) {
            // No data or an error in poll => stop
            break;
        }
        // If we have data to read...
        if (fds.revents & POLLIN) {
            // Attempt to read
            ssize_t bytes_read = read(STDIN_FILENO, buf, sizeof(buf));
            if (bytes_read <= 0) {
                // EOF or error => stop
                break;
            }
            // Otherwise, we keep looping to drain more
        } else {
            // Nothing to read => done
            break;
        }
    }
}

void update_cars_per_origins(t_main *main){
    int diff = main->nb_cars_per_origins - main->origs[0].max_cars;
    for(int i = 0; i < MAX_ORIGS; i++){
        main->origs[i].max_cars = main->nb_cars_per_origins;
        main->origs[i].nb_cars_in += diff;
    }
}

int read_input(t_main *main) {
    char buf[64];
    char to_split[64];
    struct pollfd fds;
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;

    memset(buf, 0, sizeof(buf));
    memset(to_split, 0, sizeof(to_split));
    
    int ret = poll(&fds, 1, 1);  // Timeout: 10ms

    if (ret > 0) {
        if (fds.revents & POLLIN) {
            int bytes_read = read(STDIN_FILENO, buf, sizeof(buf) - 1);

            // Only used at the beginning to start the game
            if (main->game_started == false){
                if (bytes_read > 0){
                    main->game_started = true;
                    // flush_input();
                }
                return 0;
            }
            // ----------------------------------------------

            if (bytes_read > 60){
                flush_input();
                return 0;
            }

            if (bytes_read > 3) {

                if (strncmp(buf, "\033[<", 3) != 0) {
                    dprtxy(1, 1, "Unknown sequence: %s", buf);
                    return 0;
                }

                int n = 0;
                while (buf[n + 3]){
                    to_split[n] = buf[n + 3];
                    n++;
                }
                char **tokens = split(to_split, ';');
                if (tokens == NULL) {
                    return 0;
                }

                int i = 0;
                int btn = 0;
                while (tokens[i]){
                    if (i == 0){
                        btn = atoi(tokens[i]);
                    } else if (i == 1){
                        main->mouse_x = atoi(tokens[i]);
                    } else if (i == 2){
                        main->mouse_y = atoi(tokens[i]);
                    }
                    i++;
                }

                bool pressed = (buf[bytes_read - 1] == 'M') ? true : false;

                // Move with btn pressed :
                if (pressed){
                    if (btn == 32 || btn == 0) {
                        main->mouse_btn = 1;
                    } else if (btn == 33 || btn == 1) {
                        main->mouse_btn = 2;
                    } else if (btn == 34 || btn == 2) {
                        main->mouse_btn = 3;
                    } else if (btn == 64) { // Wheel up
                        main->mouse_btn = 4;
                    } else if (btn == 65) { // Wheel down
                        main->mouse_btn = 5;
                    } else {
                        main->mouse_btn = 0;
                    }
                } else {
                    main->mouse_btn = 0;
                }

                i = 0;
                while (tokens[i]){
                    free(tokens[i]);
                    i++;
                }
                free(tokens);

                return 1;


            } else if (bytes_read){
                switch (buf[0]){

                    // GENERAL game controls
                    case ' ':
                        main->paused = (main->paused) ? false : true;
                        break;
                    case '+': 
                        main->volume += 5;
                        if (main->volume > 100) main->volume = 100;
                        set_volume(main->volume);
                        break;
                    case '-':
                        main->volume -= 5;
                        if (main->volume < 0) main->volume = 0;
                        set_volume(main->volume);
                        break;
                    case '1':
                        main->speed_index = 0;
                        main->game_speed = speeds[main->speed_index];
                        break;
                    case '2':
                        main->speed_index = 1;
                        main->game_speed = speeds[main->speed_index];
                        break;
                    case '3':
                        main->speed_index = 2;
                        main->game_speed = speeds[main->speed_index];
                        break;
                    case '4':
                        main->speed_index = 3;
                        main->game_speed = speeds[main->speed_index];
                        break;
                    case '5':
                        main->speed_index = 4;
                        main->game_speed = speeds[main->speed_index];
                        break;
                    case 'd':
                        prt_debug = (prt_debug) ? false : true;
                        break;
                    case 'x':
                        reset_game(main);
                        break;
                    case 27:    // ESC -> exit
                        if (bytes_read == 1)
                            running = false;
                        break;
                    case 'm':
                        main->music = (main->music) ? false : true;
                        if (main->music){
                            play_mp3();
                        } else {
                            stop_mp3();
                        }
                        break;
                    case 'f':
                        select_charset(main);
                        break;
                    case 'h':
                        main->ui.show_help = (main->ui.show_help) ? false : true;
                        main->paused = (main->ui.show_help) ? true : false;
                        print_help(main);
                        break;
                    case 'r':
                        main->ui.tunnel_rotation++;
                        if (main->ui.tunnel_rotation > 3) main->ui.tunnel_rotation = 0;
                        main->ui.update_mouse_tile = true;
                        break;
                }

                // Only for sandbox mode
                if (main->game_mode == 0){
                    switch (buf[0]){   
                        // CARS controls
                        case 'c':
                            disable_new_cars = (disable_new_cars) ? false : true;
                            break;
                        case '*':
                            main->chance_new_car += 5;
                            if (main->chance_new_car > MAX_CHANCE_ADD_CAR) main->chance_new_car = MAX_CHANCE_ADD_CAR;
                            break;
                        case '/':
                            main->chance_new_car -= 5;
                            if (main->chance_new_car < 0) main->chance_new_car = 0;
                            break;
                        case '8':
                            if (main->nb_cars_per_origins > 1) main->nb_cars_per_origins--;
                            update_cars_per_origins(main);
                            break;
                        case '9':
                            if (main->nb_cars_per_origins < 9) main->nb_cars_per_origins++;
                            update_cars_per_origins(main);
                            break;

                    }
                }
            }
        }
    }
    return 0;
}