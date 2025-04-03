// Thread that plays drum beats

#ifndef _DRUM_BEATS_H_
#define _DRUM_BEATS_H_

#define TOTAL_BEATS 3
#define BPM_MAX 300
#define BPM_MIN 40
#define VOLUME_MAX 100
#define VOLUME_MIN 0

#define INDEX_NO_BEAT 2
#define INDEX_ROCK_BEAT 0
#define INDEX_CUSTOM_BEAT 1

enum DrumBeats_sound {
    SOUND_HI_HAT = 0,
    SOUND_BASE_DRUM,
    SOUND_SNARE,
    SOUND_CYN_HARD,
    SOUND_SPLASH_HARD,
    SOUND_TOM_HI_HARD
};
typedef enum DrumBeats_sound DrumBeats_sound;

// Start playing drum beats
void DrumBeats_init(void);

// Stop playing drum beats
void DrumBeats_cleanup(void);

// Get the current beat number
int DrumBeats_getCurrentBeatNumber(void);
// set the beat to play
void DrumBeats_setBeat(int i);

// Get/set the current BPM.
// setTempo is clamped to min and max values.
int DrumBeats_getTempo(void);
void DrumBeats_setTempo(int);

// Play a single drum beat sound
void DrumBeats_playSound(DrumBeats_sound sound);

#endif
