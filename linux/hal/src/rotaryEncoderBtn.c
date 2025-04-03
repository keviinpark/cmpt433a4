// Abstraction over the button of the rotary encoder.

#include "hal/rotaryEncoderBtn.h"
#include "hal/gpio.h"

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

// Pin config info: GPIO 24 (Rotary Encoder PUSH)
//   $ gpiofind GPIO24
//   >> gpiochip0 10
#define GPIO_CHIP          GPIO_CHIP_0
#define GPIO_LINE_NUMBER   10
#define DEFAULT_START 0
#define DEFAULT_END 2
#define DEFAULT_VAL 0
#define DEBOUNCE_NS 100000000L

static bool isInitialized = false;
static struct GpioLine* s_lineBtn = NULL;
static atomic_int counter = DEFAULT_VAL;
static pthread_t mainThreadID;
static bool isRunning = false;
static int startValue = DEFAULT_START;
static int endValue = DEFAULT_END;

/*
    Define the Statemachine Data Structures
*/
struct stateEvent {
    struct state* pNextState;
    void (*action)();
};
struct state {
    struct stateEvent rising;
    struct stateEvent falling;
};

/*
    START STATEMACHINE
*/
static void onRelease(void)
{
    counter++;

    if (counter > endValue) {
        counter = startValue;
    }
}

static struct state states[] = {
    { // Not pressed
        .rising = {&states[0], NULL},
        .falling = {&states[1], NULL},
    },

    { // Pressed
        .rising = {&states[0], onRelease},
        .falling = {&states[1], NULL},
    },
};
/*
    END STATEMACHINE
*/

static struct state* pCurrentState = &states[0];

static void doState(void)
{
    assert(isInitialized);

    struct gpiod_line_bulk bulkEvents;
    int numEvents = Gpio_waitForLineChange(s_lineBtn, NULL, &bulkEvents);

    // Iterate over the event
    for (int i = 0; i < numEvents; i++)
    {
        // Get the line handle for this event
        struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

        // Get the number of this line
        unsigned int this_line_number = gpiod_line_offset(line_handle);

        // Get the line event
        struct gpiod_line_event event;
        if (gpiod_line_event_read(line_handle,&event) == -1) {
            perror("Line Event");
            exit(EXIT_FAILURE);
        }

        // Run the state machine
        bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

        // Can check with line it is, if you have more than one...
        bool isBtn = this_line_number == GPIO_LINE_NUMBER;
        assert (isBtn);

        struct stateEvent* pStateEvent = NULL;
        if (isRising) {
            pStateEvent = &pCurrentState->rising;
        } else {
            pStateEvent = &pCurrentState->falling;
        } 

        // Do the action
        if (pStateEvent->action != NULL) {
            pStateEvent->action();
        }
        pCurrentState = pStateEvent->pNextState;

        struct timespec req = {0, DEBOUNCE_NS};
        nanosleep(&req, NULL);
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

// init/end
void RotaryEncoderBtn_init(void)
{
    assert(!isInitialized);
    isInitialized = true;
    s_lineBtn = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER);

    isRunning = true;
    int err = pthread_create(&mainThreadID, NULL, &rotaryEncoderUpdateThread, NULL);
    if (err) {
        printf("Rotary Encoder Btn: failed to create main thread.\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
}

void RotaryEncoderBtn_cleanup(void)
{
    assert(isInitialized);

    isRunning = false;
    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("Rotary Encoder Btn: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    int err = pthread_join(mainThreadID, NULL);
    if (err) {
        perror("Rotary Encoder Btn: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    Gpio_close(s_lineBtn);
    isInitialized = false;
}

// get the current rotary encoder button value
int RotaryEncoderBtn_getValue(void)
{
    assert(isInitialized);

    return counter;
}

// set start value of counter
void RotaryEncoderBtn_setStartValue(int newVal)
{
    assert(isInitialized);
    assert(newVal < endValue);

    startValue = newVal;
}

// set last value of counter
void RotaryEncoderBtn_setEndValue(int val)
{
    assert(isInitialized);
    assert(val > startValue);

    endValue = val;
}

// set default value of counter
void RotaryEncoderBtn_setDefaultValue(int val)
{
    assert(isInitialized);
    assert(val >= startValue && val <= endValue);

    counter = val;
}
