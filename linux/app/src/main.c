// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "game.h"
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
    Game_init();

    Shutdown_wait();
    // End main body
    printf("Shutting down...\n");

    // End main app modules
    Game_cleanup();
    Shutdown_cleanup();
    Period_cleanup();

    // End hal modules
    Neopixel_cleanup();

    printf("!!! DONE !!!\n"); 
}
