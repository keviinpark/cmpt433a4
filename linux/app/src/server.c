#include "server.h"

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
#include "drumBeats.h"
#include "common/shutdown.h"
#include "common/timing.h"
#include "hal/audioMixer.h"

#define MAX_LEN 1024
#define PORT 12345

#define CMD_ENTER "\n"

#define BEAT_CMD "beat "
#define BPM_CMD "bpm "
#define VOL_CMD "vol "
#define GET_CMD (-1)

static bool isInitialized = false;
static bool isRunning = false;
static struct sockaddr_in sin;
static pthread_t mainThreadID;
static int socketDescriptor = -1;
struct sockaddr_in sinRemote;
static unsigned int sinLen = 0;

static void sendReply(char *str, size_t len, size_t sizeofLen)
{
    sendto(socketDescriptor, str, len, 0, (struct sockaddr*)&sinRemote, sinLen);
    memset(str, 0, sizeofLen);
}

// Get int from cmd parameter and also clamp the value
static int getParamInt(char *cmd, int clampMin, int clampMax) {
    char* param;
    char* param2;
    param = strtok(cmd, " ");
    if (param == NULL) {
        return clampMin;
    }
    param2 = strtok(NULL, "\n");

    int val = atoi(param2);

    // clamp value OR if user is requesting get rather than set/return get flag
    if (val == GET_CMD) {
        return GET_CMD;
    } else if (val > clampMax) {
        val = clampMax;
    } else if (val < clampMin) {
        val = clampMin;
    }

    return val;
}

static void parseCommand(char *rx, int rxLen)
{
    // init buffers
    int cmdLen = rxLen;
    char cmd[MAX_LEN];
    memset(cmd, 0, MAX_LEN);
    char reply[MAX_LEN];
    memset(reply, 0, MAX_LEN);

    strncpy(cmd, rx, cmdLen);

    // parse commands
    if (strncmp(cmd, "help\n", cmdLen) == 0 || strncmp(cmd, "?\n", cmdLen) == 0) {
        snprintf(
            reply, MAX_LEN,
            "    beat <num> - change the beat mode to num [0,2].\n"
            "    vol  <num> - change the volume to num [0, 100]\n"
            "    bpm  <num> - change the bpm to num [40, 300]\n"
            "    quit       - close server.\n"
            "\n"
            "    play hi_hat\n"
            "    play base_drum\n"
            "    play snare\n"
            "    play cyn_hard\n"
            "    play splash_hard\n"
            "    play tom_hi_hard\n"
        );
        sendReply(reply, strlen(reply), sizeof(reply));
    } else if (strncmp(cmd, "quit\n", cmdLen) == 0) {
        snprintf(
            reply, MAX_LEN,
            "    Shutting down...\n"
        );
        sendReply(reply, strlen(reply), sizeof(reply));

        isRunning = false;
        Shutdown_trigger();
    } else if (strncmp(cmd, BEAT_CMD, strlen(BEAT_CMD)) == 0) {
        int val = getParamInt(cmd, 0, TOTAL_BEATS - 1);

        if (val != GET_CMD) {
            printf("[Server] change beat to M%d\n", val);
            DrumBeats_setBeat(val);
        }

        snprintf(reply, MAX_LEN, "%d", DrumBeats_getCurrentBeatNumber());
        sendReply(reply, strlen(reply), sizeof(reply));
    } else if (strncmp(cmd, VOL_CMD, strlen(VOL_CMD)) == 0) {
        int val = getParamInt(cmd, VOLUME_MIN, VOLUME_MAX);

        if (val != GET_CMD) {
            printf("[Server] change volume to %d\n", val);
            AudioMixer_setVolume(val); // TODO maybe also change joystick value for volume
        }

        snprintf(reply, MAX_LEN, "%d", AudioMixer_getVolume());
        sendReply(reply, strlen(reply), sizeof(reply));
    } else if (strncmp(cmd, BPM_CMD, strlen(BPM_CMD)) == 0) {
        int val = getParamInt(cmd, BPM_MIN, BPM_MAX);

        if (val != GET_CMD) {
            printf("[Server] change bpm to %d\n", val);
            DrumBeats_setTempo(val);
        }

        snprintf(reply, MAX_LEN, "%d", DrumBeats_getTempo());
        sendReply(reply, strlen(reply), sizeof(reply));
    } else if (strncmp(cmd, "play hi_hat\n", cmdLen) == 0) {
        DrumBeats_playSound(SOUND_HI_HAT);
    } else if (strncmp(cmd, "play base_drum\n", cmdLen) == 0) {
        DrumBeats_playSound(SOUND_BASE_DRUM);
    } else if (strncmp(cmd, "play snare\n", cmdLen) == 0) {
        DrumBeats_playSound(SOUND_SNARE);
    } else if (strncmp(cmd, "play cyn_hard\n", cmdLen) == 0) {
        DrumBeats_playSound(SOUND_CYN_HARD);
    } else if (strncmp(cmd, "play splash_hard\n", cmdLen) == 0) {
        DrumBeats_playSound(SOUND_SPLASH_HARD);
    } else if (strncmp(cmd, "play tom_hi_hard\n", cmdLen) == 0) {
        DrumBeats_playSound(SOUND_TOM_HI_HARD);
    } else {
        snprintf(
            reply, MAX_LEN,
            "    Unknown command. See `help` or `?` for more information.\n"
        );
        sendReply(reply, strlen(reply), sizeof(reply));

    }
}

static void *listenThread(void *args)
{
    (void)args; // disable unused warning

    char cmd[MAX_LEN];

    while (isRunning) {
        sinLen = sizeof(sinRemote);
        int cmdLen = recvfrom(
            socketDescriptor,
            cmd, MAX_LEN - 1, 0,
            (struct sockaddr*) &sinRemote, &sinLen
        );
        cmd[cmdLen] = 0;

        parseCommand(cmd, cmdLen);
    }

    return NULL;
}

// init
void Server_init(void)
{
    assert(!isInitialized);
    isInitialized = true;

    // create and bind UDP socket

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        perror("Server: failed to create socket:");
        exit(EXIT_FAILURE);
    }

    int bindErr = bind(socketDescriptor, (struct sockaddr*)&sin, sizeof(sin));
    if (bindErr == -1) {
        perror("Server: failed to bind socket:");
        exit(EXIT_FAILURE);
    }

    // Create threads

    isRunning = true;
    int threadErr = pthread_create(&mainThreadID, NULL, &listenThread, NULL);
    if (threadErr) {
        perror("Server: failed to create main thread:");
        exit(EXIT_FAILURE);
    }
}

// cleanup
void Server_cleanup(void)
{
    assert(isInitialized);

    isRunning = false;

    int cancelErr = pthread_cancel(mainThreadID);
    if (cancelErr) {
        perror("Server: failed to cancel main thread:");
        exit(EXIT_FAILURE);
    }
    int threadErr = pthread_join(mainThreadID, NULL);
    if (threadErr) {
        perror("Server: failed to join main thread:");
        exit(EXIT_FAILURE);
    }

    close(socketDescriptor);

    isInitialized = false;
}
