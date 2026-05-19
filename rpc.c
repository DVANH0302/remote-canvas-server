#include "rpc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <animate/animate.h>

void handle_rpc(struct Client *client, char *cmd, char *response) {
    if (strcmp(cmd, "create_canvas") == 0) {
        char *h = strtok(NULL, " ");
        char *w = strtok(NULL, " ");
        char *c = strtok(NULL, " ");
        if (h == NULL || w == NULL || c == NULL) {
            snprintf(response, 256, "-2\n"); return;
        }
        size_t height = (size_t)atoi(h);
        size_t width  = (size_t)atoi(w);
        uint32_t color = (uint32_t)strtoul(c, NULL, 10);
        struct canvas *cv = animate_create_canvas(height, width, color);
        if (cv == NULL) { snprintf(response, 256, "-3\n"); return; }
        client->canvases[client->num_canvases] = cv;
        int handle = client->num_canvases + 1;
        client->num_canvases++;
        snprintf(response, 256, "0 %d\n", handle);

    } else if (strcmp(cmd, "create_rectangle") == 0) {
        char *w = strtok(NULL, " ");
        char *h = strtok(NULL, " ");
        char *c = strtok(NULL, " ");
        char *f = strtok(NULL, " ");
        if (w == NULL || h == NULL || c == NULL || f == NULL) {
            snprintf(response, 256, "-2\n"); return;
        }
        size_t width  = (size_t)atoi(w);
        size_t height = (size_t)atoi(h);
        uint32_t color = (uint32_t)strtoul(c, NULL, 10);
        bool filled = atoi(f);
        struct sprite *sp = animate_create_rectangle(width, height, color, filled);
        if (sp == NULL) { snprintf(response, 256, "-3\n"); return; }
        client->sprites[client->num_sprites] = sp;
        int handle = client->num_sprites + 1;
        client->num_sprites++;
        snprintf(response, 256, "0 %d\n", handle);

    } else if (strcmp(cmd, "create_circle") == 0) {
        char *r = strtok(NULL, " ");
        char *c = strtok(NULL, " ");
        char *f = strtok(NULL, " ");
        if (r == NULL || c == NULL || f == NULL) {
            snprintf(response, 256, "-2\n"); return;
        }
        size_t radius = (size_t)atoi(r);
        uint32_t color = (uint32_t)strtoul(c, NULL, 10);
        bool filled = atoi(f);
        struct sprite *sp = animate_create_circle(radius, color, filled);
        if (sp == NULL) { snprintf(response, 256, "-3\n"); return; }
        client->sprites[client->num_sprites] = sp;
        int handle = client->num_sprites + 1;
        client->num_sprites++;
        snprintf(response, 256, "0 %d\n", handle);

    } else if (strcmp(cmd, "create_sprite") == 0) {
        char *file = strtok(NULL, " ");
        if (file == NULL) { snprintf(response, 256, "-2\n"); return; }
        struct sprite *sp = animate_create_sprite(file);
        if (sp == NULL) { snprintf(response, 256, "-3\n"); return; }
        client->sprites[client->num_sprites] = sp;
        int handle = client->num_sprites + 1;
        client->num_sprites++;
        snprintf(response, 256, "0 %d\n", handle);

    } else if (strcmp(cmd, "place_sprite") == 0) {
        char *cv_h = strtok(NULL, " ");
        char *sp_h = strtok(NULL, " ");
        char *x    = strtok(NULL, " ");
        char *y    = strtok(NULL, " ");
        if (cv_h == NULL || sp_h == NULL || x == NULL || y == NULL) {
            snprintf(response, 256, "-2\n"); return;
        }
        int cv_handle = atoi(cv_h);
        int sp_handle = atoi(sp_h);
        if (cv_handle < 1 || cv_handle > client->num_canvases ||
            sp_handle < 1 || sp_handle > client->num_sprites) {
            snprintf(response, 256, "-2\n"); return;
        }
        struct canvas *cv = client->canvases[cv_handle - 1];
        struct sprite *sp = client->sprites[sp_handle - 1];
        struct sprite_placement *pl = animate_place_sprite(cv, sp, atoi(x), atoi(y));
        if (pl == NULL) { snprintf(response, 256, "-3\n"); return; }
        client->placements[client->num_placements] = pl;
        int handle = client->num_placements + 1;
        client->num_placements++;
        snprintf(response, 256, "0 %d\n", handle);

    } else if (strcmp(cmd, "destroy_canvas") == 0) {
        char *cv_h = strtok(NULL, " ");
        if (cv_h == NULL) { snprintf(response, 256, "-2\n"); return; }
        int cv_handle = atoi(cv_h);
        if (cv_handle < 1 || cv_handle > client->num_canvases) {
            snprintf(response, 256, "-2\n"); return;
        }
        animate_destroy_canvas(client->canvases[cv_handle - 1]);
        client->canvases[cv_handle - 1] = NULL;
        snprintf(response, 256, "0\n");

    } else if (strcmp(cmd, "destroy_sprite") == 0) {
        char *sp_h = strtok(NULL, " ");
        if (sp_h == NULL) { snprintf(response, 256, "-2\n"); return; }
        int sp_handle = atoi(sp_h);
        if (sp_handle < 1 || sp_handle > client->num_sprites) {
            snprintf(response, 256, "-2\n"); return;
        }
        bool result = animate_destroy_sprite(client->sprites[sp_handle - 1]);
        if (result != 0) { snprintf(response, 256, "-3\n"); return; }
        client->sprites[sp_handle - 1] = NULL;
        snprintf(response, 256, "0\n");

    } else if (strcmp(cmd, "destroy_placement") == 0) {
        char *pl_h = strtok(NULL, " ");
        if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
        int pl_handle = atoi(pl_h);
        if (pl_handle < 1 || pl_handle > client->num_placements) {
            snprintf(response, 256, "-2\n"); return;
        }
        animate_destroy_placement(client->placements[pl_handle - 1]);
        client->placements[pl_handle - 1] = NULL;
        snprintf(response, 256, "0\n");

    } else if (strcmp(cmd, "placement_up") == 0) {
        char *pl_h = strtok(NULL, " ");
        if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
        int pl_handle = atoi(pl_h);
        if (pl_handle < 1 || pl_handle > client->num_placements) {
            snprintf(response, 256, "-2\n"); return;
        }
        animate_placement_up(client->placements[pl_handle - 1]);
        snprintf(response, 256, "0\n");

    } else if (strcmp(cmd, "placement_down") == 0) {
        char *pl_h = strtok(NULL, " ");
        if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
        int pl_handle = atoi(pl_h);
        if (pl_handle < 1 || pl_handle > client->num_placements) {
            snprintf(response, 256, "-2\n"); return;
        }
        animate_placement_down(client->placements[pl_handle - 1]);
        snprintf(response, 256, "0\n");

    } else if (strcmp(cmd, "placement_top") == 0) {
        char *pl_h = strtok(NULL, " ");
        if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
        int pl_handle = atoi(pl_h);
        if (pl_handle < 1 || pl_handle > client->num_placements) {
            snprintf(response, 256, "-2\n"); return;
        }
        animate_placement_top(client->placements[pl_handle - 1]);
        snprintf(response, 256, "0\n");

    } else if (strcmp(cmd, "placement_bottom") == 0) {
        char *pl_h = strtok(NULL, " ");
        if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
        int pl_handle = atoi(pl_h);
        if (pl_handle < 1 || pl_handle > client->num_placements) {
            snprintf(response, 256, "-2\n"); return;
        }
        animate_placement_bottom(client->placements[pl_handle - 1]);
        snprintf(response, 256, "0\n");

    } else if (strcmp(cmd, "set_animation_params") == 0) {
        char *pl_h = strtok(NULL, " ");
        char *vx   = strtok(NULL, " ");
        char *vy   = strtok(NULL, " ");
        char *ax   = strtok(NULL, " ");
        char *ay   = strtok(NULL, " ");
        if (pl_h == NULL || vx == NULL || vy == NULL ||
            ax == NULL || ay == NULL) {
            snprintf(response, 256, "-2\n"); return;
        }
        int pl_handle = atoi(pl_h);
        if (pl_handle < 1 || pl_handle > client->num_placements) {
            snprintf(response, 256, "-2\n"); return;
        }
        animate_set_animation_params(
            client->placements[pl_handle - 1],
            atoi(vx), atoi(vy), atoi(ax), atoi(ay)
        );
        snprintf(response, 256, "0\n");

    } else {
        snprintf(response, 256, "-1\n");
    }
}