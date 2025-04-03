// Abstraction over the button of the rotary encoder.

#ifndef _ROTARY_ENCODER_BTN_H_
#define _ROTARY_ENCODER_BTN_H_

// init/end
void RotaryEncoderBtn_init(void);
void RotaryEncoderBtn_cleanup(void);

// get the current rotary encoder btn value
int RotaryEncoderBtn_getValue(void);

// set start/end/default values of encoder btn value
void RotaryEncoderBtn_setStartValue(int);
void RotaryEncoderBtn_setEndValue(int);
void RotaryEncoderBtn_setDefaultValue(int);

#endif
