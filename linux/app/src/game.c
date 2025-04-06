#include "game.h"

#include "hal/neopixelR5.h"
#include "hal/accelerometer.h"
#include "hal/rotaryEncoderBtn.h"
#include <hal/joystickBtn.h>
#include "common/shutdown.h"
#include "common/timing.h"
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ABS_POINT_RANGE 0.5

// abs. Breakpoints from 5 (furthest away) to 1 (closest/accurate)
#define BREAKPOINT_5 0.5
#define BREAKPOINT_4 0.4
#define BREAKPOINT_3 0.3
#define BREAKPOINT_2 0.2
#define BREAKPOINT_1 0.1

// LED indexes (0 - 7 = moving up)
#define LED_0 0
#define LED_1 1
#define LED_2 2
#define LED_3 3
#define LED_4 4
#define LED_5 5
#define LED_6 6
#define LED_7 7

#define LOOP_DELAY_MS 100

#define ANIMATION_PLAY_TIME_MS 540
#define ANIMATION_FRAMES 6
static uint32_t hitAnimation[ANIMATION_FRAMES][NEO_NUM_LEDS] = {
    {
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT
    },
    {
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT
    },
    {
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_BLUE_BRIGHT,
        LED_BLUE_BRIGHT
    },
    {
        LED_BLUE_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_OFF,
        LED_OFF,
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_BLUE_BRIGHT
    },
    {
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_GREEN_BRIGHT,
        LED_GREEN_BRIGHT
    },
    {
        LED_GREEN_BRIGHT,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_GREEN_BRIGHT
    }
};

static uint32_t missAnimation[ANIMATION_FRAMES][NEO_NUM_LEDS] = {
    {
        LED_WHITE,
        LED_RED,
        LED_RED,
        LED_RED_BRIGHT,
        LED_RED_BRIGHT,
        LED_RED,
        LED_RED,
        LED_WHITE
    },
    {
        LED_WHITE,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_WHITE
    },
    {
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED
    },
    {
        LED_OFF,
        LED_OFF,
        LED_WHITE,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED,
        LED_RED
    },
    {
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_WHITE,
        LED_RED,
        LED_RED,
        LED_RED
    },
    {
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_OFF,
        LED_WHITE,
        LED_RED
    }
};

static bool isInitialized = false;
static bool isRunning = false;
static pthread_t mainThreadID;
static int hits = 0;
static int misses = 0;
static long long elapsedTimeMS = 0;
static bool onTarget = false;
static int prevRotaryCounter = 0;
static int currentRotaryCounter;
static coordinates Target;
static bool isPlayingAnimation = false;

static void newTarget() {
    srand(time(NULL));

    Target.x = ((double)rand() / RAND_MAX) - ABS_POINT_RANGE;
    Target.y = ((double)rand() / RAND_MAX) - ABS_POINT_RANGE;
}

static void playAnimation(uint32_t animation[][NEO_NUM_LEDS])
{
    isPlayingAnimation = true;
    for (int frame = 0; frame < ANIMATION_FRAMES; frame++) {
        for (int led = 0; led < NEO_NUM_LEDS; led++) {
            Neopixel_setLED(led, animation[frame][led]);
        }

        Timing_sleepForMS(ANIMATION_PLAY_TIME_MS / ANIMATION_FRAMES);
    }
    isPlayingAnimation = false;
}

// COLOR: color to use, set to bright color to ignore param BRIGHTCOLOR
// Y: index of the middle bright led [-1, 8] (just 1 difference from led indexes [0, 7])
// onY: if the target is already on Y or not, causes Y to be ignored
// BRIGHTCOLOR: the bright version of COLOR, ignore if param COLOR is already bright
static void setLEDsFromTarget(uint32_t color, int y, bool onY, uint32_t brightColor)
{
    if (isPlayingAnimation) {
        return;
    }

    Neopixel_resetLEDs();

    // turn on all LEDs
    if (onY) {
        for (int i = 0; i < NEO_NUM_LEDS; i++) {
            Neopixel_setLED(i, brightColor);
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

// Determine brightest LED index (does not check whether target y is in the correct range)
static int getBrightestLEDindex(coordinates currentCoords, coordinates target)
{
    double diff = fabs(currentCoords.y - target.y);
    bool isHigh = currentCoords.y >= target.y;

    if (isHigh) {
        if (diff > BREAKPOINT_5) {
            return LED_0 - 1;
        } else if (diff > BREAKPOINT_4) {
            return LED_0;
        } else if (diff > BREAKPOINT_3) {
            return LED_1;
        } else if (diff > BREAKPOINT_2) {
            return LED_2;
        } else if (diff > BREAKPOINT_1) {
            return LED_3;
        }
    } else {
        if (diff > BREAKPOINT_5) {
            return LED_7 + 1;
        } else if (diff > BREAKPOINT_4) {
            return LED_7;
        } else if (diff > BREAKPOINT_3) {
            return LED_6;
        } else if (diff > BREAKPOINT_2) {
            return LED_5;
        } else if (diff > BREAKPOINT_1) {
            return LED_4;
        }
    }

    return -1;
}

// main thread
static void* gameThread(void* _args)
{
    (void)_args; // disable unused error

    Neopixel_resetLEDs();

    // curr represents the brightest led index.
    // can be 1 off of 0 or 7 since there may not always be a brightest led value.
    int curr = LED_0 - 1;

    // Set random point as target
    newTarget();

    while (isRunning) {
        assert(curr >= (LED_0 - 1));
        assert(curr <= NEO_NUM_LEDS);

        long long startTimeMS = Timing_getTimeMS();

        bool checkShutdown = JoystickBtn_getValue() != 0;
        if (checkShutdown) {
            Shutdown_trigger();
            isRunning = false;
            break;
        }

        coordinates CurrentCoords = Accel_getCurrentCoords();

        // check y axis
        double diff = fabs(CurrentCoords.y - Target.y);
        bool onTargetY = diff <= BREAKPOINT_1;
        if (!onTargetY) { // every led should be the brightest anyway if onTargetY == true
            curr = getBrightestLEDindex(CurrentCoords, Target);
        }

        // Directly pointing at target (IMPLEMENT BLUE WITH ALL LED ON)
        if ((fabs(CurrentCoords.x - Target.x) <= BREAKPOINT_1) && (fabs(CurrentCoords.y - Target.y) <= BREAKPOINT_1)) {
            setLEDsFromTarget(LED_BLUE_BRIGHT, curr, onTargetY, LED_BLUE_BRIGHT);

            onTarget = true;
        } 
        
        // IMPLEMENT LED TO SHOW CLOSENESS TO TARGET
        // RED AND GREEN FOR LEFT AND RIGHT, BLUE FOR ON TARGET X

        else if (fabs(CurrentCoords.x - Target.x) <= BREAKPOINT_1) {
            setLEDsFromTarget(LED_BLUE, curr, onTargetY, LED_BLUE_BRIGHT);

            onTarget = false;   
        }
        else if (Target.x < CurrentCoords.x) {
            setLEDsFromTarget(LED_RED, curr, onTargetY, LED_RED_BRIGHT);
            onTarget = false;        
        }

        else if (Target.x > CurrentCoords.x) {
            setLEDsFromTarget(LED_GREEN, curr, onTargetY, LED_GREEN_BRIGHT);
            onTarget = false;        
        }
        currentRotaryCounter = RotaryEncoderBtn_getValue();

        // On target and fired (IMPLEMENT LED HIT EFFECT)
        if (onTarget && currentRotaryCounter != prevRotaryCounter) {
            printf("Hit!\n");
            hits += 1;

            playAnimation(hitAnimation);
            newTarget();
        }
        // Off target and fired (IMPLEMENT LED MISS EFFECT)
        else if (!onTarget && currentRotaryCounter != prevRotaryCounter) {
            printf("Miss!\n");
            misses += 1;

            playAnimation(missAnimation);
        }

        prevRotaryCounter = currentRotaryCounter;

        Timing_sleepForMS(LOOP_DELAY_MS);
        long long currentTimeMS = Timing_getTimeMS() + LOOP_DELAY_MS; // + account for sleep
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
