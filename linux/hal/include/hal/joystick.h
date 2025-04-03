// Joystick module 
// Part of the Hardware Abstraction Layer (HAL) 

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include <stdbool.h>

enum Direction {
    joystick_up,
    joystick_down,
    joystick_left,
    joystick_right,
    joystick_idle,
    joystick_pushed,
};

typedef enum Direction Direction;

void Joystick_init(void);
void Joystick_cleanup(void);

// get direction
Direction Joystick_getState(void);

#endif
