// Manage neopixel program that runs on the R5

#ifndef _NEOPIXEL_R5_
#define _NEOPIXEL_R5_

#include <stdbool.h>
#include <stdint.h>

#define MSG_OFFSET 0
#define MSG_SIZE   32
#define LED_DELAY_MS_OFFSET (MSG_OFFSET + MSG_SIZE)
#define IS_BUTTON_PRESSED_OFFSET (LED_DELAY_MS_OFFSET + sizeof(uint32_t))
#define BTN_COUNT_OFFSET (IS_BUTTON_PRESSED_OFFSET + sizeof(uint32_t))
#define LOOP_COUNT_OFFSET (BTN_COUNT_OFFSET + sizeof(uint32_t))
#define END_MEMORY_OFFSET (LOOP_COUNT_OFFSET + sizeof(uint32_t))

#define MEM_UINT8(addr) *(uint8_t*)(addr)
#define MEM_UINT32(addr) *(uint32_t*)(addr)

// init/cleanup
void Neopixel_init(void);
void Neopixel_cleanup(void);

// print shared data
void Neopixel_printData(void);

#endif
