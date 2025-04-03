// Joystick module 
// Part of the Hardware Abstraction Layer (HAL) 
#include "hal/joystick.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "hal/i2c.h"

// Device bus & address
#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x48

// Register in TLA2024
#define REG_CONFIGURATION 0x01
#define REG_DATA 0x00

// Configuration reg contents for continuously sampling different channels
#define TLA2024_CHANNEL_CONF_0 0x83C2
#define TLA2024_CHANNEL_CONF_1 0x83D2
#define TLA2024_CHANNEL_CONF_2 0x83E2
#define TLA2024_CHANNEL_CONF_3 0x83F2

#define BREAKPOINT_UP 20
#define BREAKPOINT_DOWN 1610
#define USLEEP_DELAY 5000

// Allow module to ensure it has been initialized (once!)
static bool isInitialized = false;
static bool isRunning = false;
static pthread_t mainThreadID;

static Direction current_state = joystick_idle;
static void do_state();

Direction Joystick_getState(void)
{
    return current_state;
}

static void *joystickUpdateThread(void *args)
{
    (void)args;

    while (isRunning) {
        do_state();
    }

    return NULL;
}

void Joystick_init(void)
{
    assert(!isInitialized);
    isInitialized = true;

    isRunning = true;

    int err = pthread_create(&mainThreadID, NULL, &joystickUpdateThread, NULL);
    if (err) {
        printf("Joystick: failed to create main thread.\n");
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
}

static void do_state(void) {

    // read Y direction
    int i2c_file_desc_Y = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    write_i2c_reg16(i2c_file_desc_Y, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_0);
    usleep(USLEEP_DELAY);

    uint16_t raw_read_Y = read_i2c_reg16(i2c_file_desc_Y, REG_DATA);
    uint16_t value_Y = ((raw_read_Y & 0xFF) << 8) | ((raw_read_Y & 0xFF00) >> 8);
    value_Y = value_Y >> 4;
    close(i2c_file_desc_Y);

    // if joystick is pushed up or down over threshold, return up or down
    
    if (value_Y <= BREAKPOINT_UP) {
        current_state = joystick_up;
    }

    else if (value_Y >= BREAKPOINT_DOWN) {
        current_state = joystick_down;
    }

    else {
        current_state = joystick_idle;
    }
}

void Joystick_cleanup(void)
{
    assert(isInitialized);
    isRunning = false;
    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("Joystick: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    int err = pthread_join(mainThreadID, NULL);
    if (err) {
        perror("Joystick: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }
    isInitialized = false;
}

