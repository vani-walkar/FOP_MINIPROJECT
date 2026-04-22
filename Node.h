#ifndef NODE_H
#define NODE_H

#include <stdbool.h>

/* ─── State enum (mirrors Java State enum) ────────────────────────────────── */
typedef enum {
    STATE_EMPTY,
    STATE_WALL,
    STATE_START,
    STATE_END,
    STATE_OPEN,
    STATE_CLOSED,
    STATE_PATH
} State;

/* ─── Node struct (mirrors Java Node class) ───────────────────────────────── */
typedef struct Node {
    int         x;
    int         y;
    int         nodeWidth;
    int         nodeHeight;
    int         gCost;
    int         hCost;
    int         fCost;
    struct Node *parent;
    State       state;
    bool        visited;    /* used by maze generator */
} Node;

/* ─── Constructor (mirrors Java Node constructor) ─────────────────────────── */
static inline Node node_create(int x, int y, int nodeWidth, int nodeHeight, State state) {
    Node n;
    n.x          = x / nodeWidth;
    n.y          = y / nodeHeight;
    n.nodeWidth  = nodeWidth;
    n.nodeHeight = nodeHeight;
    n.state      = state;
    n.gCost      = 0;
    n.hCost      = 0;
    n.fCost      = 0;
    n.parent     = NULL;
    n.visited    = false;
    return n;
}

/* ─── Color helper for rendering (mirrors paintComponent switch) ───────────── */
typedef struct { unsigned char r, g, b; } NodeColor;

static inline NodeColor node_get_color(State state) {
    switch (state) {
        case STATE_EMPTY:   return (NodeColor){242, 242, 242};
        case STATE_CLOSED:  return (NodeColor){220, 220, 220};
        case STATE_START:   return (NodeColor){ 99, 249,   0};
        case STATE_END:     return (NodeColor){254,  41,  27};
        case STATE_PATH:    return (NodeColor){255, 199,   0};
        case STATE_WALL:    return (NodeColor){ 30,  33,  36};
        default:            return (NodeColor){200, 200, 200};
        case STATE_OPEN:    return (NodeColor){147, 197, 253};
    }
}

/* ─── contains (mirrors Java contains method) ─────────────────────────────── */
static inline bool node_contains(Node *n, int px, int py) {
    int rx = n->x * n->nodeWidth;
    int ry = n->y * n->nodeHeight;
    return px >= rx && px < rx + n->nodeWidth &&
           py >= ry && py < ry + n->nodeHeight;
}

#endif /* NODE_H */