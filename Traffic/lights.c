#include "main.h"

#define LIGHT_RR    0
#define LIGHT_RG    1
#define LIGHT_RY    2
#define LIGHT_RR2   3
#define LIGHT_GR    4
#define LIGHT_YR    5

const int lights_times[6] = {
    2,
    70,
    4,
    2,
    70,
    4
};

const char *lights_colors[6][2] = {
    {RED, RED},
    {RED, GREEN},
    {RED, YELLOW},
    {RED, RED},
    {GREEN, RED},
    {YELLOW, RED}
};



void draw_light_colors(t_light *light, const char *color1, const char *color2){
    int mx, my;
    mapToMousePos(light->x, light->y, &mx, &my);
    prtxy(mx + 4, my,       "%s⬤", color1);
    prtxy(mx, my + 3,       "⬤");
    prtxy(mx, my,           "%s⬤", color2);
    prtxy(mx + 4, my + 3,   "⬤%s", RESET);
}

void update_lights(t_main *main){

    t_light_list *lights = main->lights;

    while (lights != NULL) {
        t_light *light = lights->light;

        bool has_changed = false;
        if (light->enabled){
            if (light->elapsed >= lights_times[light->state]){
                light->elapsed = 0;
                light->state++;
                if (light->state > 5){
                    light->state = 0;
                }
                has_changed = true;
            }

            if (has_changed){   // redraw
                draw_light_colors(light, lights_colors[light->state][0], lights_colors[light->state][1]);
            }

            light->elapsed++;
        }

        lights = lights->next;
    }

}