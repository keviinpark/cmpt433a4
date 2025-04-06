// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "game.h"
#include "common/shutdown.h"
#include "common/timing.h"
#include "hal/neopixelR5.h"
#include "hal/accelerometer.h"
#include "hal/gpio.h"
#include "hal/rotaryEncoderBtn.h"
#include "hal/joystickBtn.h"
#include "lcd.h"

int main()
{
    printf("Find dot!\n");

    // Start modules
    Neopixel_init();
    Accel_init();
    Gpio_initialize();
    RotaryEncoderBtn_init();
    JoystickBtn_init();

    // Main app modules
    Shutdown_init();
    Game_init();
    DrawStuff_init();

    Shutdown_wait();
    // End main body
    printf("Shutting down...\n");

    // End main app modules
    DrawStuff_cleanup();
    // printf("LCD off!\n");
    Game_cleanup();
    // printf("Game off!\n");
    Shutdown_cleanup();
    // printf("Shutdown off!\n");

    // End hal modules
    JoystickBtn_cleanup();
    // printf("Joystick off!\n");
    RotaryEncoderBtn_cleanup();
    // printf("RotaryEncoder off!\n");
    Gpio_cleanup();
    // printf("GPIO off!\n");
    Accel_cleanup();
    // printf("Accel off!\n");
    Neopixel_cleanup();
    // printf("Neopixel off!\n");

    printf("!!! DONE !!!\n"); 
}
