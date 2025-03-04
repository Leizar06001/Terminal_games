#include "main.h"

const char* reccord_file = "reccord.txt";

int read_reccord_file(t_main *main){
    memset(main->reccord.high_scores, 0, sizeof(main->reccord.high_scores));
    main->reccord.music = true;
    main->reccord.volume = MUSIC_VOLUME;
    main->reccord.first_launch = true;
    main->reccord.alt_fonts = false;

    FILE *file = fopen(reccord_file, "r");
    if (file == NULL){
        write_reccord_file(main);
        return 0;
    }

    main->reccord.first_launch = false;

    char line[256];

    while (fgets(line, sizeof(line), file)){
        char **tokens = split(line, '=');
        if (tokens == NULL) {
            continue;
        }

        if (strcmp(tokens[0], "high_scores") == 0){
            char **scores = split(tokens[1], ',');
            for(int i = 0; i < 10; i++){
                main->reccord.high_scores[i] = atoi(scores[i]);
            }

            for(int i = 0; i < 10; i++){
                free(scores[i]);
            }
            free(scores);

        } else if (strcmp(tokens[0], "music") == 0){
            main->reccord.music = atoi(tokens[1]);
        } else if (strcmp(tokens[0], "volume") == 0){
            main->reccord.volume = atoi(tokens[1]);
        } else if (strcmp(tokens[0], "alt_fonts") == 0){
            main->reccord.alt_fonts = atoi(tokens[1]);
        }

        for(int i = 0; i < 2; i++){
            free(tokens[i]);
        }
        free(tokens);
    }

    fread(&main->reccord, sizeof(t_reccord), 1, file);
    fclose(file);

    return 1;
}

int write_reccord_file(t_main *main){
    FILE *file = fopen(reccord_file, "w");
    if (file == NULL){
        return -1;
    }

    fprintf(file, "high_scores=");
    for(int i = 0; i < 10; i++){
        fprintf(file, "%d", main->reccord.high_scores[i]);
        if (i < 9) fprintf(file, ",");
    }
    fprintf(file, "\n");

    fprintf(file, "music=%d\n", main->reccord.music);
    fprintf(file, "volume=%d\n", main->reccord.volume);
    fprintf(file, "alt_fonts=%d\n", main->reccord.alt_fonts);

    fclose(file);

    return 1;
}

int update_reccord(t_main *main){
    main->reccord.first_launch = false;
    main->reccord.music = main->music;
    main->reccord.volume = main->volume;
    main->reccord.alt_fonts = main->ui.alt_fonts;
    return write_reccord_file(main);
}
