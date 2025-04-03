// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

#include "hal/gpio.h"
#include "hal/joystickBtn.h"
#include "hal/audioMixer.h"
#include "hal/rotaryEncoder.h"
#include "hal/rotaryEncoderBtn.h"
#include "hal/joystick.h"
#include "hal/accelerometer.h"
#include "common/shutdown.h"
#include "common/timing.h"
#include "common/periodTimer.h"
#include "lcd.h"
#include "server.h"
#include "drumBeats.h"
#include "terminal.h"

int main()
{
    printf("Beat box!\n");

    // Start modules

    Accel_init();
    Gpio_initialize();
    Period_init();
    Shutdown_init();
    AudioMixer_init();
    RotaryEncoder_init();
    RotaryEncoderBtn_init();
    JoystickBtn_init();
    Joystick_init();

    // Main app modules
    Server_init();
    DrumBeats_init();
    Terminal_init();
    DrawStuff_init();

    Shutdown_wait();
    printf("Shutting down...\n");

    // End app modules
    DrawStuff_cleanup();
    Terminal_cleanup();
    DrumBeats_cleanup();
    Server_cleanup();

    // End modules

    Joystick_cleanup();
    JoystickBtn_cleanup();
    RotaryEncoderBtn_cleanup();
    RotaryEncoder_cleanup();
    AudioMixer_cleanup();
    Shutdown_cleanup();
    Period_cleanup();
    Gpio_cleanup();
    Accel_cleanup();

    printf("!!! DONE !!!\n"); 
}
