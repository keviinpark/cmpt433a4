// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "common/shutdown.h"
#include "common/timing.h"
#include "common/periodTimer.h"
#include "hal/neopixelR5.h"

//static bool inRange(int index) {
//    if (index >= 0 && index < 8) {
//        return true;
//    }
//    return false;
//}

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
    int curr = -1;
    while (true) {
        // curr should be [-1 (first led index - 1), 8 (last led index + 1)]
        assert(curr >= -1);
        assert(curr <= NEO_NUM_LEDS);

        for (int j = 0; j < NEO_NUM_LEDS; j++) {
            Neopixel_setLED(j, LED_OFF);
        }

        // animation
        int prev = curr - 1;
        if (prev >= 0 && prev < NEO_NUM_LEDS) {
            Neopixel_setLED(prev, LED_GREEN);
        }

        if (curr >= 0 && curr < NEO_NUM_LEDS) {
            Neopixel_setLED(curr, LED_GREEN_BRIGHT);
        }

        int next = curr + 1;
        if (next >= 0 && next < NEO_NUM_LEDS) {
            Neopixel_setLED(next, LED_GREEN);
        }

        if (curr < 0) {
            reverse = false;
        } else if (curr >= NEO_NUM_LEDS) {
            reverse = true;
        }

        if (reverse) {
            curr--;
        } else {
            curr++;
        }

        // Timing
        Timing_sleepForMS(100);
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
