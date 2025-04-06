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
#include <math.h>

#include "common/periodTimer.h"
#include "common/timing.h"
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

static int16_t read_axis(int i2c_file_desc, uint8_t reg_l, uint8_t reg_h);
static void do_state();

static bool isInitialized = false;
static bool isRunning = false;
static pthread_t mainThreadID;
static int i2c_file_desc;

coordinates currentCoords;

coordinates Accel_getCurrentCoords() {
    return currentCoords;
}

static void *accelUpdateThread(void *args)
{
    (void)args;

    printf("Reading Accelerometer Data...\n");

    i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    
    // Configure accelerometer
    write_i2c_reg8(i2c_file_desc, REG_CONFIGURATION, 0x46);    

    while (isRunning) {
        do_state();
    }

    close(i2c_file_desc);

    return NULL;
}

static void do_state() { 

    int16_t raw_x = read_axis(i2c_file_desc, REG_OUT_X_L, REG_OUT_X_H);
    int16_t raw_y = read_axis(i2c_file_desc, REG_OUT_Y_L, REG_OUT_Y_H);

    const float SCALE = 16384.0f;

    float gy = raw_x / SCALE;
    float gx = raw_y / SCALE;

    currentCoords.x = -1 * gx;
    currentCoords.y = -1 * gy;
}

static int16_t read_axis(int i2c_file_desc, uint8_t reg_l, uint8_t reg_h) {
    uint8_t low = read_i2c_reg8(i2c_file_desc, reg_l); 
    uint8_t high = read_i2c_reg8(i2c_file_desc, reg_h);
    int16_t value = ((int16_t)high << 8) | low;

    return value;
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
