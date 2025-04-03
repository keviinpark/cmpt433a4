// Manage application shutdown.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>

#include "common/shutdown.h"

static bool isInitialized = false;

static pthread_mutex_t shutdownMutex = PTHREAD_MUTEX_INITIALIZER;
static bool isLocked = false;

// init
void Shutdown_init(void)
{
    assert(!isInitialized);
    isInitialized = true;

    isLocked = true;
    int err = pthread_mutex_lock(&shutdownMutex);
    if (err) {
        isLocked = false;
        printf("Shutdown: shutdown mutex error\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
}

// block main process
void Shutdown_wait(void)
{
    assert(isInitialized);

    // attempting a 2nd lock will cause the process to wait
    isLocked = true;
    int err = pthread_mutex_lock(&shutdownMutex);
    if (err) {
        isLocked = false;
        printf("Shutdown: shutdown mutex error\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }

    // from here on out if (!err), Shutdown_trigger() has been called
    err = pthread_mutex_unlock(&shutdownMutex);
    if (err) {
        printf("Shutdown: shutdown mutex error\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
    isLocked = false;
}

// unblock main process
void Shutdown_trigger(void)
{
    assert(isInitialized);

    // do nothing if it mutex is not locked
    if (!isLocked) {
        return;
    }

    int err = pthread_mutex_unlock(&shutdownMutex);
    if (err) {
        printf("Shutdown: shutdown mutex error\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }

    isLocked = false;
}

// cleanup
void Shutdown_cleanup(void)
{
    assert(isInitialized);

    isLocked = false;
    isInitialized = false;
}
