// Manage Joystick press-in button
#ifndef _JOYSTICK_BTN_H_
#define _JOYSTICK_BTN_H_

#include <stdbool.h>

void JoystickBtn_init(void);
void JoystickBtn_cleanup(void);

// get the current value of joystick btn
int JoystickBtn_getValue(void);

#endif
