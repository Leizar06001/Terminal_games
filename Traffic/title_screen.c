#include "main.h"

#define Y_TITLE_SCREEN 3
#define X_TITLE_SCREEN 10

const int ascii_h = 9;
const char ascii_art[9][130] = {
        "_____/\\\\\\\\\\\\\\\\\\________/\\\\\\\\\\\\\\\\\\\\\\__________/\\\\\\\\\\\\\\\\\\_____/\\\\\\\\\\\\\\\\\\_______/\\\\\\\\\\\\\\\\\\_________/\\\\\\\\\\\\\\\\\\\\\\___",
        "___/\\\\\\\\\\\\\\\\\\\\\\\\\\____/\\\\\\/////////\\\\\\_____/\\\\\\////////____/\\\\\\\\\\\\\\\\\\\\\\\\\\___/\\\\\\///////\\\\\\_____/\\\\\\/////////\\\\\\_",
        "__/\\\\\\/////////\\\\\\__\\//\\\\\\______\\///____/\\\\\\/____________/\\\\\\/////////\\\\\\_\\/\\\\\\_____\\/\\\\\\____\\//\\\\\\______\\///__",
        "_\\/\\\\\\_______\\/\\\\\\___\\////\\\\\\__________/\\\\\\_____________\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\\\\\\\\\\\\\\\\\/______\\////\\\\\\_________",
        "_\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\______\\////\\\\\\______\\/\\\\\\_____________\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_\\/\\\\\\//////\\\\\\_________\\////\\\\\\______",
        "_\\/\\\\\\/////////\\\\\\_________\\////\\\\\\___\\//\\\\\\____________\\/\\\\\\/////////\\\\\\_\\/\\\\\\____\\//\\\\\\___________\\////\\\\\\___",
        "_\\/\\\\\\_______\\/\\\\\\__/\\\\\\______\\//\\\\\\___\\///\\\\\\__________\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\_____\\//\\\\\\___/\\\\\\______\\//\\\\\\__",
        "_\\/\\\\\\_______\\/\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/______\\////\\\\\\\\\\\\\\\\\\_\\/\\\\\\_______\\/\\\\\\_\\/\\\\\\______\\//\\\\\\_\\///\\\\\\\\\\\\\\\\\\\\\\/___",
        "_\\///________\\///____\\///////////___________\\/////////__\\///________\\///__\\///________\\///____\\///////////_____"
    };

void print_ascii(int x_title, int y_title){
    for(int i = 0; i < ascii_h; i++){
        // prtxy(1 + i, Y_TITLE_SCREEN + i, "%s%.*s%s", BOLD_CYAN, 130, ascii_art[i], RESET);
        int j = 0;
        while (ascii_art[i][j] != '\0'){
            if (ascii_art[i][j] == '_'){
                prtxy(x_title + j + i, y_title + i, "%s%s%s", WHITE, "_", RESET);
            } else {
                prtxy(x_title + j + i, y_title + i, "%s%c%s", BOLD_CYAN, ascii_art[i][j], RESET);
            }
            j++;
        }
    }
}

void print_title_screen(){
    system("clear");

    print_ascii(1, Y_TITLE_SCREEN);

    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 0,  "╔════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 1,  "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 2,  "║   To play this game you'd better full screen the terminal and reduce the font size %s(CTRL + '-')%s    ║\n", BOLD_CYAN, RESET);
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 3,  "║       I recommend at least 230 x 60, but you don't have to. The max game size is %d x %d         ║\n", MAX_W * TILE_W, MAX_H * TILE_H);
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 4,  "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 5,  "║                       %sAvoid resizing the terminal while playing%s                                    ║\n", BOLD_YELLOW, RESET);
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 6,  "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 7,  "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 8,  "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 9,  "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 10, "║                            If you have %sfont errors%s, press %s'F'%s                                      ║\n", BOLD_RED, RESET, BOLD_CYAN, RESET);
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 11, "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 12, "║      The game has %sMusic%s, to play it you must have %smpg123 installed%s > %ssudo apt install mpg123%s       ║\n", BOLD_HI_MAGENTA, RESET, BOLD_HI_MAGENTA, RESET, BOLD_YELLOW, RESET);
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 13, "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 14, "║                                Press %sENTER%s when you're ready to start...                           ║\n", BOLD_GREEN, RESET);
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 15, "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 16, "║                                  %s https://github.com/Leizar06001 %s                                  ║\n", BOLD_CYAN, RESET);
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 17, "║                                                                                                    ║\n");
    prtxy(X_TITLE_SCREEN, Y_TITLE_SCREEN + ascii_h + 3 + 18, "╚════════════════════════════════════════════════════════════════════════════════════════════════════╝\n");
}

void anim_ascii(){
    int w, h;
    get_terminal_size(&w, &h);

    const int target_x = 0;
    int x = w;

    for(int i = 0; i < ascii_h; i++){
        x = w;
        while (x > target_x + i){
            prtxy(x, Y_TITLE_SCREEN + i, "-");
            usleep(200);
            x--;
        }
    }

    x = w;
    while (x > target_x){
        print_ascii(x, Y_TITLE_SCREEN);
        usleep(1500);
        x--;
    }

    x = w;
    int ascii_len = strlen(ascii_art[0]);
    while (x > target_x + ascii_len){
        for(int i = 0; i < ascii_h; i++){
            int xl = x + i;
            if (xl > w) continue;
            prtxy(xl, Y_TITLE_SCREEN + i, " ");
            usleep(100);
        }
        x--;
    }
}

void title_screen(t_main *main){
    system("clear");

    anim_ascii();

    print_title_screen();

    int w, h;
    int prev_w = -1;
    int prev_h = -1;
    while (main->game_started == false){
        get_terminal_size(&w, &h);
        if (w != prev_w || h != prev_h){
            prtxy(X_TITLE_SCREEN + 27, Y_TITLE_SCREEN + ascii_h + 10, " ■ Your terminal size: %d x %d", w, h);
            if (w < 188 || h < 49){
                prtxy(X_TITLE_SCREEN + 65, Y_TITLE_SCREEN + ascii_h + 10, " %s> TOO SMALL !%s      ", BOLD_RED, RESET);
            } else if (w < 230 || h < 60){
                prtxy(X_TITLE_SCREEN + 65, Y_TITLE_SCREEN + ascii_h + 10, " %s> Acceptable%s       ", BOLD_YELLOW, RESET);
            } else {
                prtxy(X_TITLE_SCREEN + 65, Y_TITLE_SCREEN + ascii_h + 10, " %s> OK !%s             ", BOLD_GREEN, RESET);
            }
            prev_w = w;
            prev_h = h;
        }
        read_input(main);
        usleep(100000);
    }
}

void exit_screen(t_main *main){
    int w, h;
    int Xez[150];
    int yColors[150];

    get_terminal_size(&w, &h);
    for(int i = 0; i < h; i++){
        Xez[i] = get_random(w) + w;
        yColors[i] = get_random(6);
    }

    int delay = 12000;
    const int min_delay = 5000;
    const int dec = 40;

    bool done = false;
    while (!done){
        done = true;
        for(int y = 0; y < h; y++){
            if (Xez[y] < 1) continue;
            if (Xez[y] > w) {Xez[y]--; done = false; continue;}

            if (Xez[y] == 1){
                if (y % 2 == 0){
                    prtxy(0, y, "   ");
                } else {
                    prtxy(w - 2, y, "   ");
                }
            } else {
                if (y % 2 == 0){
                    prtxy(Xez[y], y, "%s%s%s◁ ", colors[yColors[y] + 7], main->cars_charset[1][3], colors[yColors[y]]);
                } else {
                    prtxy(w - Xez[y], y, " %s▷%s%s", colors[yColors[y]], colors[yColors[y] + 7], main->cars_charset[1][2]);
                }
            }
            done = false;
            Xez[y]--;
        }
        usleep(delay);
        delay -= dec;
        if (delay < min_delay) delay = min_delay;
    }

    usleep(100000);
    system("clear");

    printf("%s", colors[get_random(6) + 7]);
    
    const char *msg = "\n\n      Thanks for playing !\n\n";
    int len = strlen(msg);

    for(int i = 0; i < len; i++){
        write(1, &msg[i], 1);
        usleep(50000);
    }
    
    flush_input();
}