#include "game.h"

#include "hal/neopixelR5.h"
#include "hal/accelerometer.h"
#include "hal/rotaryEncoderBtn.h"
#include "common/timing.h"
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static bool isInitialized = false;
static bool isRunning = false;
static pthread_t mainThreadID;
static int hits = 0;
static int misses = 0;
static long long elapsedTimeMS = 0;
bool onTarget = false;
int prevRotaryCounter = 0;
int currentRotaryCounter;

// main thread
static void* gameThread(void* _args)
{
    (void)_args; // disable unused error

    // TODO support multiple different animations.
    // This is currently just testing out LEDs
    // count to 8 (last led index + 1) and then loop back around to 1, then
    // repeat this process (i.e. -1, 0, 1, 2, ..., 7, 8, 7, 6, ..., 0, -1, 0, 1, 2 ...)
    bool reverse = false;
    int curr = -1;

    // Set random point as target
    srand(time(NULL));

    double x = ((double)rand() / RAND_MAX) - 0.5;
    double y = ((double)rand() / RAND_MAX) - 0.5;

    printf("Random point: %f, %f\n", x, y);

    while (isRunning) {
        assert(curr >= -1);
        assert(curr <= NEO_NUM_LEDS);

        coordinates CurrentCoords = Accel_getCurrentCoords();
        printf("Current x, y coordinates: %f, %f\n", CurrentCoords.x, CurrentCoords.y);

        // Directly pointing at target (IMPLEMENT BLUE WITH ALL LED ON)
        if (CurrentCoords.x - x <= 0.1 && CurrentCoords.y - y <= 0.1) {
            printf("Shoot!\n");
            onTarget = true;
        } 
        
        else {
            onTarget = false;        
        }
        
        currentRotaryCounter = RotaryEncoderBtn_getValue();

        // On target and fired (IMPLEMENT LED)
        if (onTarget && currentRotaryCounter != prevRotaryCounter) {
            printf("Hit!\n");
        }

        // Off target and fired (IMPLEMENT LED)
        else if (!onTarget && currentRotaryCounter != prevRotaryCounter) {
            printf("Miss!\n");
        }

        prevRotaryCounter = currentRotaryCounter;

        printf("Elapsed time ms: %lld\n", elapsedTimeMS);

        long long startTimeMS = Timing_getTimeMS();

        Neopixel_resetLEDs();

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

        Timing_sleepForMS(100);

        long long currentTimeMS = Timing_getTimeMS() + 100; // + account for sleep
        elapsedTimeMS += currentTimeMS - startTimeMS;
    }

    return NULL;
}

// init
void Game_init(void)
{
    assert(!isInitialized);

    // start thread
    isRunning = true;
    if (pthread_create(&mainThreadID, NULL, &gameThread, NULL)) {
        perror("Game: failed to create main thread:");
        exit(EXIT_FAILURE);
    }

    isInitialized = true;
}

// cleanup
void Game_cleanup(void)
{
    assert(isInitialized);

    // end thread
    isRunning = false;
    if (pthread_join(mainThreadID, NULL)) {
        perror("Game: failed to cancel join thread:");
        exit(EXIT_FAILURE);
    }

    isInitialized = false;
}

// get # hits
int Game_getHits(void)
{
    assert(isInitialized);

    return hits;
}

// get # misses
int Game_getMisses(void)
{
    assert(isInitialized);

    return misses;
}

// get elapsed time in milliseconds
long long Game_getElapsedTimeMS(void)
{
    assert(isInitialized);

    return elapsedTimeMS;
}
