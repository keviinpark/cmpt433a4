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
#include <math.h>

// abs. Breakpoints from 5 (furthest away) to 1 (closest/accurate)
#define BREAKPOINT_5 0.5
#define BREAKPOINT_4 0.4
#define BREAKPOINT_3 0.3
#define BREAKPOINT_2 0.2
#define BREAKPOINT_1 0.1

static bool isInitialized = false;
static bool isRunning = false;
static pthread_t mainThreadID;
static int hits = 0;
static int misses = 0;
static long long elapsedTimeMS = 0;
bool onTarget = false;
int prevRotaryCounter = 0;
int currentRotaryCounter;
coordinates Target;

void newTarget() {
    srand(time(NULL));

    Target.x = ((double)rand() / RAND_MAX) - 0.5;
    Target.y = ((double)rand() / RAND_MAX) - 0.5;
}

// COLOR: color to use, set to bright color to ignore param BRIGHTCOLOR
// Y: index of the middle bright led [-1, 8] (just 1 difference from led indexes [0, 7])
// onY: if the target is already on Y or not, causes Y to be ignored
// BRIGHTCOLOR: the bright version of COLOR, ignore if param COLOR is already bright
static void setLEDsFromTarget(uint32_t color, int y, bool onY, uint32_t brightColor)
{
    Neopixel_resetLEDs();

    // turn on all LEDs
    if (onY) {
        for (int i = 0; i < NEO_NUM_LEDS; i++) {
            Neopixel_setLED(i, color);
        }

        return;
    }

    int nextY = y + 1;
    int prevY = y - 1;

    if (y >= 0 && y < NEO_NUM_LEDS) {
        Neopixel_setLED(y, brightColor);
    }

    if (prevY >= 0 && prevY < NEO_NUM_LEDS) {
        Neopixel_setLED(prevY, color);
    }

    if (nextY >= 0 && nextY < NEO_NUM_LEDS) {
        Neopixel_setLED(nextY, color);
    }
}

// main thread
static void* gameThread(void* _args)
{
    (void)_args; // disable unused error

    // TODO support multiple different animations.
    // This is currently just testing out LEDs
    // count to 8 (last led index + 1) and then loop back around to 1, then
    // repeat this process (i.e. -1, 0, 1, 2, ..., 7, 8, 7, 6, ..., 0, -1, 0, 1, 2 ...)

    Neopixel_resetLEDs();

    // bool reverse = false;
    int curr = -1;

    // Set random point as target
    newTarget();
    
    printf("Random point: %f, %f\n", Target.x, Target.y);

    while (isRunning) {
        assert(curr >= -1);
        assert(curr <= NEO_NUM_LEDS);

        coordinates CurrentCoords = Accel_getCurrentCoords();
        printf("Current: %f, %f   |   Target: %f, %f\n", CurrentCoords.x, CurrentCoords.y, Target.x, Target.y);

        // check y axis
        double diff = fabs(CurrentCoords.y - Target.y);
        bool onTargetY = diff <= BREAKPOINT_1;
        bool isHigh = CurrentCoords.y >= Target.y;

        if (onTargetY) {
            // skip condition checking
        } else if (isHigh) {
            if (diff > BREAKPOINT_5) {
                curr = -1;
            }
            else if (diff > BREAKPOINT_4) {
                curr = 0;
            }
            else if (diff > BREAKPOINT_3) {
                curr = 1;
            }
            else if (diff > BREAKPOINT_2) {
                curr = 2;
            }
            else if (diff > BREAKPOINT_1) {
                curr = 3;
            }
        } else {
            if (diff > BREAKPOINT_5) {
                curr = 8;
            } else if (diff > BREAKPOINT_4) {
                curr = 7;
            } else if (diff > BREAKPOINT_3) {
                curr = 6;
            } else if (diff > BREAKPOINT_2) {
                curr = 5;
            } else if (diff > BREAKPOINT_1) {
                curr = 4;
            }
        }

        if (fabs(CurrentCoords.y - Target.y) <= BREAKPOINT_1) {
            printf("Stay (y)!\n");
        } else if (CurrentCoords.y < Target.y) {
            printf("Move up!\n");
        } else if (CurrentCoords.y > Target.y) {
            printf("Move down!\n");
        }

        // Directly pointing at target (IMPLEMENT BLUE WITH ALL LED ON)
        if ((fabs(CurrentCoords.x - Target.x) <= BREAKPOINT_1) && (fabs(CurrentCoords.y - Target.y) <= BREAKPOINT_1)) {
            printf("Shoot!\n");

            setLEDsFromTarget(LED_BLUE_BRIGHT, curr, onTargetY, LED_BLUE_BRIGHT);

            onTarget = true;
        } 
        
        // IMPLEMENT LED TO SHOW CLOSENESS TO TARGET
        // RED AND GREEN FOR LEFT AND RIGHT, BLUE FOR ON TARGET X

        else if (fabs(CurrentCoords.x - Target.x) <= BREAKPOINT_1) {
            printf("Stay (x)!\n");

            setLEDsFromTarget(LED_BLUE, curr, onTargetY, LED_BLUE_BRIGHT);

            onTarget = false;   
        }
        else if (Target.x < CurrentCoords.x) {
            printf("Move left!\n");

            setLEDsFromTarget(LED_RED, curr, onTargetY, LED_RED_BRIGHT);
            onTarget = false;        
        }

        else if (Target.x > CurrentCoords.x) {
            printf("Move right!\n");
            setLEDsFromTarget(LED_GREEN, curr, onTargetY, LED_GREEN_BRIGHT);
            onTarget = false;        
        }
        
        currentRotaryCounter = RotaryEncoderBtn_getValue();

        // On target and fired (IMPLEMENT LED HIT EFFECT)
        if (onTarget && currentRotaryCounter != prevRotaryCounter) {
            printf("Hit!\n");
            hits += 1;
            newTarget();
            printf("Random point: %f, %f\n", Target.x, Target.y);
            Timing_sleepForMS(1000);
        }

        // Off target and fired (IMPLEMENT LED MISS EFFECT)
        else if (!onTarget && currentRotaryCounter != prevRotaryCounter) {
            printf("Miss!\n");
            misses += 1;
            Timing_sleepForMS(1000);
        }

        prevRotaryCounter = currentRotaryCounter;

        Timing_sleepForMS(1000);

        /* printf("Elapsed time ms: %lld\n", elapsedTimeMS);

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

        

        long long currentTimeMS = Timing_getTimeMS() + 100; // + account for sleep
        elapsedTimeMS += currentTimeMS - startTimeMS; */
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
