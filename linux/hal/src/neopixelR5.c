#include "hal/neopixelR5.h"

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>

// General R5 Memomry Sharing Routine
// ----------------------------------------------------------------
#define ATCM_ADDR     0x79000000  // MCU ATCM (p59 TRM)
#define BTCM_ADDR     0x79020000  // MCU BTCM (p59 TRM)
#define MEM_LENGTH    0x8000

static bool isInitialized = false;
static volatile void *r5base = NULL;

// Return the address of the base address of the ATCM memory region for the R5-MCU
volatile void* getR5MmapAddr(void)
{
    // Access /dev/mem to gain access to physical memory (for memory-mapped devices/specialmemory)
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("ERROR: could not open /dev/mem; Did you run with sudo?");
        exit(EXIT_FAILURE);
    }

    // Inside main memory (fd), access the part at offset BTCM_ADDR:
    // (Get points to start of R5 memory after it's memory mapped)
    volatile void* pR5Base = mmap(0, MEM_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BTCM_ADDR);
    if (pR5Base == MAP_FAILED) {
        perror("ERROR: could not map memory");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return pR5Base;
}

void freeR5MmapAddr(volatile void* pR5Base)
{
    if (munmap((void*) pR5Base, MEM_LENGTH)) {
        perror("R5 munmap failed");
        exit(EXIT_FAILURE);
    }
}

void Neopixel_init(void)
{
    assert(!isInitialized);

    // Get access to shared memory for my uses
    r5base = getR5MmapAddr();

    Neopixel_resetLEDs();

    isInitialized = true;
}

void Neopixel_cleanup(void)
{
    assert(isInitialized);

    Neopixel_resetLEDs();

    freeR5MmapAddr(r5base);

    isInitialized = false;
}

// Set the color of an LED
void Neopixel_setLED(uint32_t index, uint32_t color)
{
    assert(index < NEO_NUM_LEDS);

    setSharedMem_uint32(r5base, C0_OFFSET + index * sizeof(uint32_t), color);
}

// reset color of LEDs
void Neopixel_resetLEDs(void)
{
    for (int i = 0; i < NEO_NUM_LEDS; i++) {
        Neopixel_setLED(i, LED_OFF);
    }
}
