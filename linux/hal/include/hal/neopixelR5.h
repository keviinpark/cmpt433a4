// Manage neopixel program that runs on the R5

#ifndef _NEOPIXEL_R5_
#define _NEOPIXEL_R5_

#include <stdbool.h>
#include <stdint.h>

#define MEM_START_OFFSET 0x7000

#define MSG_SIZE   32

#define MSG_OFFSET MEM_START_OFFSET
#define LED_DELAY_MS_OFFSET (MSG_OFFSET + MSG_SIZE)
#define IS_BUTTON_PRESSED_OFFSET (LED_DELAY_MS_OFFSET + sizeof(uint32_t))
#define BTN_COUNT_OFFSET (IS_BUTTON_PRESSED_OFFSET + sizeof(uint32_t))
#define LOOP_COUNT_OFFSET (BTN_COUNT_OFFSET + sizeof(uint32_t))
#define C0_OFFSET (LOOP_COUNT_OFFSET + sizeof(uint32_t))
#define C1_OFFSET (C0_OFFSET + sizeof(uint32_t))
#define C2_OFFSET (C1_OFFSET + sizeof(uint32_t))
#define C3_OFFSET (C2_OFFSET + sizeof(uint32_t))
#define C4_OFFSET (C3_OFFSET + sizeof(uint32_t))
#define C5_OFFSET (C4_OFFSET + sizeof(uint32_t))
#define C6_OFFSET (C5_OFFSET + sizeof(uint32_t))
#define C7_OFFSET (C6_OFFSET + sizeof(uint32_t))
#define END_MEMORY_OFFSET (C7_OFFSET + sizeof(uint32_t))

static inline uint8_t getSharedMem_uint8(volatile void *base, uint32_t byte_offset) {
    volatile uint8_t *addr_tmp = (uint8_t *)base + byte_offset ;
    volatile uint8_t val = *addr_tmp;
    return val;
}

static inline void setSharedMem_uint8(volatile void* base, uint32_t byte_offset, uint8_t val) {
    volatile uint8_t *addr_tmp = (uint8_t *)base + byte_offset;
    volatile uint8_t val_tmp = val;
    // printf("        --> Set8  addr 0x%08x to %x\n", (uint32_t *)addr_tmp, val_tmp);
    *addr_tmp = val_tmp;
}

static inline uint32_t getSharedMem_uint32(volatile void *base, uint32_t byte_offset) {
    volatile uint32_t *addr_tmp = (uint32_t *) ((uint8_t *)base + byte_offset);
    volatile uint32_t val = *addr_tmp;
    return val;
}

static inline void setSharedMem_uint32(volatile void* base, uint32_t byte_offset, uint32_t val) {
    volatile uint32_t *addr_tmp = (uint32_t *) ((uint8_t *)base + byte_offset);
    volatile uint32_t val_tmp = val;
    // printf("        --> Set32 addr 0x%08x to %x\n", (uint32_t *)addr_tmp, val_tmp);
    *addr_tmp = val_tmp;
}

// OLD: These are replaced by the above functions.
#define MEM_UINT8(addr) "ERROR DO NOT USE THIS"
#define MEM_UINT32(addr) "ERROR DO NOT USE THIS"

#define NEO_NUM_LEDS     8
#define LED_OFF          0x00000000
#define LED_GREEN        0x0f000000
#define LED_GREEN_BRIGHT 0xff000000
#define LED_RED          0x000f0000
#define LED_RED_BRIGHT   0x00ff0000
#define LED_BLUE         0x00000f00
#define LED_BLUE_BRIGHT  0x0000ff00
#define LED_WHITE        0x000000ff

// init/cleanup
void Neopixel_init(void);
void Neopixel_cleanup(void);

// set color of an LED
void Neopixel_setLED(uint32_t index, uint32_t color);

// reset color of LEDs
void Neopixel_resetLEDs(void);

#endif
