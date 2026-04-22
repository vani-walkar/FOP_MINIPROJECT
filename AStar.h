#ifndef ASTAR_H
#define ASTAR_H

#include "node.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

typedef struct {
    Node  *grid;          /* flat array */
    int    gridWidth;
    int    gridHeight;
    Node  *start;
    Node  *end;
    Node **openList;
    int    openSize;
    int    openCap;
    Node **closedList;
    int    closedSize;
    int    closedCap;
    Node **path;
    int    pathSize;
    int    pathCap;
    bool   pathFound;
    int    pathLength;
} AStar;

/* flat grid access */
#define astar_get(a, i, j) (&(a)->grid[(i) * (a)->gridHeight + (j)])

static void list_append(Node ***arr, int *size, int *cap, Node *n) {
    if (*size == *cap) {
        *cap = (*cap == 0) ? 8 : (*cap) * 2;
        *arr = (Node **)realloc(*arr, sizeof(Node *) * (*cap));
    }
    (*arr)[(*size)++] = n;
}

static bool list_contains(Node **arr, int size, Node *n) {
    for (int i = 0; i < size; i++)
        if (arr[i] == n) return true;
    return false;
}

static void list_remove(Node **arr, int *size, Node *n) {
    for (int i = 0; i < *size; i++) {
        if (arr[i] == n) {
            arr[i] = arr[--(*size)];
            return;
        }
    }
}

static AStar astar_create(Node *grid, Node *start, Node *end, int gridWidth, int gridHeight) {
    AStar a;
    memset(&a, 0, sizeof(AStar));
    a.grid       = grid;
    a.gridWidth  = gridWidth;
    a.gridHeight = gridHeight;
    a.start      = start;
    a.end        = end;
    a.pathFound  = false;
    a.pathLength = 0;
    return a;
}

static Node *astar_getLowestF(AStar *a) {
    Node *lowest = a->openList[0];
    for (int i = 1; i < a->openSize; i++)
        if (a->openList[i]->fCost < lowest->fCost)
            lowest = a->openList[i];
    return lowest;
}

static void astar_getNeighbors(AStar *a, Node *node, Node **neighbors, int *count) {
    *count = 0;
    int x = node->x, y = node->y;
    if (x > 0)                neighbors[(*count)++] = astar_get(a, x-1, y);
    if (x < a->gridWidth - 1) neighbors[(*count)++] = astar_get(a, x+1, y);
    if (y > 0)                neighbors[(*count)++] = astar_get(a, x, y-1);
    if (y < a->gridHeight- 1) neighbors[(*count)++] = astar_get(a, x, y+1);
}

static void astar_findPath(AStar *a) {
    list_append(&a->openList, &a->openSize, &a->openCap, a->start);

    Node *neighbors[4];
    int   neighborCount;

    while (a->openSize > 0) {
        Node *current = astar_getLowestF(a);

        if (current == a->end) {
            a->pathFound = true;
            break;
        }

        list_remove(a->openList, &a->openSize, current);
        list_append(&a->closedList, &a->closedSize, &a->closedCap, current);

        astar_getNeighbors(a, current, neighbors, &neighborCount);
        for (int i = 0; i < neighborCount; i++) {
            Node *nb = neighbors[i];
            if (nb->state == STATE_WALL ||
                list_contains(a->closedList, a->closedSize, nb))
                continue;

            nb->gCost = current->gCost + 1;
            nb->hCost = (int)pow(nb->x - a->end->x, 2)
                      + (int)pow(nb->y - a->end->y, 2);
            nb->fCost = nb->gCost + nb->hCost;

            if (list_contains(a->openList, a->openSize, nb)
                && current->gCost < nb->gCost)
                continue;

            list_append(&a->openList, &a->openSize, &a->openCap, nb);
            nb->parent = current;
        }
    }

    if (a->pathFound) {
        Node *current = a->end->parent;
        while (current != a->start) {
            list_append(&a->path, &a->pathSize, &a->pathCap, current);
            current = current->parent;
        }
        /* reverse */
        for (int i = 0, j = a->pathSize - 1; i < j; i++, j--) {
            Node *tmp = a->path[i]; a->path[i] = a->path[j]; a->path[j] = tmp;
        }
        a->pathLength = a->pathSize;
    }
}

static void astar_free(AStar *a) {
    free(a->openList);
    free(a->closedList);
    free(a->path);
}

#endif /* ASTAR_H */