#include "effects.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>

Mix_Chunk* EFFECTS::s_moveSound = nullptr;
Mix_Chunk* EFFECTS::s_invalidSound = nullptr;
Mix_Chunk* EFFECTS::s_captureSound = nullptr;
Mix_Chunk* EFFECTS::s_checkSound = nullptr;
bool EFFECTS::s_initialized = false;

bool EFFECTS::initAudio(int frequency, Uint16 format, int channels, int chunksize) {
    if (s_initialized) {
        return true; // already initialized
    }
    // Initialize SDL_mixer; assume SDL_Init(SDL_INIT_AUDIO) already done or include audio flag earlier
    // If we need OGG/MP3 support, we must ensure Mix_Init is called elsewhere before this or add here:
    if (!(Mix_Init(MIX_INIT_OGG) & MIX_INIT_OGG)) { SDL_Log("Mix_Init OGG failed: %s", Mix_GetError()); }
    if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
        SDL_Log("Effects::initAudio: Mix_OpenAudio failed: %s", Mix_GetError());
        return false;
    }
    // Optionally increase number of channels for simultaneous sounds:
    Mix_AllocateChannels(16);
    s_initialized = true;
    return true;
}

bool EFFECTS::loadSounds(   const std::string& moveSoundPath,
                            const std::string& captureSoundPath,
                            const std::string& invalidSoundPath,
                            const std::string& checkSoundPath)
{
    bool anyLoaded = false;

    // Free previous if any
    if (s_moveSound) {
        Mix_FreeChunk(s_moveSound);
        s_moveSound = nullptr;
    }
    if (s_captureSound) {
        Mix_FreeChunk(s_captureSound);
        s_captureSound = nullptr;
    }
    if (s_invalidSound) {
        Mix_FreeChunk(s_invalidSound);
        s_invalidSound = nullptr;
    }
    if (s_checkSound) {
        Mix_FreeChunk(s_checkSound);
        s_checkSound = nullptr;
    }

    if (!moveSoundPath.empty()) {
        s_moveSound = Mix_LoadWAV(moveSoundPath.c_str());
        if (!s_moveSound) {
            SDL_Log("Effects::loadSounds: failed to load move sound '%s': %s",
                    moveSoundPath.c_str(), Mix_GetError());
        } else {
            anyLoaded = true;
        }
    }
    if (!captureSoundPath.empty()) {
        s_captureSound = Mix_LoadWAV(captureSoundPath.c_str());
        if (!s_captureSound) {
            SDL_Log("Effects::loadSounds: failed to load capture sound '%s': %s",
                    captureSoundPath.c_str(), Mix_GetError());
        } else {
            anyLoaded = true;
        }
    }
    if (!invalidSoundPath.empty()) {
        s_invalidSound = Mix_LoadWAV(invalidSoundPath.c_str());
        if (!s_invalidSound) {
            SDL_Log("Effects::loadSounds: failed to load move sound '%s': %s",
                    invalidSoundPath.c_str(), Mix_GetError());
        } else {
            anyLoaded = true;
        }
    }
    if (!checkSoundPath.empty()) {
        s_checkSound = Mix_LoadWAV(checkSoundPath.c_str());
        if (!s_checkSound) {
            SDL_Log("Effects::loadSounds: failed to load capture sound '%s': %s",
                    checkSoundPath.c_str(), Mix_GetError());
        } else {
            anyLoaded = true;
        }
    }

    return anyLoaded;
}

void EFFECTS::playMove() {
    if (!s_initialized) {
        SDL_Log("Effects::playMove called before initAudio()");
        return;
    }
    if (s_moveSound) {
        Mix_PlayChannel(-1, s_moveSound, 0);
    }
}

void EFFECTS::playCapture() {
    if (!s_initialized) {
        SDL_Log("Effects::playCapture called before initAudio()");
        return;
    }
    if (s_captureSound) {
        Mix_PlayChannel(-1, s_captureSound, 0);
    }
}

void EFFECTS::playInvalid()
{
    if (!s_initialized) {
        SDL_Log("Effects::playCapture called before initAudio()");
        return;
    }
    if (s_invalidSound) {
        Mix_PlayChannel(-1, s_invalidSound, 0);
    }
}

void EFFECTS::playCheck()
{
    if (!s_checkSound) {
        SDL_Log("Effects::playCapture called before initAudio()");
        return;
    }
    if (s_checkSound) {
        Mix_PlayChannel(-1, s_checkSound, 0);
    }
}

void EFFECTS::cleanup() {
    if (s_moveSound) {
        Mix_FreeChunk(s_moveSound);
        s_moveSound = nullptr;
    }
    if (s_captureSound) {
        Mix_FreeChunk(s_captureSound);
        s_captureSound = nullptr;
    }
    if (s_invalidSound) {
        Mix_FreeChunk(s_invalidSound);
        s_invalidSound = nullptr;
    }
    if (s_checkSound) {
        Mix_FreeChunk(s_checkSound);
        s_checkSound = nullptr;
    }
    if (s_initialized) {
        Mix_CloseAudio();
        Mix_Quit();
        s_initialized = false;
    }
}