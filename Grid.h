#ifndef GRID_H
#define GRID_H

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "node.h"
#include "astar.h"

typedef struct {
    int    gridWidth;
    int    gridHeight;
    int    nodeWidth;
    int    nodeHeight;
    Node  *grid;
    int  **maze;
    Node  *start;
    Node  *end;
    Node  *lastSelectedNode;
    State  selectedState;
    bool   isRunning;
    AStar  aStar;
    bool   astarReady;
    Node **animClosed;
    int    animClosedSize;
    int    animClosedIdx;
    Node **animPath;
    int    animPathSize;
    int    animPathIdx;
    int    animPhase;
    int    blinkTick;
    bool   pathFound;
    int    pathLength;
} Grid;

#define grid_get(g, i, j) (&(g)->grid[(i) * (g)->gridHeight + (j)])

static void grid_set_start_and_end(Grid *g) {
    g->start = grid_get(g, 1, 1);
    g->start->state = STATE_START;
    g->end = grid_get(g, g->gridWidth - 2, g->gridHeight - 2);
    g->end->state = STATE_END;
}

static void grid_init(Grid *g, int width, int height, int nodeWidth, int nodeHeight) {
    g->gridWidth        = width;
    g->gridHeight       = height;
    g->nodeWidth        = nodeWidth;
    g->nodeHeight       = nodeHeight;
    g->isRunning        = false;
    g->selectedState    = STATE_START;
    g->lastSelectedNode = NULL;
    g->astarReady       = false;
    g->animPhase        = 0;
    g->blinkTick        = 0;
    g->pathFound        = false;
    g->pathLength       = 0;
    g->animClosed       = NULL;
    g->animPath         = NULL;
    g->grid = (Node *)malloc(sizeof(Node) * width * height);
    g->maze = (int **)malloc(sizeof(int *) * width);
    for (int i = 0; i < width; i++)
        g->maze[i] = (int *)calloc(height, sizeof(int));
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            *grid_get(g, i, j) = node_create(
                i * nodeWidth, j * nodeHeight, nodeWidth, nodeHeight, STATE_EMPTY);
    grid_set_start_and_end(g);
    srand((unsigned int)time(NULL));
}

static void grid_clear(Grid *g) {
    for (int i = 0; i < g->gridWidth; i++)
        for (int j = 0; j < g->gridHeight; j++) {
            State s = grid_get(g, i, j)->state;
            if (s == STATE_CLOSED || s == STATE_PATH || s == STATE_OPEN)
                grid_get(g, i, j)->state = STATE_EMPTY;
        }
}

static void grid_reset(Grid *g) {
    for (int i = 0; i < g->gridWidth; i++)
        for (int j = 0; j < g->gridHeight; j++)
            grid_get(g, i, j)->state = STATE_EMPTY;
    grid_set_start_and_end(g);
    g->pathLength = 0;
    g->animPhase  = 0;
    g->isRunning  = false;
}

static void grid_handle_click(Grid *g, int mx, int my, int button) {
    if (g->isRunning) return;
    for (int i = 0; i < g->gridWidth; i++) {
        for (int j = 0; j < g->gridHeight; j++) {
            Node *n = grid_get(g, i, j);
            if (node_contains(n, mx, my)) {
                grid_clear(g);
                if (g->selectedState == STATE_START) {
                    g->start->state = STATE_EMPTY;
                    g->start = n;
                    n->state = STATE_START;
                } else if (g->selectedState == STATE_END) {
                    g->end->state = STATE_EMPTY;
                    g->end = n;
                    n->state = STATE_END;
                } else if (g->selectedState == STATE_WALL) {
                    if (n != g->lastSelectedNode) {
                        if (button == SDL_BUTTON_LEFT && n->state == STATE_EMPTY)
                            n->state = STATE_WALL;
                        else if (n->state == STATE_WALL)
                            n->state = STATE_EMPTY;
                        g->lastSelectedNode = n;
                    }
                }
                return;
            }
        }
    }
}

static void grid_handle_drag(Grid *g, int mx, int my, Uint32 btnState) {
    if (g->isRunning) return;
    int button = (btnState & SDL_BUTTON_LMASK) ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
    grid_handle_click(g, mx, my, button);
}

static void shuffle_dirs(int *dirs, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = dirs[i]; dirs[i] = dirs[j]; dirs[j] = t;
    }
}

typedef struct { int row, col; } MazeCell;

static void maze_recursion(Grid *g, int startRow, int startCol) {
    MazeCell *stack = (MazeCell *)malloc(sizeof(MazeCell) * g->gridWidth * g->gridHeight);
    int top = 0;
    stack[top].row = startRow;
    stack[top].col = startCol;
    top++;
    while (top > 0) {
        int row = stack[top - 1].row;
        int col = stack[top - 1].col;
        int dirs[] = {1, 2, 3, 4};
        shuffle_dirs(dirs, 4);
        bool moved = false;
        for (int d = 0; d < 4; d++) {
            switch (dirs[d]) {
                case 1:
                    if (row - 2 <= 0) continue;
                    if (g->maze[row-2][col] != 0) {
                        g->maze[row-2][col] = 0; g->maze[row-1][col] = 0;
                        stack[top].row = row-2; stack[top].col = col; top++;
                        moved = true;
                    }
                    break;
                case 2:
                    if (col + 2 >= g->gridHeight - 1) continue;
                    if (g->maze[row][col+2] != 0) {
                        g->maze[row][col+2] = 0; g->maze[row][col+1] = 0;
                        stack[top].row = row; stack[top].col = col+2; top++;
                        moved = true;
                    }
                    break;
                case 3:
                    if (row + 2 >= g->gridWidth - 1) continue;
                    if (g->maze[row+2][col] != 0) {
                        g->maze[row+2][col] = 0; g->maze[row+1][col] = 0;
                        stack[top].row = row+2; stack[top].col = col; top++;
                        moved = true;
                    }
                    break;
                case 4:
                    if (col - 2 <= 0) continue;
                    if (g->maze[row][col-2] != 0) {
                        g->maze[row][col-2] = 0; g->maze[row][col-1] = 0;
                        stack[top].row = row; stack[top].col = col-2; top++;
                        moved = true;
                    }
                    break;
            }
            if (moved) break;
        }
        if (!moved) top--;
    }
    free(stack);
}

static void grid_generate_maze(Grid *g) {
    if (g->isRunning) return;
    g->isRunning = true;
    grid_reset(g);
    for (int i = 0; i < g->gridWidth; i++)
        for (int j = 0; j < g->gridHeight; j++)
            g->maze[i][j] = 1;
    int randomRow = rand() % g->gridWidth;
    while (randomRow % 2 == 0) randomRow = rand() % g->gridWidth;
    int randomCol = rand() % g->gridHeight;
    while (randomCol % 2 == 0) randomCol = rand() % g->gridHeight;
    g->maze[randomRow][randomCol] = 0;
    maze_recursion(g, randomRow, randomCol);
    for (int i = 0; i < g->gridWidth; i++)
        for (int j = 0; j < g->gridHeight; j++)
            if (g->maze[i][j] == 1)
                grid_get(g, i, j)->state = STATE_WALL;
    grid_set_start_and_end(g);
    g->isRunning = false;
}

static void grid_find_path(Grid *g) {
    if (g->isRunning) return;
    g->isRunning = true;
    grid_clear(g);
    g->aStar = astar_create(g->grid, g->start, g->end, g->gridWidth, g->gridHeight);
    astar_findPath(&g->aStar);
    g->astarReady     = true;
    g->pathFound      = g->aStar.pathFound;
    g->pathLength     = g->aStar.pathLength;
    g->animClosed     = g->aStar.closedList;
    g->animClosedSize = g->aStar.closedSize;
    g->animClosedIdx  = 0;
    g->animPath       = g->aStar.path;
    g->animPathSize   = g->aStar.pathSize;
    g->animPathIdx    = 0;
    g->animPhase = 1;
    g->blinkTick = 0;
}

static void grid_step(Grid *g) {
    if (!g->isRunning) return;
    if (g->animPhase == 1) {
        for (int i = 0; i < 3 && g->animClosedIdx < g->animClosedSize; i++) {
            Node *n = g->animClosed[g->animClosedIdx++];
            if (n->state != STATE_START && n->state != STATE_END)
                n->state = STATE_CLOSED;
        }
        if (g->animClosedIdx >= g->animClosedSize)
            g->animPhase = g->pathFound ? 2 : 3;
    } else if (g->animPhase == 2) {
        if (g->animPathIdx < g->animPathSize) {
            g->animPath[g->animPathIdx++]->state = STATE_PATH;
        } else {
            g->animPhase = 3;
        }
    } else if (g->animPhase == 3) {
        g->isRunning = false;
        g->animPhase = 0;
    }
}

static void grid_draw(Grid *g, SDL_Renderer *ren) {
    for (int i = 0; i < g->gridWidth; i++) {
        for (int j = 0; j < g->gridHeight; j++) {
            Node *n = grid_get(g, i, j);
            NodeColor c = node_get_color(n->state);
            SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, 255);
            SDL_Rect rect = {i * n->nodeWidth + 1, j * n->nodeHeight + 1,
                             n->nodeWidth - 2, n->nodeHeight - 2};
            SDL_RenderFillRect(ren, &rect);
            if (n->state != STATE_WALL) {
                SDL_SetRenderDrawColor(ren, 178, 178, 178, 255);
                SDL_Rect border = {i * n->nodeWidth, j * n->nodeHeight,
                                   n->nodeWidth, n->nodeHeight};
                SDL_RenderDrawRect(ren, &border);
            }
        }
    }
}

static void grid_free(Grid *g) {
    free(g->grid);
    for (int i = 0; i < g->gridWidth; i++)
        free(g->maze[i]);
    free(g->maze);
    if (g->astarReady)
        astar_free(&g->aStar);
}

#endif /* GRID_H */