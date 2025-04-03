// Module managing rotary encoder turns.

#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_

#include <stdbool.h>

// init/end
void RotaryEncoder_init(void);
void RotaryEncoder_cleanup(void);

// get the current rotary encoder value
int RotaryEncoder_getValue(void);

// set the rotary encoder default/min/max values (auto clamps)
void RotaryEncoder_setDefaultValue(int);
void RotaryEncoder_setMaxValue(int);
void RotaryEncoder_setMinValue(int);
void RotaryEncoder_setDecrementValue(int);
void RotaryEncoder_setIncrementValue(int);

#endif
