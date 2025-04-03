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
#include "hal/joystickBtn.h"
#include "drumBeats.h"
#include "hal/audioMixer.h"
#include "hal/accelerometer.h"

static UWORD *s_fb;
static bool isInitialized = false;

static pthread_t lcdThread;

#define NUM_SCREENS 3
#define BEAT_NAME_MAX_LEN 50
#define BUFF_MAX_LEN 15
#define INDEX_MAIN_SCREEN 0
#define INDEX_AUDIO_TIMING_SCREEN 1
#define INDEX_ACCEL_TIMING_SCREEN 2
#define US_PER_S 1000000
#define LCD_DEV_DELAY_MS 2000
#define LCD_BACKLIGHT_LEVEL 1023
#define LCD_LINE_HEIGHT 20
#define LCD_VOL_POS_X 10
#define LCD_VOL_POS_Y_OFFSET 30
#define LCD_BPM_POS_X_OFFSET 110
#define LCD_BPM_POS_Y_OFFSET 30

static void* lcdThreadProgram (void* arg) {

    (void)arg;

    while (true) {

        int joystickButtonVal = JoystickBtn_getValue();

        if (joystickButtonVal == INDEX_MAIN_SCREEN || (joystickButtonVal % NUM_SCREENS)  == INDEX_MAIN_SCREEN) {
            // first screen
            char beatName[BEAT_NAME_MAX_LEN] = "unknown beat";
            int beatNum = DrumBeats_getCurrentBeatNumber();
            if (beatNum == INDEX_ROCK_BEAT) {
                strcpy(beatName, "rock beat"); 
            } else if (beatNum == INDEX_CUSTOM_BEAT) {
                strcpy(beatName, "custom beat");
            } else if (beatNum == INDEX_NO_BEAT) {
                strcpy(beatName, "no beat");
            }

            char volume[BUFF_MAX_LEN];
            char bpm[BUFF_MAX_LEN];

            snprintf(volume, BUFF_MAX_LEN, "Vol = %d", AudioMixer_getVolume());
            snprintf(bpm, BUFF_MAX_LEN, "BPM = %d", DrumBeats_getTempo());

            DrawStuff_updateScreen_main(beatName, volume, bpm);
        }

        else if (joystickButtonVal == INDEX_AUDIO_TIMING_SCREEN || (joystickButtonVal % NUM_SCREENS)  == INDEX_AUDIO_TIMING_SCREEN) {
            // second screen

            AudioMixer_stats_t data;
            AudioMixer_getBufferStats(&data);
            double audioMinTime = data.minPeriodInMs;
            double audioMaxTime = data.maxPeriodInMs;
            double audioAvgTime = data.avgPeriodInMs;

            char min[BUFF_MAX_LEN];
            char max[BUFF_MAX_LEN];
            char avg[BUFF_MAX_LEN];

            snprintf(min, BUFF_MAX_LEN, "min = %.3lf", audioMinTime);
            snprintf(max, BUFF_MAX_LEN, "max = %.3lf", audioMaxTime);
            snprintf(avg, BUFF_MAX_LEN, "avg = %.3lf", audioAvgTime);

            DrawStuff_updateScreen_aux(0, min, max, avg);

        }

        else if (joystickButtonVal == INDEX_ACCEL_TIMING_SCREEN || (joystickButtonVal % NUM_SCREENS)  == INDEX_ACCEL_TIMING_SCREEN) {
            // third screen

            accel_stats_t data;
            Accel_getTiming(&data);
            double accelMinTime = data.minPeriodInMs;
            double accelMaxTime = data.maxPeriodInMs;
            double accelAvgTime = data.avgPeriodInMs;

            char min[BUFF_MAX_LEN];
            char max[BUFF_MAX_LEN];
            char avg[BUFF_MAX_LEN];

            snprintf(min, BUFF_MAX_LEN, "min = %.3lf", accelMinTime);
            snprintf(max, BUFF_MAX_LEN, "max = %.3lf", accelMaxTime);
            snprintf(avg, BUFF_MAX_LEN, "avg = %.3lf", accelAvgTime);

            DrawStuff_updateScreen_aux(1, min, max, avg);
        }

        // update every second
        usleep(US_PER_S);
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

    pthread_create(&lcdThread, NULL, lcdThreadProgram, NULL);


}
void DrawStuff_cleanup()
{
    assert(isInitialized);

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

void DrawStuff_updateScreen_aux(int type, char* min, char* max, char* average) 
{
    assert(isInitialized);

    const int x = 35;
    const int y = 100;

    // Initialize the RAM frame buffer to be blank (white)
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    // Draw into the RAM frame buffer
    // WARNING: Don't print strings with `\n`; will crash!
    if (type == 0) {
        Paint_DrawString_EN(x, y - (LCD_LINE_HEIGHT * 2), "Audio Timing", &Font20, WHITE, BLACK);
    }
    else {
        Paint_DrawString_EN(x - 2, y - (LCD_LINE_HEIGHT * 2), "Accel. Timing", &Font20, WHITE, BLACK);
    }
    Paint_DrawString_EN(x + LCD_LINE_HEIGHT, y, min, &Font16, WHITE, BLACK);
    Paint_DrawString_EN(x + LCD_LINE_HEIGHT, y + LCD_LINE_HEIGHT, max, &Font16, WHITE, BLACK);
    Paint_DrawString_EN(x + LCD_LINE_HEIGHT, y + (LCD_LINE_HEIGHT * 2), average, &Font16, WHITE, BLACK);

    // Send the RAM frame buffer to the LCD (actually display it)
    // Option 1) Full screen refresh (~1 update / second)
    // LCD_1IN54_Display(s_fb);
    // Option 2) Update just a small window (~15 updates / second)
    //           Assume font height <= 20
    LCD_1IN54_DisplayWindows(0, 0, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, s_fb);
}

void DrawStuff_updateScreen_main(char* beatName, char* volume, char* bpm)
{
    assert(isInitialized);

    const int x = 60;
    const int y = 80;

    // Initialize the RAM frame buffer to be blank (white)
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    // Draw into the RAM frame buffer
    // WARNING: Don't print strings with `\n`; will crash!
    Paint_DrawString_EN(x, y, beatName, &Font20, WHITE, BLACK);
    Paint_DrawString_EN(LCD_VOL_POS_X, LCD_1IN54_HEIGHT-LCD_VOL_POS_Y_OFFSET, volume, &Font16, WHITE, BLACK);
    Paint_DrawString_EN(LCD_1IN54_WIDTH-LCD_BPM_POS_X_OFFSET, LCD_1IN54_HEIGHT-LCD_BPM_POS_Y_OFFSET, bpm, &Font16, WHITE, BLACK);

    // Send the RAM frame buffer to the LCD (actually display it)
    // Option 1) Full screen refresh (~1 update / second)
    // LCD_1IN54_Display(s_fb);
    // Option 2) Update just a small window (~15 updates / second)
    //           Assume font height <= 20
    LCD_1IN54_DisplayWindows(0, 0, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, s_fb);
}
