#ifndef MAIN_H
#define MAIN_H

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "node.h"
#include "grid.h"

#define WINDOW_WIDTH      800
#define WINDOW_HEIGHT     600
#define SIDE_PANEL_WIDTH  150
#define NODE_WIDTH         25
#define NODE_HEIGHT        25
#define GRID_WIDTH        ((WINDOW_WIDTH - SIDE_PANEL_WIDTH) / NODE_WIDTH - 1)
#define GRID_HEIGHT       (WINDOW_HEIGHT / NODE_HEIGHT - 3)
#define BTN_W             (SIDE_PANEL_WIDTH - 20)
#define BTN_H              35
#define BTN_MARGIN         10
#define BTN_X             ((WINDOW_WIDTH - SIDE_PANEL_WIDTH) + 10)

typedef struct {
    SDL_Rect rect;
    char     label[32];
    bool     selected;
    bool     hovered;
} Button;

typedef struct {
    Button actionBtns[4];
    int    actionCount;
    Button nodeBtns[3];
    int    nodeCount;
    int    selectedNodeBtn;
    int    pathLength;
} UI;

static void ui_init(UI *ui) {
    const char *actions[] = {"Visualize Path", "Generate Maze", "Clear Path", "Reset Grid"};
    ui->actionCount = 4;
    for (int i = 0; i < ui->actionCount; i++) {
        ui->actionBtns[i].rect     = (SDL_Rect){BTN_X, 40 + i * (BTN_H + BTN_MARGIN), BTN_W, BTN_H};
        ui->actionBtns[i].selected = false;
        ui->actionBtns[i].hovered  = false;
        strncpy(ui->actionBtns[i].label, actions[i], 31);
    }
    const char *nodes[] = {"Start", "End", "Wall"};
    ui->nodeCount = 3;
    for (int i = 0; i < ui->nodeCount; i++) {
        int yBase = 40 + ui->actionCount * (BTN_H + BTN_MARGIN) + 60;
        ui->nodeBtns[i].rect     = (SDL_Rect){BTN_X, yBase + i * (BTN_H + BTN_MARGIN), BTN_W, BTN_H};
        ui->nodeBtns[i].selected = false;
        ui->nodeBtns[i].hovered  = false;
        strncpy(ui->nodeBtns[i].label, nodes[i], 31);
    }
    ui->selectedNodeBtn = 0;
    ui->nodeBtns[0].selected = true;
    ui->pathLength = 0;
}

/* Draw text centered in a rect */
static void draw_text(SDL_Renderer *ren, TTF_Font *font, const char *text,
                      SDL_Rect *rect, SDL_Color color) {
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    if (tex) {
        int tw = surf->w, th = surf->h;
        SDL_Rect dst = {
            rect->x + (rect->w - tw) / 2,
            rect->y + (rect->h - th) / 2,
            tw, th
        };
        SDL_RenderCopy(ren, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

static void draw_button(SDL_Renderer *ren, TTF_Font *font, Button *btn) {
    if (btn->hovered)
        SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
    else
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderFillRect(ren, &btn->rect);

    if (btn->selected) {
        SDL_SetRenderDrawColor(ren, 255, 165, 0, 255);
        for (int t = 0; t < 2; t++) {
            SDL_Rect border = {btn->rect.x - t, btn->rect.y - t,
                               btn->rect.w + 2*t, btn->rect.h + 2*t};
            SDL_RenderDrawRect(ren, &border);
        }
    }

    SDL_Color black = {0, 0, 0, 255};
    draw_text(ren, font, btn->label, &btn->rect, black);
}

static void draw_label(SDL_Renderer *ren, TTF_Font *font, const char *text,
                       int x, int y) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Rect r = {x, y, SIDE_PANEL_WIDTH - 20, 20};
    draw_text(ren, font, text, &r, white);
}

static void ui_draw(SDL_Renderer *ren, TTF_Font *font, UI *ui) {
    SDL_SetRenderDrawColor(ren, 30, 33, 36, 255);
    SDL_Rect panel = {WINDOW_WIDTH - SIDE_PANEL_WIDTH, 0, SIDE_PANEL_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(ren, &panel);

    draw_label(ren, font, "-- Actions --", BTN_X, 15);
    for (int i = 0; i < ui->actionCount; i++)
        draw_button(ren, font, &ui->actionBtns[i]);

    int divY = ui->nodeBtns[0].rect.y - 25;
    draw_label(ren, font, "-- Nodes --", BTN_X, divY);
    for (int i = 0; i < ui->nodeCount; i++)
        draw_button(ren, font, &ui->nodeBtns[i]);

    /* Path length */
    char buf[32];
    snprintf(buf, sizeof(buf), "Path: %d", ui->pathLength);
    draw_label(ren, font, buf, BTN_X, ui->nodeBtns[ui->nodeCount-1].rect.y + 55);
}

static bool point_in_rect(int x, int y, SDL_Rect *r) {
    return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h;
}

static const char *ui_handle_click(UI *ui, int mx, int my, Grid *grid) {
    for (int i = 0; i < ui->actionCount; i++) {
        if (point_in_rect(mx, my, &ui->actionBtns[i].rect)) {
            if (!grid->isRunning) {
                ui->pathLength = 0;
                return ui->actionBtns[i].label;
            }
        }
    }
    for (int i = 0; i < ui->nodeCount; i++) {
        if (point_in_rect(mx, my, &ui->nodeBtns[i].rect)) {
            ui->nodeBtns[ui->selectedNodeBtn].selected = false;
            ui->selectedNodeBtn = i;
            ui->nodeBtns[i].selected = true;
            if (i == 0)      grid->selectedState = STATE_START;
            else if (i == 1) grid->selectedState = STATE_END;
            else             grid->selectedState = STATE_WALL;
            return NULL;
        }
    }
    return NULL;
}

static void ui_handle_hover(UI *ui, int mx, int my) {
    for (int i = 0; i < ui->actionCount; i++)
        ui->actionBtns[i].hovered = point_in_rect(mx, my, &ui->actionBtns[i].rect);
    for (int i = 0; i < ui->nodeCount; i++)
        ui->nodeBtns[i].hovered = point_in_rect(mx, my, &ui->nodeBtns[i].rect);
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        return 1;
    }

    /* Load a font — uses a Windows system font */
    TTF_Font *font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 11);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont error: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Path Finding Visualizer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Grid grid;
    grid_init(&grid, GRID_WIDTH, GRID_HEIGHT, NODE_WIDTH, NODE_HEIGHT);

    UI ui;
    ui_init(&ui);

    bool running = true;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEBUTTONDOWN: {
                    int mx = ev.button.x, my = ev.button.y;
                    if (mx >= WINDOW_WIDTH - SIDE_PANEL_WIDTH) {
                        const char *action = ui_handle_click(&ui, mx, my, &grid);
                        if (action) {
                            if (strcmp(action, "Visualize Path") == 0) {
                                grid_find_path(&grid);
                                ui.pathLength = grid.pathLength;
                            } else if (strcmp(action, "Generate Maze") == 0) {
                                grid_generate_maze(&grid);
                            } else if (strcmp(action, "Clear Path") == 0) {
                                grid_clear(&grid);
                            } else if (strcmp(action, "Reset Grid") == 0) {
                                grid_reset(&grid);
                                ui.pathLength = 0;
                            }
                        }
                    } else {
                        grid_handle_click(&grid, mx, my, ev.button.button);
                    }
                    break;
                }

                case SDL_MOUSEMOTION: {
                    int mx = ev.motion.x, my = ev.motion.y;
                    ui_handle_hover(&ui, mx, my);
                    if (ev.motion.state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
                        if (mx < WINDOW_WIDTH - SIDE_PANEL_WIDTH)
                            grid_handle_drag(&grid, mx, my, ev.motion.state);
                    break;
                }

                case SDL_KEYDOWN:
                    if (ev.key.keysym.sym == SDLK_ESCAPE) running = false;
                    break;
            }
        }

        if (grid.isRunning)
            grid_step(&grid);

        SDL_SetRenderDrawColor(ren, 242, 242, 242, 255);
        SDL_RenderClear(ren);
        grid_draw(&grid, ren);
        ui_draw(ren, font, &ui);
        SDL_RenderPresent(ren);
    }

    grid_free(&grid);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

#endif /* MAIN_H */