#include "main.h"

#include <pthread.h>

/* ********************************************************************************************************************** */
//_____/\\\\\\\\\________/\\\\\\\\\\\__________/\\\\\\\\\_____/\\\\\\\\\_______/\\\\\\\\\_________/\\\\\\\\\\\___         //
// ___/\\\\\\\\\\\\\____/\\\/////////\\\_____/\\\////////____/\\\\\\\\\\\\\___/\\\///////\\\_____/\\\/////////\\\_        //
//  __/\\\/////////\\\__\//\\\______\///____/\\\/____________/\\\/////////\\\_\/\\\_____\/\\\____\//\\\______\///__       //
//   _\/\\\_______\/\\\___\////\\\__________/\\\_____________\/\\\_______\/\\\_\/\\\\\\\\\\\/______\////\\\_________      //
//    _\/\\\\\\\\\\\\\\\______\////\\\______\/\\\_____________\/\\\\\\\\\\\\\\\_\/\\\//////\\\_________\////\\\______     //
//     _\/\\\/////////\\\_________\////\\\___\//\\\____________\/\\\/////////\\\_\/\\\____\//\\\___________\////\\\___    //
//      _\/\\\_______\/\\\__/\\\______\//\\\___\///\\\__________\/\\\_______\/\\\_\/\\\_____\//\\\___/\\\______\//\\\__   //
//       _\/\\\_______\/\\\_\///\\\\\\\\\\\/______\////\\\\\\\\\_\/\\\_______\/\\\_\/\\\______\//\\\_\///\\\\\\\\\\\/___  //
//        _\///________\///____\///////////___________\/////////__\///________\///__\///________\///____\///////////_____ //
//                                                                                                                        //
//          INUTILE DE PASSER LA NORME DE CODAGE À LA MOULINETTE, CE CODE EST PARFAIT !                                   //
//                                                                                                                        //
/* ********************************************************************************************************************** */

void sigint_handler(int signum) {
    (void)signum;
    running = false;
}

static int init_game(t_main *main){
    init_main(main);
    clear_screen();

    int w, h, bx, by;
    get_terminal_size(&w, &h);
    w -= 4; // Keep some space
    h -= 3; // Keep some space
    mousePosToMap(w, h, &bx, &by);
    main->board_w = bx - 1;
    main->board_h = by - 1;
    main->screen_w = w;
    main->screen_h = h;
    
    draw_ui(main);
    return 0;
}

int reset_game(t_main *main){
    free_all(main);
    init_game(main);
    return 0;
}

int volume_fadeout = 0;
const int game_closing_time = 3700;     // ms

// Thread to lower the volume
void *fade_out_music(void *arg){
    usleep(50000);
    t_main *main = (t_main *)arg;

    if (volume_fadeout == 0){
        return NULL;
    }
    long delay = (game_closing_time / volume_fadeout) * 1000;
    while (volume_fadeout > 0){
        set_volume(volume_fadeout--);
        usleep(delay);
    }
    kill_audio_process(main);
    return NULL;
}

int main(int argc, char *argv[]) {
    t_main main;

    main.game_started         = false;
    main.game_mode            = 1;
    main.god_mode             = false;
    main.audio_pid            = -1;
    main.audio_player_started = false;
    main.ui.alt_fonts         = false;
    
    bool print_infos = true;
    bool music_en    = true;
    
    for(int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-i") == 0){
            print_infos = false;
        }
        if (strcmp(argv[i], "-s") == 0){
            main.game_mode = 0;
        }
        if (strcmp(argv[i], "-es") == 0){
            exit_screen(&main);
            return 0;
        }
        if (strcmp(argv[i], "-x") == 0){
            main.god_mode = true;
        }
        if (strcmp(argv[i], "-m") == 0){
            music_en = false;
        }
    }
    
    // print_infos = false;
    
    signal(SIGINT, sigint_handler);
    init_terminal(&main);
    disable_mouse_tracking();
    
    // Load the previous parameters
    read_reccord_file(&main);
    main.music        = main.reccord.music;
    main.volume       = main.reccord.volume;
    main.ui.alt_fonts = main.reccord.alt_fonts;
    
    select_charset(&main);
    
    if (music_en){
        init_mpg123(&main);
        usleep(500000);
        set_volume(main.volume);
        if (main.music) play_mp3();
    }

    if (print_infos){
        title_screen(&main);
    }
    
    init_game(&main);
    enable_mouse_tracking_extended();

    const int av_range = 20;
    int nb_ranges = 0;

    uint64_t t_start = 0;
    uint64_t t_input = 0;
    uint64_t t_mouse = 0;
    uint64_t t_paths = 0;
    uint64_t t_cars = 0;
    uint64_t t_ui = 0;
    uint64_t t_end = 0;

    float av_input = 0;
    float av_mouse = 0;
    float av_paths = 0;
    float av_cars = 0;
    float av_ui = 0;
    float av_total = 0;

    uint32_t max_input = 0;
    uint32_t max_mouse = 0;
    uint32_t max_paths = 0;
    uint32_t max_cars = 0;
    uint32_t max_ui = 0;
    uint32_t max_total = 0;

    if (main.game_mode == 1){
        switch_game_to_normal(&main);
    }

    if (main.reccord.first_launch){
        main.ui.show_help = true;
        main.paused = true;
        print_help(&main);
    }

    static bool scores_updated = false;

    uint64_t last_mpg123_check = millis();

    // MAIN LOOP
    while (running) {

        if (main.audio_player_started && last_mpg123_check + 1000 < millis()){
            check_mpg123_messages();
            last_mpg123_check = millis();
        }

        t_start = millis();
        int ret = read_input(&main);

        // GAME OVER SCREEN
        if (main.game_over) {
            if (!scores_updated){
                update_high_scores(&main);
                scores_updated = true;
            }
            print_game_over(&main);
            continue;
        }
        scores_updated = false;

        t_input = millis();
        if (ret){
            ui_manage_mouse(&main);
        }
        t_mouse = millis();

        update_paths(&main);
        t_paths = millis();

        if (!main.paused){
            uint64_t now = millis();

            if (now - main.last_cars_update > main.game_speed){
                update_lights(&main);
                update_cars(&main);

                if (main.game_mode == 1){
                    game_logic_loop(&main);
                }

                main.last_cars_update = now;
                main.game_frame++;
            }
        }
        t_cars = millis();

        update_ui(&main);
        t_ui = millis();

        t_end = millis();



        if (t_input - t_start > max_input)  max_input = t_input - t_start;
        if (t_mouse - t_input > max_mouse)  max_mouse = t_mouse - t_input;
        if (t_paths - t_mouse > max_paths)  max_paths = t_paths - t_mouse;
        if (t_cars - t_paths > max_cars)    max_cars = t_cars - t_paths;
        if (t_ui - t_cars > max_ui)         max_ui = t_ui - t_cars;
        if (t_end - t_start > max_total)    max_total = t_end - t_start;

        av_input    += t_input - t_start;
        av_mouse    += t_mouse - t_input;
        av_paths    += t_paths - t_mouse;
        av_cars     += t_cars - t_paths;
        av_ui       += t_ui - t_cars;
        av_total    += t_end - t_start;
        nb_ranges++;

        if (nb_ranges >= av_range){
            av_input    /= av_range;
            av_mouse    /= av_range;
            av_paths    /= av_range;
            av_cars     /= av_range;
            av_ui       /= av_range;
            av_total    /= av_range;

            dprtxy(1, 40, "Input    : %.2f ms / %d  ", av_input, max_input);
            dprtxy(1, 41, "Mouse    : %.2f ms / %d  ", av_mouse, max_mouse);
            dprtxy(1, 42, "Paths    : %.2f ms / %d  ", av_paths, max_paths);
            dprtxy(1, 43, "Cars     : %.2f ms / %d  ", av_cars, max_cars);
            dprtxy(1, 44, "UI       : %.2f ms / %d  ", av_ui, max_ui);
            dprtxy(1, 45, "Total    : %.2f ms / %d  ", av_total, max_total);
            if (av_total != 0){
                main.fps = 1000 / av_total;
            }

            av_input = 0;
            av_mouse = 0;
            av_paths = 0;
            av_cars = 0;
            av_ui = 0;
            av_total = 0;
            nb_ranges = 0;
        }

        usleep(1000);
    }

    // Save reccord
    update_reccord(&main);
    write_reccord_file(&main);

    quit_main(&main);

    // We make a thread to lower the volume while showing the exit screen
    pthread_t thread;
    if (music_en && main.audio_player_started){
        volume_fadeout = main.volume;
        // Create a thread to lower the volume
        // pass main as argument
        pthread_create(&thread, NULL, fade_out_music, &main);
    }

    if (print_infos){
        exit_screen(&main);
        usleep(1000000); 
    }

    if (music_en && main.audio_player_started){
        kill_audio_process(&main);
        pthread_join(thread, NULL);
    }

    return 0;
}