#include "toucher_anims.h"
#include "terminal_colors.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "globals.h"    
#include "toucher_comms.h"
#include "toucher_utils.h"

void anim_shoot_received(int x, int y){

    int map_pos = (y - 1) * MAP_SIDE + x - 1;
    int boat_type = map[map_pos] - 1;
    char boat_killed = 0;
    
    if (map[map_pos] > 0){
        hits_per_boat[boat_type]++;
        if (hits_per_boat[boat_type] >= boats_size[boat_type]){
            mvCursor(60, 8);
            // printf("You've lost a boat!");
            boat_killed = 1;
        }
    }

    int cur_x = 60;
    const int yTarget = y * vsteps + MAP_SIDE * vsteps + 1 + yTOP + 3;
    const int xTarget = x * hsteps;
    // const int xTarget = MAP_SIDE * hsteps + 3;

    char color = 0;
    const int timer = 2800000 / (cur_x - xTarget);

    if (!boat_killed){
        while (cur_x >= xTarget){
        
            // Redraw the map where the shot was fired
            for(int i = 0; i < MAP_SIDE; ++i){
                mvCursor(i * hsteps + 3, yTarget);
                if (map[(y - 1) * MAP_SIDE + i] == 0){
                    printf("|   |");
                } else if (map[(y - 1) * MAP_SIDE + i] > 10){
                    printf("|%s[x]%s|", BOLD_HI_RED, RESET);
                } else if (map[(y - 1) * MAP_SIDE + i] > 0){
                    printf("|%s[x]%s|", BOLD_YELLOW, RESET);
                } else if (map[(y - 1) * MAP_SIDE + i] == -1){
                    printf("|%s @ %s|", BOLD_CYAN, RESET);
                }
            }
            printf("    ");

            // Draw the bomb
            if (cur_x != xTarget){
                mvCursor(cur_x, yTarget);
                if (color == 0){
                    printf("%s<=%s<%s<%s ", BOLD_HI_WHITE, BOLD_YELLOW, BOLD_HI_RED, RESET);
                    color = 1;
                } else {
                    printf("%s<=%s<%s<%s ", BOLD_HI_WHITE, BOLD_HI_RED, BOLD_YELLOW, RESET);
                    color = 0;
                }
                fflush(stdout);
                usleep(timer);
            }
            cur_x--;
        }
    } else {
        anim_explosion();
    }

    if (map[map_pos] > 0){
        if (boat_killed){
            send_message("k");
        }

        map[map_pos] += 10;
        mvCursor(x * hsteps, yTarget);
        printf("%s(@)%s", BOLD_HI_RED, RESET);
        nb_self_touched++;

        for(int i = 0; i < 10; ++i){
            mvCursor(x * hsteps, yTarget);
            printf("%s>%s@%s<%s", BOLD_HI_RED, BOLD_YELLOW, BOLD_HI_RED, RESET);
            fflush(stdout);
            usleep(100000);
            mvCursor(x * hsteps, yTarget);
            printf("%s @ %s", BOLD_HI_RED, RESET);
            fflush(stdout);
            usleep(100000);
        }
        mvCursor(x * hsteps, yTarget);
        printf("%s[x]%s", BOLD_HI_RED, RESET);

    } else {
        map[map_pos] = -1;
        for(int i = 0; i < 10; ++i){
            mvCursor(x * hsteps, yTarget);
            printf("%s(%s@%s)%s", BOLD_CYAN, BOLD_BLUE, BOLD_CYAN, RESET);
            fflush(stdout);
            usleep(100000);
            mvCursor(x * hsteps, yTarget);
            printf("%s @ %s", BOLD_CYAN, RESET);
            fflush(stdout);
            usleep(100000);
        }
    }
}

int fire(int x, int y){
    if (map_opp[(y - 1) * MAP_SIDE + x - 1] == 0){
        sprintf(msg, "f%d,%d", x - 1, y - 1);
        map_opp[(y - 1) * MAP_SIDE + x - 1] = 1;
        send_message(msg);

        int steps_left = 50;
        int64_t start = millis();
        int64_t last_step = 0;
        const int yLow = 35;
        const int min_sleep = 25;
        int cur_y = yLow;
        int anim_sleep = 400;
        const int target_sleep = 50;
        int64_t last_target = 0;
        int target_step = 0;
        int xBomb = 50;
        while (millis() - start < 3000 || got_answer == 0){
            if (millis() - start > 10000){
                return 0;
            }
            if (millis() - last_step > anim_sleep && cur_y > -9){
                if (cur_y > 0){
                    mvCursor(xBomb, cur_y + 0); printf("  /\\");
                    mvCursor(xBomb, cur_y + 1); printf(" /  \\");
                    mvCursor(xBomb, cur_y + 2); printf(" |  |");
                    mvCursor(xBomb, cur_y + 3); printf(" |  |");
                    mvCursor(xBomb, cur_y + 4); printf("/ == \\");
                    mvCursor(xBomb, cur_y + 5); printf("|/**\\|");
                    if (steps_left % 2 == 0){
                        mvCursor(xBomb, cur_y + 6); printf(" %s*%s* %s* ", BOLD_HI_RED, BOLD_YELLOW, BOLD_HI_RED);
                        mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_YELLOW, BOLD_HI_RED);
                        mvCursor(xBomb, cur_y + 8); printf("   %s*  ", BOLD_YELLOW);
                    } else {
                        mvCursor(xBomb, cur_y + 6); printf(" %s* %s*%s* ", BOLD_HI_YELLOW, BOLD_RED, BOLD_HI_YELLOW);
                        mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_RED, BOLD_HI_YELLOW);
                        mvCursor(xBomb, cur_y + 8); printf("  %s*   ", BOLD_RED);
                    }
                    mvCursor(xBomb, cur_y + 9); printf("       %s", RESET);
                } else if (cur_y > -9){
                    int y = 0;
                    if (cur_y > -1) {mvCursor(xBomb, y++); printf("  /\\");}
                    if (cur_y > -2) {mvCursor(xBomb, y++); printf(" /  \\");}
                    if (cur_y > -3) {mvCursor(xBomb, y++); printf(" |  |");}
                    if (cur_y > -4) {mvCursor(xBomb, y++); printf(" |  |");}
                    if (cur_y > -5) {mvCursor(xBomb, y++); printf("/ == \\");}
                    if (cur_y > -6) {mvCursor(xBomb, y++); printf("|/**\\|");}
                    if (steps_left % 2 == 0){
                        if (cur_y > -7) {mvCursor(xBomb, cur_y + 6); printf(" %s*%s* %s* ", BOLD_HI_RED, BOLD_YELLOW, BOLD_HI_RED);}
                        if (cur_y > -8) {mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_YELLOW, BOLD_HI_RED);}
                        if (cur_y > -9) {mvCursor(xBomb, cur_y + 8); printf("   %s*  ", BOLD_YELLOW);}
                    } else {
                        if (cur_y > -7) {mvCursor(xBomb, cur_y + 6); printf(" %s* %s*%s* ", BOLD_HI_YELLOW, BOLD_RED, BOLD_HI_YELLOW);}
                        if (cur_y > -8) {mvCursor(xBomb, cur_y + 7); printf("  %s*%s*  ", BOLD_RED, BOLD_HI_YELLOW);}
                        if (cur_y > -9) {mvCursor(xBomb, cur_y + 8); printf("  %s*   ", BOLD_RED);}
                    }
                    mvCursor(xBomb, cur_y + 9); printf("       %s", RESET);
                }
                anim_sleep = max(400 - ((yLow - cur_y) * 3000 / yLow), min_sleep);
                cur_y--;
                steps_left--;
                last_step = millis();
                fflush(stdout);
            }
            if (millis() - last_target > target_sleep){
                target_step++;
                last_target = millis();
                if (target_step > 3){
                    target_step = 0;
                }
                resetCursor();
                switch (target_step){
                    case 0: printf("%s |%s", BOLD_HI_RED, RESET); break;
                    case 1: printf("%s /%s", BOLD_HI_MAGENTA, RESET); break;
                    case 2: printf("%s -%s", BOLD_HI_RED, RESET); break;
                    case 3: printf("%s \\%s", BOLD_HI_YELLOW, RESET); break;
                }
                printf("%s", RESET);
                fflush(stdout);
            }
        }
        if (answer_touched){    // ENEMI TOUCHE
            map_opp[(y - 1) * MAP_SIDE + x - 1] = 3;
            for(int i = 0; i < 5; ++i){
                resetCursor();
                printf("%s( )%s", BOLD_HI_RED, RESET);
                fflush(stdout);
                usleep(300000);
                resetCursor();
                printf("%s(@)%s", BOLD_HI_RED, RESET);
                fflush(stdout);
                usleep(300000);
            }
            nb_opp_touched++;
        } else {    // DANS L'EAU
            map_opp[(y - 1) * MAP_SIDE + x - 1] = 2;
            for(int i = 0; i < 5; ++i){
                resetCursor();
                printf("%s( )%s", BOLD_CYAN, RESET);
                fflush(stdout);
                usleep(300000);
                resetCursor();
                printf("%s(@)%s", BOLD_CYAN, RESET);
                fflush(stdout);
                usleep(300000);
            }
        }
        if (check_win()) return 1;
        current_player = opponent_player;
        update_whos_turn();
        return 1;
    }
    return 0;
}

void anim_boat_sinking(int player){
    (void)player;
    int xBoat = 45;
    int yBoat = 14;
    for(int i = 0; i < 10; ++i){
        mvCursor(xBoat, yBoat + i - 1);                     printf("                                       ");
        mvCursor(xBoat, yBoat + 5);                         printf("----------------------------------------");
        if (i < 6) {mvCursor(xBoat + 13, yBoat + 0 + i);                 printf(" |    |    | ");}
        if (i < 5) {mvCursor(xBoat + 12, yBoat + 1 + i);                printf(" )_)  )_)  )_)  ");}
        if (i < 4) {mvCursor(xBoat + 11, yBoat + 2 + i);               printf(" )___))___))___)\\  ");}
        if (i < 3) {mvCursor(xBoat + 9, yBoat + 3 + i);             printf("  )____)____)_____)\\\\    ");}
        if (i < 2) {mvCursor(xBoat + 8, yBoat + 4 + i);             printf(" _____|____|____|____\\\\\\__ ");}
        if (i < 1) {mvCursor(xBoat, yBoat + 5 + i);         printf("---------\\                   /---------");}
        if (i % 2) {mvCursor(xBoat, yBoat + 6);             printf("  ^^^^^ ^^^^^^^^^^^^^^^^^^^^^          ");}
        else       {mvCursor(xBoat, yBoat + 6);             printf("        ^^^^^^^^^^^^^^^^^^^^^ ^^^^^    ");}
                    mvCursor(xBoat, yBoat + 7 + i % 2);     printf("    ^^^^      ^^^^     ^^^    ^^       ");
                    mvCursor(xBoat, yBoat + 8 - i % 2);     printf("         ^^^^      ^^^                 ");
                    // mvCursor(xBoat, yBoat + 9);             printf("");
        fflush(stdout);
        usleep(500000);
    }
    for(int i = 0; i < 5; ++i){
        mvCursor(xBoat, yBoat + 5 + i);                     printf("                                        ");
    }
}

void anim_explosion(){
    int xBomb = 55;
    int cur_y = 0;
    const int maxy = 35;

    mvCursor(xBomb - 6, maxy + 0); printf("                __/___            ");
    mvCursor(xBomb - 6, maxy + 1); printf("          _____/______|           ");
    mvCursor(xBomb - 6, maxy + 2); printf("  _______/_____\\_______\\_____     ");
    mvCursor(xBomb - 6, maxy + 3); printf("  \\              < < <       |    ");
    mvCursor(xBomb - 6, maxy + 4); printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    
    const int sleep_time = 700000 / (maxy - 4);
    for(cur_y = 0; cur_y < maxy - 4; ++cur_y){
        mvCursor(xBomb, cur_y + 0); printf("       ");
        mvCursor(xBomb, cur_y + 1); printf("|\\**/|");
        mvCursor(xBomb, cur_y + 2); printf("\\ == /");
        mvCursor(xBomb, cur_y + 3); printf(" |  |");
        mvCursor(xBomb, cur_y + 4); printf(" |  |");
        mvCursor(xBomb, cur_y + 5); printf(" \\  /");
        mvCursor(xBomb, cur_y + 6); printf("  \\/");
        fflush(stdout);
        usleep(sleep_time);
    }
    for(int i = 0; i < 5; ++i){
        mvCursor(xBomb, maxy - i); printf("       ");
    }

    const int sleep_time2 = 180000;

    char *color;
    for(int i = 0; i < 4; ++i){
        if (i % 2 == 0){
            color = BOLD_HI_WHITE;
        } else {
            color = BOLD_HI_YELLOW;
        }
        printf("%s", color);
        mvCursor(xBomb - 4, maxy + -2); printf("    \\         .  ./     ");
        mvCursor(xBomb - 4, maxy + -1); printf("  \\      .:\";'.:..\"   / ");
        mvCursor(xBomb - 4, maxy + 0); printf("      (M^^.^~~:.'\").    ");
        mvCursor(xBomb - 4, maxy + 1); printf("-   (/  .    . . \\ \\)  -");
        mvCursor(xBomb - 4, maxy + 2); printf("   ((| :. ~ ^  :. .|))  ");
        mvCursor(xBomb - 4, maxy + 3); printf("-   (\\- |  \\ /  |  /)  -");
        printf("%s", RESET);
        fflush(stdout);
        usleep(sleep_time2);
    }
    for(int i = 0; i < 4; ++i){
        if (i % 2 == 0){
            color = BOLD_HI_WHITE;
        } else {
            color = BOLD_HI_YELLOW;
        }
        printf("%s", color);
        mvCursor(xBomb - 4, maxy + -3); printf("     _.-^^---....,,--       ");
        mvCursor(xBomb - 4, maxy + -2); printf(" _--                  --_   ");
        mvCursor(xBomb - 4, maxy + -1); printf("<                        >) ");
        mvCursor(xBomb - 4, maxy + 0); printf("|                         | ");
        mvCursor(xBomb - 4, maxy + 1); printf(" \\._                   _./  ");
        mvCursor(xBomb - 4, maxy + 2);  printf("          | ;  :|           ");
        mvCursor(xBomb - 4, maxy + 3);  printf(" _____.,-#$&$@$#&#~,._____  ");
        printf("%s", RESET);
        fflush(stdout);
        usleep(sleep_time2);
    }
    for(int i = 0; i < 4; ++i){
        if (i % 2 == 0){
            color = BOLD_HI_WHITE;
        } else {
            color = BOLD_HI_YELLOW;
        }
        printf("%s", color);
        mvCursor(xBomb - 4, maxy + -7); printf("     _.-^^---....,,--       ");
        mvCursor(xBomb - 4, maxy + -6); printf(" _--                  --_   ");
        mvCursor(xBomb - 4, maxy + -5); printf("<                        >) ");
        mvCursor(xBomb - 4, maxy + -4); printf("|                         | ");
        mvCursor(xBomb - 4, maxy + -3); printf(" \\._                   _./  ");
        mvCursor(xBomb - 4, maxy + -2); printf("    ```--. . , ; .--'''     ");
        mvCursor(xBomb - 4, maxy + -1); printf("          | |   |           ");
        mvCursor(xBomb - 4, maxy + 0);  printf("       .-=||  | |=-.        ");
        mvCursor(xBomb - 4, maxy + 1);  printf("       `-=#$&@$#=-'         ");
        mvCursor(xBomb - 4, maxy + 2);  printf("          | ;  :|           ");
        mvCursor(xBomb - 4, maxy + 3);  printf(" _____.,-#$&$@$#&#~,._____  ");
        printf("%s", RESET);
        fflush(stdout);
        usleep(sleep_time2);
    }
    for(int i = 0; i < 4; ++i){
        if (i % 2 == 0){
            color = BOLD_HI_WHITE;
        } else {
            color = BOLD_HI_YELLOW;
        }
        printf("%s", color);
        mvCursor(xBomb - 4, maxy + -8); printf("                            ");
        mvCursor(xBomb - 4, maxy + -7); printf("                            ");
        mvCursor(xBomb - 4, maxy + -6); printf("<                        >) ");
        mvCursor(xBomb - 4, maxy + -5); printf("|                         | ");
        mvCursor(xBomb - 4, maxy + -4); printf(" \\._                   _./  ");
        mvCursor(xBomb - 4, maxy + -3); printf("    ```--. . , ; .--'''     ");
        mvCursor(xBomb - 4, maxy + -2); printf("                            ");
        mvCursor(xBomb - 4, maxy + -1); printf("                            ");
        mvCursor(xBomb - 4, maxy + 0);  printf("                            ");
        mvCursor(xBomb - 4, maxy + 1);  printf("                            ");
        mvCursor(xBomb - 4, maxy + 2);  printf("                            ");
        mvCursor(xBomb - 4, maxy + 3);  printf("                            ");
        printf("%s", RESET);
        fflush(stdout);
        usleep(sleep_time2);
    }
    for(int i = maxy - 8; i < maxy + 5; ++i){
        mvCursor(xBomb - 4, i); printf("                                 ");
    }
    fflush(stdout);

}