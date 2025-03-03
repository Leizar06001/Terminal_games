#include "main.h"
#include <stdarg.h>
#include <sys/ioctl.h>

void enable_mouse_tracking() {
    printf("\033[?1000h"); // Active le mode souris (mode basique)
    fflush(stdout);
}

void enable_mouse_tracking_extended() {
    printf("\033[?1003;1006h"); // Active le mode souris (mode étendu)
    fflush(stdout);
}

/*

    printf("\033[?1003h"); // Active le mode souris (mode étendu)
    fflush(stdout);

*/

void disable_mouse_tracking() {
    // printf("\033[?1000l"); // Désactive le mode souris
    printf("\033[?1000l\033[?1002l\033[?1003l\033[?1005l\033[?1006l");
    fflush(stdout);
}

void disable_cursor(){
    printf("\033[?25l");
    fflush(stdout);
}

void enable_cursor(){
    printf("\033[?25h");
    fflush(stdout);
}

void set_raw_mode(struct termios *original) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, original);
    raw = *original;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void mvCursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}


void prtxy(int x, int y, const char *format, ...){
    va_list args;
    va_start(args, format);
    mvCursor(x, y);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

void dprtxy(int x, int y, const char *format, ...){
    if (prt_debug){
        va_list args;
        va_start(args, format);
        mvCursor(x, y);
        vprintf(format, args);
        va_end(args);
        fflush(stdout);
    }
}

void clear_screen() {
    printf("\033[2J");
    fflush(stdout);
}


int init_terminal(t_main *main){
    set_raw_mode(&main->original);
    // enable_mouse_tracking_extended();
    disable_cursor();
    return 0;
}

void restore_terminal(struct termios *original) {
    disable_mouse_tracking();
    enable_cursor();
    tcsetattr(STDIN_FILENO, TCSANOW, original);
}

void get_terminal_size(int *w, int *h){
    struct winsize wsize;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize) == -1) {
        return ;
    }
    *w = wsize.ws_col;
    *h = wsize.ws_row;
}