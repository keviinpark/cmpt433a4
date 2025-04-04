// Manage neopixel program that runs on the R5

#ifndef _NEOPIXEL_R5_
#define _NEOPIXEL_R5_

#include <stdbool.h>
#include <stdint.h>

#define MSG_OFFSET          0
#define MSG_SIZE            32
#define LED_DELAY_MS_OFFSET (MSG_OFFSET + MSG_SIZE)
#define INIT_OFFSET         (LED_DELAY_MS_OFFSET + sizeof(uint32_t))
#define COLOR_0_OFFSET      (INIT_OFFSET + sizeof(uint32_t))
#define COLOR_1_OFFSET      (COLOR_0_OFFSET + sizeof(uint32_t))
#define COLOR_2_OFFSET      (COLOR_1_OFFSET + sizeof(uint32_t))
#define COLOR_3_OFFSET      (COLOR_2_OFFSET + sizeof(uint32_t))
#define COLOR_4_OFFSET      (COLOR_3_OFFSET + sizeof(uint32_t))
#define COLOR_5_OFFSET      (COLOR_4_OFFSET + sizeof(uint32_t))
#define COLOR_6_OFFSET      (COLOR_5_OFFSET + sizeof(uint32_t))
#define COLOR_7_OFFSET      (COLOR_6_OFFSET + sizeof(uint32_t))
#define END_MEMORY_OFFSET   (COLOR_7_OFFSET + sizeof(uint32_t))

#define MEM_UINT8(addr)  *(uint8_t*)(addr)
#define MEM_UINT32(addr) *(uint32_t*)(addr)

#define NEO_NUM_LEDS     8
#define LED_OFF          0x00000000
#define LED_GREEN        0x0f000000
#define LED_GREEN_BRIGHT 0xff000000
#define LED_RED          0x000f0000
#define LED_RED_BRIGHT   0x00ff0000
#define LED_BLUE         0x00000f00
#define LED_BLUE_BRIGHT  0x0000ff00

// init/cleanup
void Neopixel_init(void);
void Neopixel_cleanup(void);

// set color of an LED
void Neopixel_setLED(uint32_t index, uint32_t color);

#endif
