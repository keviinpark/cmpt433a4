// Manages the loop of the game (tracks hits/misses/plays led animation)
#ifndef _GAME_H_
#define _GAME_H_

// init/cleanup
void Game_init(void);
void Game_cleanup(void);

// get # of hits/misses from the player
int getHits(void);
int getMisses(void);

// get elapsed time in milliseconds
long long Game_getElapsedTimeMS(void);

#endif
