// Draw stuff onto LCD

#include "lcd.h"
#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h> 
#include "game.h"
#include "hal/joystickBtn.h"
#include "hal/accelerometer.h"

static UWORD *s_fb;
static bool isInitialized = false;
static bool isRunning = false;

static pthread_t lcdThread;

#define MS_PER_S 1000
#define US_PER_S 1000000
#define S_PER_MIN 60
#define LCD_DEV_DELAY_MS 2000
#define LCD_BACKLIGHT_LEVEL 1023
#define LCD_LINE_HEIGHT 20
#define LCD_VOL_POS_X 10
#define LCD_VOL_POS_Y_OFFSET 30
#define LCD_BPM_POS_X_OFFSET 110
#define LCD_BPM_POS_Y_OFFSET 30
#define BUFF_MAX_LEN 20

static void* lcdThreadProgram (void* arg) {

    (void)arg;

    while (isRunning) {

        int elapsedTimeMS = Game_getElapsedTimeMS();
        int total_seconds = elapsedTimeMS / MS_PER_S;
        int minutes = total_seconds / S_PER_MIN;
        int seconds = total_seconds % S_PER_MIN;

        char hits[BUFF_MAX_LEN];
        char misses[BUFF_MAX_LEN];
        char elapsedTimeStr[BUFF_MAX_LEN];

        snprintf(hits, BUFF_MAX_LEN, "Hits = %d", Game_getHits());
        snprintf(misses, BUFF_MAX_LEN, "Misses = %d", Game_getMisses());
        snprintf(elapsedTimeStr, BUFF_MAX_LEN, "%02d:%02d", minutes, seconds);

        DrawStuff_updateScreen_main(hits, misses, elapsedTimeStr);

    }

    return NULL;
}

void DrawStuff_init()
{
    assert(!isInitialized);

    // Module Init
    if(DEV_ModuleInit() != 0){
        DEV_ModuleExit();
        exit(0);
    }

    // LCD Init
    DEV_Delay_ms(LCD_DEV_DELAY_MS);
    LCD_1IN54_Init(HORIZONTAL);
    LCD_1IN54_Clear(WHITE);
    LCD_SetBacklight(LCD_BACKLIGHT_LEVEL);

    UDOUBLE Imagesize = LCD_1IN54_HEIGHT*LCD_1IN54_WIDTH*2;
    if((s_fb = (UWORD *)malloc(Imagesize)) == NULL) {
        perror("Failed to apply for black memory");
        exit(0);
    }
    isInitialized = true;
    isRunning = true;

    pthread_create(&lcdThread, NULL, lcdThreadProgram, NULL);


}
void DrawStuff_cleanup()
{
    
    assert(isInitialized);
    isRunning = false;

    LCD_1IN54_Clear(BLACK); 
    LCD_SetBacklight(0);

    int cancelErr = pthread_cancel(lcdThread);
    
    if (cancelErr) {
        perror("LCD: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }

    int err = pthread_join(lcdThread, NULL);
    if (err) {
        perror("LCD: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }
    // Module Exit
    free(s_fb);
    s_fb = NULL;
    DEV_ModuleExit();

    isInitialized = false;
}

void DrawStuff_updateScreen_main(char* hits, char* misses, char* timeElapsed)
{
    assert(isInitialized);

    const int x = 60;
    const int y = 80;

    // Initialize the RAM frame buffer to be blank (white)
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    // Draw into the RAM frame buffer
    // WARNING: Don't print strings with `\n`; will crash!
    Paint_DrawString_EN(x, y, hits, &Font20, WHITE, BLACK);
    Paint_DrawString_EN(x, y + 30, misses, &Font20, WHITE, BLACK);
    Paint_DrawString_EN(x, y + 60, timeElapsed, &Font20, WHITE, BLACK);

    // Send the RAM frame buffer to the LCD (actually display it)
    // Option 1) Full screen refresh (~1 update / second)
    // LCD_1IN54_Display(s_fb);
    // Option 2) Update just a small window (~15 updates / second)
    //           Assume font height <= 20
    LCD_1IN54_DisplayWindows(0, 0, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, s_fb);
}
