/*
 * R5 Sample Code for Shared Memory with Linux
 */

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

#include "sharedDataLayout.h"

// Memory
// ----------------------------------------
#define SHARED_MEM_BTCM_START 0x00000000  // TRM p848
#define SHARED_MEM_ATCM_START 0x00041010  // TRM p849
static void *pSharedMem = (void *) SHARED_MEM_BTCM_START;

// Access GPIO (for demonstration purposes)
// ----------------------------------------
// 1,000,000 uSec = 1000 msec = 1 sec
#define MICRO_SECONDS_PER_MILI_SECOND   (1000)
#define DEFAULT_LED_DELAY_MS            (100)

// NeoPixel Timing
// NEO_<one/zero>_<on/off>_NS
// (These times are what the hardware needs; the delays below are hand-tuned to give these).
#define NEO_NUM_LEDS          8   // # LEDs in our string
//
#define NEO_ONE_ON_NS       700   // Stay on 700ns
#define NEO_ONE_OFF_NS      600   // (was 800)
#define NEO_ZERO_ON_NS      350
#define NEO_ZERO_OFF_NS     800   // (Was 600)
#define NEO_RESET_NS      60000   // Must be at least 50us, use 60us

// Delay time includes 1 GPIO set action.
volatile int junk_delay = 0;
#define DELAY_350_NS() {}
#define DELAY_600_NS() {for (junk_delay=0; junk_delay<9 ;junk_delay++);}
#define DELAY_700_NS() {for (junk_delay=0; junk_delay<16 ;junk_delay++);}
#define DELAY_800_NS() {for (junk_delay=0; junk_delay<23 ;junk_delay++);}

#define DELAY_NS(ns) do {int target = k_cycle_get_32() + k_ns_to_cyc_near32(ns); \
	while(k_cycle_get_32() < target) ; } while(false)

#define NEO_DELAY_ONE_ON()     DELAY_700_NS()
#define NEO_DELAY_ONE_OFF()    DELAY_600_NS()
#define NEO_DELAY_ZERO_ON()    DELAY_350_NS()
#define NEO_DELAY_ZERO_OFF()   DELAY_800_NS()
#define NEO_DELAY_RESET()      {DELAY_NS(NEO_RESET_NS);}

// Device tree nodes for pin aliases
#define LED0_NODE DT_ALIAS(led0)
#define BTN0_NODE DT_ALIAS(btn0)
#define NEOPIXEL_NODE DT_ALIAS(neopixel)

#define LED_0 0
#define LED_1 1
#define LED_2 2
#define LED_3 3
#define LED_4 4
#define LED_5 5
#define LED_6 6
#define LED_7 7

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);
static const struct gpio_dt_spec neopixel = GPIO_DT_SPEC_GET(NEOPIXEL_NODE, gpios);

static void initialize_gpio(const struct gpio_dt_spec *pPin, int direction) 
{
	if (!gpio_is_ready_dt(pPin)) {
		printf("ERROR: GPIO pin not ready read; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}

	int ret = gpio_pin_configure_dt(pPin, direction);
	if (ret < 0) {
		printf("ERROR: GPIO Pin Configure issue; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

	initialize_gpio(&led, GPIO_OUTPUT_ACTIVE);
	initialize_gpio(&btn, GPIO_INPUT);
	initialize_gpio(&neopixel, GPIO_OUTPUT_ACTIVE);

	// COLOURS
	// - 1st element in array is 1st (bottom) on LED strip; last element is last on strip (top)
	// - Bits: {Green/8 bits} {Red/8 bits} {Blue/8 bits} {White/8 bits}
	uint32_t color[NEO_NUM_LEDS] = {
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,

		//0x0f000000, // Green
		//0x000f0000, // Red
		//0x00000f00, // Blue
		//0x0000000f, // White
		//0x0f0f0f00, // White (via RGB)
		//0x0f0f0000, // Yellow
		//0x000f0f00, // Purple
		//0x0f000f00, // Teal
		// Try these; they are birght! 
		// (You'll need to comment out some of the above)
		// 0xff000000, // Green Bright
		// 0x00ff0000, // Red Bright
		// 0x0000ff00, // Blue Bright
		// 0xffffff00, // White
		// 0xff0000ff, // Green Bright w/ Bright White
		// 0x00ff00ff, // Red Bright w/ Bright White
		// 0x0000ffff, // Blue Bright w/ Bright White
		// 0xffffffff, // White w/ Bright White
	};

	printf("Contents of Shared Memory ATCM:\n");
	for (int i = 0; i < END_MEMORY_OFFSET; i++) {
		char* addr = (char*)pSharedMem + i;
		printf("0x%08x = %2x (%c)\n", (uint32_t) addr, *addr, *addr);
	}

	// Addresses:
	printf("Addresses\n");
	printf("  %20s = 0x%08x\n", "msg", MSG_OFFSET);
	printf("  %20s = 0x%08x\n", "ledDelay_ms", LED_DELAY_MS_OFFSET);

	// Setup defaults
	printf("Writing to ATCM...\n");
	printf("   ..1\n");
	strcpy((char*) MSG_OFFSET, "Hello from R5 World (Take 6)!");

	printf("Contents of Shared Memory ATCM after string init:\n");
	for (int i = 0; i < END_MEMORY_OFFSET; i++) {
		char* addr = (char*)pSharedMem + i;
		printf("0x%08x = %2x (%c)\n", (uint32_t) addr, *addr, *addr);
	}

	MEM_UINT32(LED_DELAY_MS_OFFSET) = DEFAULT_LED_DELAY_MS;
	MEM_UINT32(INIT_OFFSET) = 0;

	printf("Contents of Shared Memory ATCM After Write:\n");
	for (int i = 0; i < END_MEMORY_OFFSET; i++) {
		char* addr = (char*)pSharedMem + i;
		printf("0x%08x = %2x (%c)\n", (uint32_t) addr, *addr, *addr);
	}

	uint32_t initialized = 0;
	while (true) {
		initialized = MEM_UINT32(INIT_OFFSET);

		if (initialized == 1) {
			color[LED_7] = MEM_UINT32(COLOR_7_OFFSET);
			color[LED_6] = MEM_UINT32(COLOR_6_OFFSET);
			color[LED_5] = MEM_UINT32(COLOR_5_OFFSET);
			color[LED_4] = MEM_UINT32(COLOR_4_OFFSET);
			color[LED_3] = MEM_UINT32(COLOR_3_OFFSET);
			color[LED_2] = MEM_UINT32(COLOR_2_OFFSET);
			color[LED_1] = MEM_UINT32(COLOR_1_OFFSET);
			color[LED_0] = MEM_UINT32(COLOR_0_OFFSET);
		}

		// Neopixel (led state basically unused)
		gpio_pin_set_dt(&neopixel, 0);
		DELAY_NS(NEO_RESET_NS);

		for(int j = 0; j < NEO_NUM_LEDS; j++) {
			for(int i = 31; i >= 0; i--) {
				if(color[j] & ((uint32_t)0x1 << i)) {
					gpio_pin_set_dt(&neopixel, 1);
					NEO_DELAY_ONE_ON();
					gpio_pin_set_dt(&neopixel, 0);
					NEO_DELAY_ONE_OFF();
				} else {
					gpio_pin_set_dt(&neopixel, 1);
					NEO_DELAY_ZERO_ON();
					gpio_pin_set_dt(&neopixel, 0);
					NEO_DELAY_ZERO_OFF();
				}
			}
		}

		gpio_pin_set_dt(&neopixel, 0);
		NEO_DELAY_RESET();

		// Wait for delay (set by Linux app)
		uint32_t delay = MEM_UINT32(LED_DELAY_MS_OFFSET);
		printf("Waiting for %d ms\n", delay);
		k_busy_wait(delay * MICRO_SECONDS_PER_MILI_SECOND);	
	}

	return 0;
}
