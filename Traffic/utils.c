#define _XOPEN_SOURCE 700       // For wcwidth
#define _DEFAULT_SOURCE         // To get usleep back

#include "main.h"

static int count_tokens(const char *str, char delim) {
    if (!str) return 0;
    if (*str == '\0') return 1;  // Ensure empty strings return 1 token

    int count = 1;
    while (*str) {
        if (*str == delim) count++;
        str++;
    }
    return count;
}

// Custom split function (no strtok)
char **split(const char *str, char delim) {
    if (!str || *str == '\0') return NULL;

    int num_tokens = count_tokens(str, delim);
    char **tokens = malloc((num_tokens + 1) * sizeof(char *));
    if (!tokens) return NULL;

    int i = 0, start = 0;
    int len = strlen(str);

    for (int j = 0; j <= len; j++) {
        if (str[j] == delim || str[j] == '\0') {
            int word_len = j - start;
            tokens[i] = malloc(word_len + 1);  // Ensure memory allocation
            if (!tokens[i]) {
                for (int k = 0; k < i; k++) free(tokens[k]);
                free(tokens);
                return NULL;
            }

            if (word_len > 0) {
                strncpy(tokens[i], &str[start], word_len);
                tokens[i][word_len] = '\0';  // Null terminate
            } else {
                tokens[i][0] = '\0';  // Handle empty token
            }
            i++;
            start = j + 1;
        }
    }

    tokens[i] = NULL;  // Null-terminate the array
    return tokens;
}

// get time in millis
uint64_t millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int get_random(int max){
    return rand() % (max + 1);
}

int get_random_range(int min, int max){
    return rand() % (max - min + 1) + min;
}

// FONT TEST

int is_arrow_supported()
{
    // Set locale from environment. For most terminals using UTF-8,
    // you'll want LC_ALL or LANG to be something like en_US.UTF-8.
    if (!setlocale(LC_ALL, "")) {
        fprintf(stderr, "Warning: could not set locale from environment.\n");
        return 0;
    }

    // The character we want to check
    wchar_t arrow = L'⮟';

    int width = wcwidth(arrow);
    // wcwidth() returns:
    //  -1 if the character is not printable (unrecognized in this locale)
    //   0 if the character is a zero-width character
    //   1 or 2 (etc.) if the character takes up space on the screen
    if (width > 0) {
        return 1;
    } else {
        return 0;
    }
}

void select_charset(t_main *main) {
    if (main->ui.alt_fonts == 0) {
        main->cars_charset = cars_charset_extended;
    } else {
        main->cars_charset = cars_charset;
    }
}