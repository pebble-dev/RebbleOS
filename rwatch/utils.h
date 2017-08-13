#pragma once

#include "pebble_defines.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(v, min, max) MIN(max, MAX(min, v))

#define POINT_EQ(p1, p2) ((p1).x == (p2).x && (p1).y == (p2).y)
#define SIZE_EQ(s1, s2) ((s1).w == (s2).w && (s1).h == (s2).h)
#define RECT_EQ(r1, r2) (POINT_EQ((r1).origin, (r2).origin) && SIZE_EQ((r1).size, (r2).size))
