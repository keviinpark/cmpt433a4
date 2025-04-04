#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

// R5 Shared Memory Note
// - It seems that using a struct for the ATCM memory does not work 
//   (hangs when accessing memory via a struct pointer).
// - Therefore, using an array.

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

#define MEM_UINT8(addr) *(uint8_t*)(addr)
#define MEM_UINT32(addr) *(uint32_t*)(addr)
#endif
