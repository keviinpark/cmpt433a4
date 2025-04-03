// Print information to stdout.

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "terminal.h"
#include "drumBeats.h"
#include "hal/audioMixer.h"
#include "hal/accelerometer.h"
#include "common/shutdown.h"
#include "common/timing.h"
#include "common/periodTimer.h"

#define OUTPUT_WAIT_MS 1000

static bool isRunning = false;
static bool isInitialized = false;
static pthread_t mainThreadID;

static void *outputThread(void *args)
{
    (void)args; // disable unused error

    while (isRunning) {
        Timing_sleepForMS(OUTPUT_WAIT_MS);

        AudioMixer_stats_t data;
        AudioMixer_getBufferStats(&data);

        int currentBeat = DrumBeats_getCurrentBeatNumber();
        int tempo = DrumBeats_getTempo();
        int volume = AudioMixer_getVolume();
        double audioMinTime = data.minPeriodInMs;
        double audioMaxTime = data.maxPeriodInMs;
        double audioAvgTime = data.avgPeriodInMs;
        int audioNumSamples = data.numSamples;

        // TODO implement accelerometer

        accel_stats_t accelData;
        Accel_getTiming(&accelData);
        double accelMinTime = accelData.minPeriodInMs;
        double accelMaxTime = accelData.maxPeriodInMs;
        double accelAvgTime = accelData.avgPeriodInMs;
        int accelNumSamples = accelData.numSamples;
        
        printf(
            "M%d %3dbpm vol:%3d  Audio[%6.3lf, %6.3lf] avg %6.3lf/%d  Accel[%6.3lf, %03.3lf] avg %6.3lf/%d\n",
            currentBeat, tempo, volume,
            audioMinTime, audioMaxTime, audioAvgTime, audioNumSamples,
            accelMinTime, accelMaxTime, accelAvgTime, accelNumSamples
        );
    }

    return NULL;
}

void Terminal_init(void)
{
    assert(!isInitialized);
    isInitialized = true;

    isRunning = true;
    int threadErr = pthread_create(&mainThreadID, NULL, &outputThread, NULL);
    if (threadErr) {
        printf("Terminal: failed to create main thread.\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
}

void Terminal_cleanup(void)
{
    assert(isInitialized);

    isRunning = false;
    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("Terminal: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    int threadErr = pthread_join(mainThreadID, NULL);
    if (threadErr) {
        printf("Terminal: failed to join main thread.\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }

    isInitialized = false;
}

