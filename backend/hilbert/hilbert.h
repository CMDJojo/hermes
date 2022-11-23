#pragma once
#include <cstdint>

//convert (x,y) to d
int64_t xy2d (int64_t n, int x, int y);

//convert d to (x,y)
void d2xy(int64_t n, int64_t d, int *x, int *y);

//rotate/flip a quadrant appropriately
void rot(int64_t n, int *x, int *y, int64_t rx, int64_t ry);