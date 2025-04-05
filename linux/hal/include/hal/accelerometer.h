#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int numSamples;
    double minPeriodInMs;
    double maxPeriodInMs;
    double avgPeriodInMs;
} accel_stats_t;

typedef struct {
    double x;
    double y;
} coordinates;

// returns a struct of x, y, z acceleration data

coordinates Accel_getCurrentCoords();

void Accel_getTiming(accel_stats_t* stats);

void Accel_init(void);

void Accel_cleanup(void);

#endif