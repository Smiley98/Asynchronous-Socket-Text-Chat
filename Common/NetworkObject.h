#pragma once
struct Point {
    unsigned short x;
    unsigned short y;
};

struct Puck {
    Point position;
    Point velocity;
};