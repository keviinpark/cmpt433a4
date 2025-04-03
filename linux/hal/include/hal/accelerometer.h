#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool x_changed;
    bool y_changed;
    bool z_changed;
} accel_data_t;

typedef struct {
    int numSamples;
    double minPeriodInMs;
    double maxPeriodInMs;
    double avgPeriodInMs;
} accel_stats_t;

// returns a struct of x, y, z acceleration data
accel_data_t Accel_getData();

void Accel_getTiming(accel_stats_t* stats);

void Accel_init(void);

void Accel_cleanup(void);

#endif