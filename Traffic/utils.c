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