// TLA2024 Sample Code
// - Configure DAC to continuously read an input channel on the BeagleY-AI
// Reference Data Sheet: https://www.ti.com/lit/ds/symlink/tla2021.pdf
#include <hal/accelerometer.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#include "common/periodTimer.h"
#include "common/timing.h"
#include "../../app/include/drumBeats.h"
#include "hal/i2c.h"

// Device bus & address
#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x19

// Register in TLA2024
#define REG_CONFIGURATION 0x20
#define REG_DATA 0x00
#define REG_CTRL 0x21

#define REG_OUT_X_L 0x28
#define REG_OUT_X_H 0x29
#define REG_OUT_Y_L 0x2A
#define REG_OUT_Y_H 0x2B
#define REG_OUT_Z_L 0x2C
#define REG_OUT_Z_H 0x2D

#define DEBOUNCE_MS_X 550
#define DEBOUNCE_MS_Y 550
#define DEBOUNCE_MS_Z 520
#define BREAKPOINT_X 8000
#define BREAKPOINT_Y 8000
#define BREAKPOINT_Z 10000

static accel_data_t data;

static int16_t read_axis(int i2c_file_desc, uint8_t reg_l, uint8_t reg_h);

static bool isInitialized = false;
static bool isRunning = false;
static pthread_t mainThreadID;

static void do_state();

static long long timeChangedX, timeChangedY, timeChangedZ = 0;
static int16_t prev_raw_x = 0;
static int16_t prev_raw_y = 0;
static int16_t prev_raw_z = 0;
static int i2c_file_desc;
static int firstRun = 1;

accel_data_t Accel_getData() {
    return data;
}

static bool isPassedEnoughTime(int debounce, long long currentTime, long long startTime) {
    if ((currentTime - startTime) < debounce) {
        return false;
    }
    return true;
}

static void *accelUpdateThread(void *args)
{
    (void)args;

    data.x_changed = false;
    data.y_changed = false;
    data.z_changed = false;

    printf("Reading Accelerometer Data...\n");

    i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    
    // Configure accelerometer
    write_i2c_reg8(i2c_file_desc, REG_CONFIGURATION, 0x46);    

    sleep(1);

    while (isRunning) {
        do_state();
    }

    close(i2c_file_desc);

    return NULL;
}

static void do_state() { 

    int16_t raw_x = read_axis(i2c_file_desc, REG_OUT_X_L, REG_OUT_X_H);
    int16_t raw_y = read_axis(i2c_file_desc, REG_OUT_Y_L, REG_OUT_Y_H);
    int16_t raw_z = read_axis(i2c_file_desc, REG_OUT_Z_L, REG_OUT_Z_H);

    Period_markEvent(PERIOD_EVENT_ACCEL);

    if (firstRun == 1) {
        prev_raw_x = raw_x;
        prev_raw_y = raw_y;
        prev_raw_z = raw_z;
        firstRun = 0;
    }

    long long currentTime = Timing_getTimeMS();

    // debounce
    if (!isPassedEnoughTime(DEBOUNCE_MS_X, currentTime, timeChangedX)) {
        data.x_changed = false;
    }
    else {
        if (abs(raw_x - prev_raw_x) > BREAKPOINT_X) {
            timeChangedX = currentTime;
            data.x_changed = true;
        }
        else {
            data.x_changed = false;
        }
        prev_raw_x = raw_x;

    }
    if (!isPassedEnoughTime(DEBOUNCE_MS_Y, currentTime, timeChangedY)) {
        data.y_changed = false;
    }
    else {
        if (abs(raw_y - prev_raw_y) > BREAKPOINT_Y) {
            timeChangedY = currentTime;
            data.y_changed = true;
        }
        else {
            data.y_changed = false;
        }
        prev_raw_y = raw_y;
    }

    if (!isPassedEnoughTime(DEBOUNCE_MS_Z, currentTime, timeChangedZ)) {
        data.z_changed = false;
    } 
    else{
        if (abs(raw_z - prev_raw_z) > BREAKPOINT_Z) {
            timeChangedZ = currentTime;
            data.z_changed = true;
        }
        else {
            data.z_changed = false;
        }
        prev_raw_z = raw_z;
    }

    if (data.x_changed) {
        DrumBeats_playSound(SOUND_HI_HAT);
    }
    if (data.y_changed) {
        DrumBeats_playSound(SOUND_BASE_DRUM);
    }
    if (data.z_changed) {
        DrumBeats_playSound(SOUND_SNARE);
    }
}

static int16_t read_axis(int i2c_file_desc, uint8_t reg_l, uint8_t reg_h) {
    uint8_t low = read_i2c_reg8(i2c_file_desc, reg_l); 
    uint8_t high = read_i2c_reg8(i2c_file_desc, reg_h);
    int16_t value = ((int16_t)high << 8) | low;

    return value;
}

void Accel_getTiming(accel_stats_t* stats)
{
    assert(isInitialized);
    assert(stats != NULL);

    Period_statistics_t tmp;
    Period_getStatisticsAndClear(PERIOD_EVENT_ACCEL, &tmp);

    stats->minPeriodInMs = tmp.minPeriodInMs;
    stats->maxPeriodInMs = tmp.maxPeriodInMs;
    stats->avgPeriodInMs = tmp.avgPeriodInMs;
    stats->numSamples = tmp.numSamples;
}

void Accel_init(void)
{
    assert(!isInitialized);
    isInitialized = true;

    isRunning = true;

    int err = pthread_create(&mainThreadID, NULL, &accelUpdateThread, NULL);
    if (err) {
        printf("Accelerometer: failed to create main thread.\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
}

void Accel_cleanup(void)
{
    assert(isInitialized);
    isRunning = false;
    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("Accelerometer: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    int err = pthread_join(mainThreadID, NULL);
    if (err) {
        perror("Accelerometer: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }
    isInitialized = false;
}
