// Thread that plays drum beats

#include "drumBeats.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <unistd.h>

#include "hal/audioMixer.h"
#include "hal/rotaryEncoder.h"
#include "hal/rotaryEncoderBtn.h"
#include "hal/joystick.h"
#include "common/timing.h"

#define S_PER_MIN          60
#define MS_PER_S           1000
#define BPM_DEFAULT        120
#define BPM_MAX            300
#define BPM_MIN            40
#define BPM_INC            5
#define BPM_DEC            5
#define VOLUME_DEFAULT     80
#define VOLUME_MAX         100
#define VOLUME_CHANGE      5
#define SOUND_DEFAULT_VAL  NULL
#define SOURCE_BASE_DRUM   "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define SOURCE_HI_HAT      "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define SOURCE_SNARE       "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
#define SOURCE_CYN_HARD    "beatbox-wav-files/100056__menegass__gui-drum-cyn-hard.wav"
#define SOURCE_SPLASH_HARD "beatbox-wav-files/100060__menegass__gui-drum-splash-hard.wav"
#define SOURCE_TOM_HI_HARD "beatbox-wav-files/100062__menegass__gui-drum-tom-hi-hard.wav"

static wavedata_t soundHiHat;
static wavedata_t soundBaseDrum;
static wavedata_t soundSnare;
static wavedata_t soundCynHard;
static wavedata_t soundSplashHard;
static wavedata_t soundTomHiHard;

#define BEAT_SIZE_PER_ITERATION 8
#define MAX_SOURCES_AT_ONCE 2

#define INDEX_NO_BEAT 2
static wavedata_t* noBeat[BEAT_SIZE_PER_ITERATION][MAX_SOURCES_AT_ONCE] = {
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
    { NULL, NULL },
};

#define INDEX_ROCK_BEAT 0
static wavedata_t* rockBeat[BEAT_SIZE_PER_ITERATION][MAX_SOURCES_AT_ONCE] = {
    { &soundHiHat, &soundBaseDrum },
    { &soundHiHat, NULL },
    { &soundHiHat, &soundSnare },
    { &soundHiHat, NULL },
    { &soundHiHat, &soundBaseDrum },
    { &soundHiHat, NULL },
    { &soundHiHat, &soundSnare },
    { &soundHiHat, NULL },
};

#define INDEX_CUSTOM_BEAT 1
static wavedata_t* customBeat[BEAT_SIZE_PER_ITERATION][MAX_SOURCES_AT_ONCE] = {
    { &soundTomHiHard, NULL },
    { &soundCynHard, &soundSplashHard },
    { &soundCynHard, &soundSplashHard },
    { &soundCynHard, &soundSplashHard },
    { &soundCynHard, &soundSplashHard },
    { &soundCynHard, &soundSplashHard },
    { &soundCynHard, &soundSplashHard },
    { &soundCynHard, NULL },
};

static atomic_int playingBeat = INDEX_ROCK_BEAT;
static long long playingBeatIndex = 0;
static int bpm = BPM_DEFAULT;
static int volume = VOLUME_DEFAULT;
static bool isInitialized = false;
static bool isRunning = false;
static pthread_t mainThreadID;
static pthread_t volumeControlThreadID;

// connect volume control to joystick control
static void* volumeControlThread(void* args)
{
    (void)args; // disable unused err

    while (isRunning) {
        Direction currentDirection = Joystick_getState();
        if (currentDirection == joystick_idle) {
            // do nothing if no joystick activity
            continue;
        }

        if (currentDirection == joystick_up) {
            volume += VOLUME_CHANGE;
            if (volume > VOLUME_MAX) {
                volume = VOLUME_MAX;
            }
            AudioMixer_setVolume(volume);
        }

        else if (currentDirection == joystick_down) {
            volume -= VOLUME_CHANGE;
            if (volume < 0) {
                volume = 0;
            }
            AudioMixer_setVolume(volume);
        }
        sleep(1);
    }
    return NULL;
}

// TODO connect this to rotary encoder and stuff
static void* drumBeatThread(void* args)
{
    (void)args; // disable unused err

    while (isRunning) {
        // beat mode
        int newBeat = RotaryEncoderBtn_getValue();
        if (newBeat == playingBeat) {
            // do nothing if it is already playing the same beat
        } else {
            playingBeat = newBeat;
            playingBeatIndex = 0; // start from beginning when switching beats
        }

        // bpm
        bpm = RotaryEncoder_getValue();

        if (playingBeatIndex >= BEAT_SIZE_PER_ITERATION) { // prevent long long overflow by resetting its value
            playingBeatIndex = 0;
        }

        // Queue beats
        for (int j = 0; j < MAX_SOURCES_AT_ONCE; j++) {
            wavedata_t* wavedata;
            if (playingBeat == INDEX_ROCK_BEAT) {
                wavedata = rockBeat[playingBeatIndex % BEAT_SIZE_PER_ITERATION][j];
            } else if (playingBeat == INDEX_CUSTOM_BEAT) {
                wavedata = customBeat[playingBeatIndex % BEAT_SIZE_PER_ITERATION][j];
            } else {
                wavedata = noBeat[playingBeatIndex % BEAT_SIZE_PER_ITERATION][j];
            }

            if (wavedata == NULL) {
                break;
            }
            AudioMixer_queueSound(wavedata);
        }

        // Time For Half Beat [sec] = 60 [sec/min] / BPM / 2 [half-beats per beat]
        Timing_sleepForMS((S_PER_MIN * MS_PER_S / bpm) / 2);

        playingBeatIndex++;
    }

    return NULL;
}

// Start playing drum beats
void DrumBeats_init(void)
{
    assert(!isInitialized);
    isInitialized = true;

    AudioMixer_readWaveFileIntoMemory(SOURCE_HI_HAT, &soundHiHat);
    AudioMixer_readWaveFileIntoMemory(SOURCE_BASE_DRUM, &soundBaseDrum);
    AudioMixer_readWaveFileIntoMemory(SOURCE_SNARE, &soundSnare);
    AudioMixer_readWaveFileIntoMemory(SOURCE_CYN_HARD, &soundCynHard);
    AudioMixer_readWaveFileIntoMemory(SOURCE_SPLASH_HARD, &soundSplashHard);
    AudioMixer_readWaveFileIntoMemory(SOURCE_TOM_HI_HARD, &soundTomHiHard);

    AudioMixer_setVolume(VOLUME_DEFAULT);

    // bpm control
    RotaryEncoder_setMaxValue(BPM_MAX);
    RotaryEncoder_setMinValue(BPM_MIN);
    RotaryEncoder_setDecrementValue(BPM_DEC);
    RotaryEncoder_setIncrementValue(BPM_INC);
    RotaryEncoder_setDefaultValue(BPM_DEFAULT);

    // selected beat control
    RotaryEncoderBtn_setStartValue(0);
    RotaryEncoderBtn_setDefaultValue(INDEX_ROCK_BEAT);
    RotaryEncoderBtn_setEndValue(TOTAL_BEATS - 1);

    isRunning = true;
    int err = pthread_create(&mainThreadID, NULL, &drumBeatThread, NULL);
    if (err) {
        perror("DrumBeats: failed to create main thread:");
        exit(EXIT_FAILURE);
    }

    int err3 = pthread_create(&volumeControlThreadID, NULL, &volumeControlThread, NULL);
    if (err3) {
        perror("DrumBeats: failed to create volume control thread:");
        exit(EXIT_FAILURE);
    }
}

// Play the next beat
void DrumBeats_setBeat(int i)
{
    assert(isInitialized);

    playingBeat = i;

    if (playingBeat < 0) {
        playingBeat = 0;
    }

    if (playingBeat >= TOTAL_BEATS) { // loop back around
        playingBeat = INDEX_ROCK_BEAT;
    }

    RotaryEncoderBtn_setDefaultValue(playingBeat);
}

// Stop playing drum beats
void DrumBeats_cleanup(void)
{
    assert(isInitialized);

    isRunning = false;
    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("DrumBeats: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }
    int cancelErr3 = pthread_cancel(volumeControlThreadID);
    if (cancelErr3) {
        perror("DrumBeats: failed to cancel volume thread:");
        exit(EXIT_FAILURE);
    }

    int err = pthread_join(mainThreadID, NULL);
    if (err) {
        perror("DrumBeats: failed to join main thread:");
        exit(EXIT_FAILURE);
    }
    int err3 = pthread_join(volumeControlThreadID, NULL);
    if (err3) {
        perror("DrumBeats: failed to join volume control thread:");
        exit(EXIT_FAILURE);
    }

    AudioMixer_freeWaveFileData(&soundHiHat);
    AudioMixer_freeWaveFileData(&soundBaseDrum);
    AudioMixer_freeWaveFileData(&soundSnare);
    AudioMixer_freeWaveFileData(&soundCynHard);
    AudioMixer_freeWaveFileData(&soundSplashHard);
    AudioMixer_freeWaveFileData(&soundTomHiHard);

    isInitialized = false;
}

int DrumBeats_getTempo(void)
{
    assert(isInitialized);

    return bpm;
}

void DrumBeats_setTempo(int val)
{
    assert(isInitialized);

    int tempo = val;
    if (tempo < BPM_MIN) {
        tempo = BPM_MIN;
    } else if (tempo > BPM_MAX) {
        tempo = BPM_MAX;
    }

    bpm = tempo;
    RotaryEncoder_setDefaultValue(tempo);
}

int DrumBeats_getCurrentBeatNumber(void)
{
    assert(isInitialized);

    return playingBeat;
}

void DrumBeats_playSound(DrumBeats_sound sound)
{
    assert(isInitialized);

    switch (sound) {
        case SOUND_BASE_DRUM:
            AudioMixer_queueSound(&soundBaseDrum);
            break;
        case SOUND_SNARE:
            AudioMixer_queueSound(&soundSnare);
            break;
        case SOUND_CYN_HARD:
            AudioMixer_queueSound(&soundCynHard);
            break;
        case SOUND_SPLASH_HARD:
            AudioMixer_queueSound(&soundSplashHard);
            break;
        case SOUND_TOM_HI_HARD:
            AudioMixer_queueSound(&soundTomHiHard);
            break;
        default:
            AudioMixer_queueSound(&soundHiHat);
            break;
    }
}
