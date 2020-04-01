#pragma once
#include "../Common/spritelib/vector3.h"

struct Point {
    short x;
    short y;
};

struct Puck {
    Point position;
    Point velocity;
};

struct Kinematic {
    spritelib::math::Vector3 position;
    spritelib::math::Vector3 velocity;
    spritelib::math::Vector3 acceleration;
};

struct Correction {
    spritelib::math::Vector3 position;
};