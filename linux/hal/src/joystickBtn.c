// Manage Joystick press-in button

#include "hal/joystickBtn.h"
#include "hal/gpio.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <time.h>
#include <pthread.h>

// Pin config info: GPIO 24 (Rotary Encoder PUSH)
//   $ gpiofind GPIO5
//   >> gpiochip2 15
#define GPIO_CHIP          GPIO_CHIP_2
#define GPIO_LINE_NUMBER   15
#define DEBOUNCE_NS 100000000L

static bool isInitialized = false;
static bool isRunning = false;

static struct GpioLine* s_lineBtn = NULL;
static atomic_int counter = 0;
static pthread_t mainThreadID;

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
static void on_release(void)
{
    counter++;
}

static struct state states[] = {
    { // Not pressed
        .rising = {&states[0], NULL},
        .falling = {&states[1], NULL},
    },

    { // Pressed
        .rising = {&states[0], on_release},
        .falling = {&states[1], NULL},
    },
};
/*
    END STATEMACHINE
*/

static struct state* pCurrentState = &states[0];

static void JoystickBtn_doState(void);

static void *joystickButtonUpdateThread(void *args)
{
    (void)args;

    while (isRunning) {
        JoystickBtn_doState();
    }

    return NULL;
}

void JoystickBtn_init()
{
    assert(!isInitialized);
    isInitialized = true;
    s_lineBtn = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER);
    isRunning = true;

    int err = pthread_create(&mainThreadID, NULL, &joystickButtonUpdateThread, NULL);
    if (err) {
        printf("Joystick Button: failed to create main thread.\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
}
void JoystickBtn_cleanup()
{
    assert(isInitialized);
    isRunning = false;
    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("Joystick button: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    int err = pthread_join(mainThreadID, NULL);
    if (err) {
        perror("Joystick button: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }
    isInitialized = false;
    Gpio_close(s_lineBtn);
}

int JoystickBtn_getValue()
{
    return counter;
}

static void JoystickBtn_doState()
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
