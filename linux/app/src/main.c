// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>

#include "common/shutdown.h"
#include "common/timing.h"
#include "common/periodTimer.h"
#include "hal/neopixelR5.h"

int main()
{
    printf("Find dot!\n");

    // Start modules
    Neopixel_init();

    // Main app modules
    Period_init();
    Shutdown_init();

    // DEBUG: set leds
    for (int i = 0; i < NEO_NUM_LEDS; i++) {
        Neopixel_setLED(i, LED_BLUE_BRIGHT);
    }

    Shutdown_wait();

    // End main body
    printf("Shutting down...\n");

    //JoystickBtnShutdown_cleanup();

    // End main app modules
    Shutdown_cleanup();
    Period_cleanup();

    // End hal modules
    Neopixel_cleanup();

    printf("!!! DONE !!!\n"); 
}
