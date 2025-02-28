#include "main.h"
#include <stdarg.h>

#define DEBUG_X 200
#define DEBUG_Y 10
#define DEBUG_H 40
#define DEBUG_W 50
#define MAX_LEN 4096

static char str[MAX_LEN];
static int str_len = 0;

void add_debug(const char *format, ...) {
    if (!prt_debug) return;
    va_list args;
    va_start(args, format);
    str_len += vsnprintf(str + str_len, MAX_LEN - str_len, format, args);
    va_end(args);
}

// We must not print '\n' but change the line y when we encounter it
void print_debug() {
    if (!prt_debug) return;

    int x = DEBUG_X;
    int y = DEBUG_Y;
    int i = 0;
    int start = 0;
    char tmp[512];

    while (i < str_len) {
        if (str[i] == '\n') {
            // Copy the string to tmp and fill until DEBUG_W with spaces
            strncpy(tmp, str + start, i - start);
            int len = strlen(tmp);
            for (int j = len; j < DEBUG_W; j++) {
                tmp[j] = ' ';
            }
            tmp[DEBUG_W] = '\0';

            prtxy(x, y, "%s", tmp);

            y++;
            start = i + 1;
        }
        i++;
    }
    // Print the last line
    strncpy(tmp, str + start, i - start);
    int len = strlen(tmp);
    for (int j = len; j < DEBUG_W; j++) {
        tmp[j] = ' ';
    }
    tmp[DEBUG_W] = '\0';
    prtxy(x, y, "%s", tmp);

    str_len = 0;
}