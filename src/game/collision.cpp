#include "collision.h"

// Detects collision between two rectangles
bool coll::rect_rect(int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1) {
    return
        x0 < x1 + w1 &&
        x0 + w0 > x1 &&
        y0 < y1 + h1 &&
        y0 + h0 > y1;
}
