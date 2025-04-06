// Draw stuff onto LCD
#ifndef _DRAW_STUFF_H_
#define _DRAW_STUFF_H_

void DrawStuff_init();
void DrawStuff_cleanup();

// update the main screen
void DrawStuff_updateScreen_main(char* beatName, char* volume, char* bpm);

#endif
