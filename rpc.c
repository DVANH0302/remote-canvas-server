#include "rpc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <animate/animate.h>
#include "barrier.h"

static void rpc_create_canvas(struct Client *client, char *response) {
    char *h = strtok(NULL, " ");
    char *w = strtok(NULL, " ");
    char *c = strtok(NULL, " ");
    if (h == NULL || w == NULL || c == NULL) {
        snprintf(response, 256, "-2\n"); return;
    }
    size_t height = (size_t)atoi(h);
    size_t width  = (size_t)atoi(w);

    // DEBUG
    printf("DEBUG canvas: width=%zu height=%zu\n", width, height);
    fflush(stdout);
    //


    uint32_t color = (uint32_t)strtoul(c, NULL, 10);
    struct canvas *cv = animate_create_canvas(height, width, color);
    if (cv == NULL) { snprintf(response, 256, "-3\n"); return; }
    client->canvases[client->num_canvases] = cv;
    int handle = client->num_canvases + 1;
    client->canvas_widths[client->num_canvases] = width;
    client->canvas_heights[client->num_canvases] = height;
    client->canvas_owned[client->num_canvases] = 1; 
    client->num_canvases++;

    snprintf(response, 256, "0 %d\n", handle);
}

static void rpc_create_rectangle(struct Client *client, char *response) {
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
}

static void rpc_create_circle(struct Client *client, char *response) {
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
}

static void rpc_create_sprite(struct Client *client, char *response) {
    char *file = strtok(NULL, " ");
    if (file == NULL) { snprintf(response, 256, "-2\n"); return; }
    struct sprite *sp = animate_create_sprite(file);
    if (sp == NULL) { snprintf(response, 256, "-3\n"); return; }
    client->sprites[client->num_sprites] = sp;
    int handle = client->num_sprites + 1;
    client->num_sprites++;
    snprintf(response, 256, "0 %d\n", handle);
}

static void rpc_place_sprite(struct Client *client, char *response) {
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
}

static void rpc_destroy_canvas(struct Client *client, char *response) {
    char *cv_h = strtok(NULL, " ");
    if (cv_h == NULL) { snprintf(response, 256, "-2\n"); return; }
    int cv_handle = atoi(cv_h);
    if (cv_handle < 1 || cv_handle > client->num_canvases) {
        snprintf(response, 256, "-2\n"); return;
    }
    if (!client->canvas_owned[cv_handle - 1]) {
        snprintf(response, 256, "-2\n"); 
        return;
    }

    animate_destroy_canvas(client->canvases[cv_handle - 1]);
    client->canvases[cv_handle - 1] = NULL;
    snprintf(response, 256, "0\n");
}

static void rpc_destroy_sprite(struct Client *client, char *response) {
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
}

static void rpc_destroy_placement(struct Client *client, char *response) {
    char *pl_h = strtok(NULL, " ");
    if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
    int pl_handle = atoi(pl_h);
    if (pl_handle < 1 || pl_handle > client->num_placements) {
        snprintf(response, 256, "-2\n"); return;
    }
    animate_destroy_placement(client->placements[pl_handle - 1]);
    client->placements[pl_handle - 1] = NULL;
    snprintf(response, 256, "0\n");
}

static void rpc_placement_up(struct Client *client, char *response) {
    char *pl_h = strtok(NULL, " ");
    if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
    int pl_handle = atoi(pl_h);
    if (pl_handle < 1 || pl_handle > client->num_placements) {
        snprintf(response, 256, "-2\n"); return;
    }
    animate_placement_up(client->placements[pl_handle - 1]);
    snprintf(response, 256, "0\n");
}

static void rpc_placement_down(struct Client *client, char *response) {
    char *pl_h = strtok(NULL, " ");
    if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
    int pl_handle = atoi(pl_h);
    if (pl_handle < 1 || pl_handle > client->num_placements) {
        snprintf(response, 256, "-2\n"); return;
    }
    animate_placement_down(client->placements[pl_handle - 1]);
    snprintf(response, 256, "0\n");
}

static void rpc_placement_top(struct Client *client, char *response) {
    char *pl_h = strtok(NULL, " ");
    if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
    int pl_handle = atoi(pl_h);
    if (pl_handle < 1 || pl_handle > client->num_placements) {
        snprintf(response, 256, "-2\n"); return;
    }
    animate_placement_top(client->placements[pl_handle - 1]);
    snprintf(response, 256, "0\n");
}

static void rpc_placement_bottom(struct Client *client, char *response) {
    char *pl_h = strtok(NULL, " ");
    if (pl_h == NULL) { snprintf(response, 256, "-2\n"); return; }
    int pl_handle = atoi(pl_h);
    if (pl_handle < 1 || pl_handle > client->num_placements) {
        snprintf(response, 256, "-2\n"); return;
    }
    animate_placement_bottom(client->placements[pl_handle - 1]);
    snprintf(response, 256, "0\n");
}

static void rpc_share_canvas(struct Client *client, char *response) {
    char *cv_h     = strtok(NULL, " ");
    char *username = strtok(NULL, " ");

    if (cv_h == NULL || username == NULL) {
        snprintf(response, 256, "-2\n"); return;
    }

    int cv_handle = atoi(cv_h);
    if (cv_handle < 1 || cv_handle > client->num_canvases ||
        client->canvases[cv_handle - 1] == NULL) {
        snprintf(response, 256, "-2\n"); return;
    }

    struct canvas *cv = client->canvases[cv_handle - 1]; 

    // find other client by username
    struct Client *other = NULL;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].logged_in && 
            strcmp(clients[i].username, username) == 0) {
            other = &clients[i];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (other == NULL) {
        snprintf(response, 256, "-1\n"); return;
    }

    // add canvas to other client's table
    // only adds if other doesn't already have it
    pthread_mutex_lock(&clients_mutex);
    int already_has = 0;
    for (int i = 0; i < other->num_canvases; i++) {
        if (other->canvases[i] == cv) {
            already_has = 1;
            break;
        }
    }
    if (!already_has) {
        other->canvases[other->num_canvases] = cv;
        other->canvas_widths[other->num_canvases] = client->canvas_widths[cv_handle - 1];
        other->canvas_heights[other->num_canvases] = client->canvas_heights[cv_handle - 1];
        other->canvas_owned[other->num_canvases] = 0; 
        other->num_canvases++;
    }
    pthread_mutex_unlock(&clients_mutex);

    // add both to barrier tracking
    barrier_add_client(cv);

    snprintf(response, 256, "0\n");
}



static void rpc_set_animation_params(struct Client *client, char *response) {
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
}

static void rpc_generate(struct Client *client, char *response) {
    printf("DEBUG generate called\n");
    fflush(stdout);
    char *cv_h     = strtok(NULL, " ");
    printf("DEBUG cv_h=%s\n", cv_h ? cv_h : "NULL");
    fflush(stdout);
    char *filename = strtok(NULL, " ");
    char *start_s  = strtok(NULL, " ");
    char *end_s    = strtok(NULL, " ");
    char *fps_s    = strtok(NULL, " ");

    if (cv_h == NULL || filename == NULL || start_s == NULL ||
        end_s == NULL || fps_s == NULL) {
        snprintf(response, 256, "-2\n"); return;
    }

    int cv_handle = atoi(cv_h);
    if (cv_handle < 1 || cv_handle > client->num_canvases ||
        client->canvases[cv_handle - 1] == NULL) {
        snprintf(response, 256, "-2\n"); return;
    }

    struct canvas *cv = client->canvases[cv_handle - 1];
    size_t start = (size_t)atoi(start_s);
    size_t end   = (size_t)atoi(end_s);
    size_t fps   = (size_t)atoi(fps_s);

    // allocate frame buffer
    size_t frame_size = animate_frame_size_bytes(cv);
    void *buf = malloc(frame_size);
    if (buf == NULL) {
        snprintf(response, 256, "-3\n"); return;
    }

    // write dat file
    char dat_file[300];
    snprintf(dat_file, sizeof(dat_file), "%s.dat", filename);
    FILE *dat = fopen(dat_file, "wb");
    if (dat == NULL) {
        free(buf);
        snprintf(response, 256, "0 -1\n"); return;
    }

    for (size_t i = start; i <= end; i++) {
        animate_generate_frame(cv, i, fps, buf);
        if (fwrite(buf, frame_size, 1, dat) != 1) {
            fclose(dat);
            free(buf);
            snprintf(response, 256, "0 -1\n"); return;
        }
    }
    fclose(dat);
    free(buf);

    // get canvas dimensions
    size_t width  = client->canvas_widths[cv_handle - 1];
    size_t height = client->canvas_heights[cv_handle - 1];

    // DEBUG
    printf("DEBUG generate: cv=%d width=%zu height=%zu\n", cv_handle, width, height);
    fflush(stdout);
    //


    // build ffmpeg command
    char mp4_file[300];
    char log_file[300];
    snprintf(mp4_file, sizeof(mp4_file), "%s.mp4", filename);
    snprintf(log_file, sizeof(log_file), "%s.log", filename);

    char ffmpeg_cmd[1024];
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd),
        "ffmpeg -f rawvideo -pix_fmt bgra -s %zux%zu -r %zu -i %s -c:v libx264 -y %s 2>&1",
        width, height, fps, dat_file, mp4_file);

    // run ffmpeg and capture output to log
    printf("DEBUG ffmpeg cmd: %s\n", ffmpeg_cmd);
    fflush(stdout);
    FILE *fp = popen(ffmpeg_cmd, "r");
    printf("DEBUG fp=%p\n", (void*)fp);
    fflush(stdout);
    if (fp == NULL) {
        snprintf(response, 256, "0 0 -1\n");
        return;
    }

    FILE *log = fopen(log_file, "w");
    if (log == NULL) {
        pclose(fp);
        snprintf(response, 256, "0 0 -1\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        fputs(line, log);
    }

    fclose(log);
    int ret = pclose(fp);
    if (ret != 0) {
        snprintf(response, 256, "0 0 -1\n");
        return;
    }

    snprintf(response, 256, "0 0 0\n");
}


static void rpc_barrier(struct Client *client, char *response) {
    char *cv_h = strtok(NULL, " ");
    if (cv_h == NULL) { snprintf(response, 256, "-2\n"); return; }

    int cv_handle = atoi(cv_h);
    if (cv_handle < 1 || cv_handle > client->num_canvases ||
        client->canvases[cv_handle - 1] == NULL) {
        snprintf(response, 256, "-2\n"); return;
    }

    barrier_wait(client->canvases[cv_handle - 1], response);
}

void handle_rpc(struct Client *client, char *cmd, char *response) {
    if      (strcmp(cmd, "create_canvas") == 0)        rpc_create_canvas(client, response);
    else if (strcmp(cmd, "create_rectangle") == 0)     rpc_create_rectangle(client, response);
    else if (strcmp(cmd, "create_circle") == 0)        rpc_create_circle(client, response);
    else if (strcmp(cmd, "create_sprite") == 0)        rpc_create_sprite(client, response);
    else if (strcmp(cmd, "place_sprite") == 0)         rpc_place_sprite(client, response);
    else if (strcmp(cmd, "destroy_canvas") == 0)       rpc_destroy_canvas(client, response);
    else if (strcmp(cmd, "destroy_sprite") == 0)       rpc_destroy_sprite(client, response);
    else if (strcmp(cmd, "destroy_placement") == 0)    rpc_destroy_placement(client, response);
    else if (strcmp(cmd, "placement_up") == 0)         rpc_placement_up(client, response);
    else if (strcmp(cmd, "placement_down") == 0)       rpc_placement_down(client, response);
    else if (strcmp(cmd, "placement_top") == 0)        rpc_placement_top(client, response);
    else if (strcmp(cmd, "placement_bottom") == 0)     rpc_placement_bottom(client, response);
    else if (strcmp(cmd, "set_animation_params") == 0) rpc_set_animation_params(client, response);
    else if (strcmp(cmd, "generate") == 0)             rpc_generate(client, response);
    else if (strcmp(cmd, "share_canvas") == 0)         rpc_share_canvas(client, response);
    else if (strcmp(cmd, "barrier") == 0)              rpc_barrier(client, response);

    else snprintf(response, 256, "-1\n");
}