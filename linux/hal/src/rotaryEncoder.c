// Module managing rotary encoder turns.

#include "hal/rotaryEncoder.h"
#include "hal/gpio.h"

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

#define GPIO_CHIP            GPIO_CHIP_2
#define GPIO_LINE_NUMBER_A   7
#define GPIO_LINE_NUMBER_B   8

#define STATE_REST 0
#define STATE_CCW 1
#define STATE_UNDETERMINED 2
#define STATE_CW 3
#define DEFAULT_COUNTER_VALUE 0
#define DEFAULT_MIN_COUNT 0
#define DEFAULT_MAX_COUNT 500
#define DEFAULT_INCREMENT 1
#define DEFAULT_DECREMENT 1

static bool isInitialized = false;

static struct GpioLine* lineA = NULL;
static struct GpioLine* lineB = NULL;
static atomic_int counter = DEFAULT_COUNTER_VALUE;
static atomic_bool isCCW = false;
static atomic_bool isCW = false;
static pthread_t mainThreadID;
static bool isRunning = false;
static int minValue = DEFAULT_MIN_COUNT;
static int maxValue = DEFAULT_MAX_COUNT;
static int incrementValue = DEFAULT_INCREMENT;
static int decrementValue = DEFAULT_INCREMENT;

/*
    Define the Statemachine Data Structures
*/
struct stateEvent {
    struct state* pNextState;
    void (*action)();
};

struct state {
    struct stateEvent risingA;
    struct stateEvent fallingA;
    struct stateEvent risingB;
    struct stateEvent fallingB;
};

/*
    START STATEMACHINE
*/
static void clearFlags(void)
{
    isCW = false;
    isCCW = false;
}

static void setFlagsCW(void)
{
    isCW = true;
    isCCW = false;
}

static void setFlagsCCW(void)
{
    isCW = false;
    isCCW = true;
}

static void increment(void)
{
    if (isCW && (counter < maxValue)) {
        counter += incrementValue;

        if (counter > maxValue) {
            counter = maxValue;
        }
    }

    clearFlags();
}

static void decrement(void)
{
    if (isCCW && (counter > minValue)) {
        counter -= decrementValue;

        if (counter < minValue) {
            counter = minValue;
        }
    }

    clearFlags();
}

static struct state states[] = {
    { // Rest
        .risingA = {&states[STATE_REST], clearFlags},
        .fallingA = {&states[STATE_CCW], setFlagsCW},
        .risingB = {&states[STATE_REST], clearFlags},
        .fallingB = {&states[STATE_CW], setFlagsCCW},
    },

    { // CCW (1)
        .risingA = {&states[STATE_REST], decrement},
        .fallingA = {&states[STATE_CCW], NULL},
        .risingB = {&states[STATE_CCW], NULL},
        .fallingB = {&states[STATE_UNDETERMINED], NULL},
    },

    { // (2)
        .risingA = {&states[STATE_CW], NULL},
        .fallingA = {&states[STATE_UNDETERMINED], NULL},
        .risingB = {&states[STATE_CCW], NULL},
        .fallingB = {&states[STATE_UNDETERMINED], NULL},
    },

    { // CW (3)
        .risingA = {&states[STATE_CW], NULL},
        .fallingA = {&states[STATE_UNDETERMINED], NULL},
        .risingB = {&states[STATE_REST], increment},
        .fallingB = {&states[STATE_CW], NULL},
    },
};
/*
    END STATEMACHINE
*/

static struct state* pCurrentState = &states[STATE_REST];

static void doState(void)
{
    assert(isInitialized);

    struct gpiod_line_bulk bulkEvents;
    int numEvents = Gpio_waitForLineChange(lineA, lineB, &bulkEvents);

    // Iterate over the event
    for (int i = 0; i < numEvents; i++)
    {
        // Get the line handle for this event
        struct gpiod_line *lineHandle = gpiod_line_bulk_get_line(&bulkEvents, i);
        // Get the number of this line
        unsigned int lineNumber = gpiod_line_offset(lineHandle);

        // Get the line event
        struct gpiod_line_event event;
        if (gpiod_line_event_read(lineHandle, &event) == -1) {
            perror("Line Event");
            exit(EXIT_FAILURE);
        }

        // Run the state machine
        bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

        struct stateEvent* pStateEvent = NULL;
        assert(lineNumber == GPIO_LINE_NUMBER_A || lineNumber == GPIO_LINE_NUMBER_B);
        if (lineNumber == GPIO_LINE_NUMBER_A) {
            if (isRising) {
                pStateEvent = &pCurrentState->risingA;
            } else {
                pStateEvent = &pCurrentState->fallingA;
            }
        } else {
            if (isRising) {
                pStateEvent = &pCurrentState->risingB;
            } else {
                pStateEvent = &pCurrentState->fallingB;
            }
        }

        // Do the action
        if (pStateEvent->action != NULL) {
            pStateEvent->action();
        }
        pCurrentState = pStateEvent->pNextState;
    }
}

static void *rotaryEncoderUpdateThread(void *args)
{
    (void)args;

    while (isRunning) {
        doState();
    }

    return NULL;
}

// init
void RotaryEncoder_init(void)
{
    assert(!isInitialized);
    isInitialized = true;

    lineA = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER_A);
    lineB = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER_B);

    isRunning = true;

    int err = pthread_create(&mainThreadID, NULL, &rotaryEncoderUpdateThread, NULL);
    if (err) {
        printf("Rotary Encoder: failed to create main thread.\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
}

void RotaryEncoder_cleanup(void)
{
    assert(isInitialized);
    isRunning = false;
    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("Rotary Encoder: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    int err = pthread_join(mainThreadID, NULL);
    if (err) {
        perror("Rotary Encoder: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    Gpio_close(lineA);
    Gpio_close(lineB);
    isInitialized = false;
}

int RotaryEncoder_getValue(void)
{
    assert(isInitialized);

    return counter;
}

// set default value
void RotaryEncoder_setDefaultValue(int value)
{
    assert(isInitialized);
    assert(value >= 0);
    assert(value <= maxValue && value >= minValue);

    counter = value;
}

// set max value
void RotaryEncoder_setMaxValue(int value)
{
    assert(isInitialized);
    assert(value >= 0);

    maxValue = value;

    if (counter > maxValue) {
        counter = maxValue;
    }
}

// set min value
void RotaryEncoder_setMinValue(int value)
{
    assert(isInitialized);
    assert(value >= 0);

    minValue = value;

    if (counter < minValue) {
        counter = minValue;
    }
}

// set decrement amount
void RotaryEncoder_setDecrementValue(int value)
{
    assert(isInitialized);
    assert(value >= 0);

    decrementValue = value;
}

// set increment amount
void RotaryEncoder_setIncrementValue(int value)
{
    assert(isInitialized);
    assert(value >= 0);

    incrementValue = value;
}
