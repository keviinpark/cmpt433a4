// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "common/shutdown.h"
#include "common/timing.h"
#include "common/periodTimer.h"
#include "hal/neopixelR5.h"

static bool inRange(int index) {
    if (index >= 0 && index < 8) {
        return true;
    }
    return false;
}

int main()
{
    printf("Find dot!\n");

    // Start modules
    Neopixel_init();

    // Main app modules
    Period_init();
    Shutdown_init();

    // DEBUG: testing animation
    bool reverse = false;
    int iter = -1;
    while (true) {
        printf("current iteration: %d\n", iter);

        if (inRange(iter)) {
            Neopixel_setLED(iter, LED_GREEN_BRIGHT);
        }

        if (inRange(iter-1) && (iter-1) >= 0) {
            Neopixel_setLED(iter-1, LED_GREEN);
        }

        if (inRange(iter+1) && (iter+1) <= 7) {
            Neopixel_setLED(iter+1, LED_GREEN);
        }

        if (iter < 0) {
            reverse = false;
        } else if (iter >= NEO_NUM_LEDS) {
            reverse = true;
        }

        if (!reverse) {
            iter += 1;
        } else {
            iter -= 1;
        }
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
