// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>

#include "common/shutdown.h"
#include "common/periodTimer.h"

int main()
{
    printf("Find dot!\n");

    // Start modules
    Period_init();
    Shutdown_init();

    // Main app modules
    Shutdown_wait();
    printf("Shutting down...\n");

    Shutdown_cleanup();
    Period_cleanup();

    printf("!!! DONE !!!\n"); 
}
