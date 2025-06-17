// #include "effects.h"
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_log.h>

// Mix_Chunk* EFFECTS::s_moveSound = nullptr;
// Mix_Chunk* EFFECTS::s_invalidSound = nullptr;
// Mix_Chunk* EFFECTS::s_captureSound = nullptr;
// Mix_Chunk* EFFECTS::s_checkSound = nullptr;
// Mix_Music* EFFECTS::s_bgMusic = nullptr;
// bool EFFECTS::s_initialized = false;

// bool EFFECTS::initAudio(int frequency, Uint16 format, int channels, int chunksize) {
//     if (s_initialized) {
//         return true; // already initialized
//     }
//     // Initialize SDL_mixer; assume SDL_Init(SDL_INIT_AUDIO) already done or include audio flag earlier
//     // If we need OGG/MP3 support, we must ensure Mix_Init is called elsewhere before this or add here:
//     if (!(Mix_Init(MIX_INIT_OGG) & MIX_INIT_OGG)) { SDL_Log("Mix_Init OGG failed: %s", Mix_GetError()); }
//     if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
//         SDL_Log("Effects::initAudio: Mix_OpenAudio failed: %s", Mix_GetError());
//         return false;
//     }
//     // Optionally increase number of channels for simultaneous sounds:
//     Mix_AllocateChannels(16);
//     s_initialized = true;
//     return true;
// }

// bool EFFECTS::loadSounds(   const std::string& moveSoundPath,
//                             const std::string& captureSoundPath,
//                             const std::string& invalidSoundPath,
//                             const std::string& checkSoundPath,
//                             const std::string& bgmPath)
// {
//     bool anyLoaded = false;

//     // Free previous if any
//     if (s_moveSound) {
//         Mix_FreeChunk(s_moveSound);
//         s_moveSound = nullptr;
//     }
//     if (s_captureSound) {
//         Mix_FreeChunk(s_captureSound);
//         s_captureSound = nullptr;
//     }
//     if (s_invalidSound) {
//         Mix_FreeChunk(s_invalidSound);
//         s_invalidSound = nullptr;
//     }
//     if (s_checkSound) {
//         Mix_FreeChunk(s_checkSound);
//         s_checkSound = nullptr;
//     }
//     if (s_bgMusic) {
//             Mix_FreeMusic(s_bgMusic);
//             s_bgMusic = nullptr;
//     }

//     if (!moveSoundPath.empty()) {
//         s_moveSound = Mix_LoadWAV(moveSoundPath.c_str());
//         if (!s_moveSound) {
//             SDL_Log("Effects::loadSounds: failed to load move sound '%s': %s",
//                     moveSoundPath.c_str(), Mix_GetError());
//         } else {
//             anyLoaded = true;
//         }
//     }
//     if (!captureSoundPath.empty()) {
//         s_captureSound = Mix_LoadWAV(captureSoundPath.c_str());
//         if (!s_captureSound) {
//             SDL_Log("Effects::loadSounds: failed to load capture sound '%s': %s",
//                     captureSoundPath.c_str(), Mix_GetError());
//         } else {
//             anyLoaded = true;
//         }
//     }
//     if (!invalidSoundPath.empty()) {
//         s_invalidSound = Mix_LoadWAV(invalidSoundPath.c_str());
//         if (!s_invalidSound) {
//             SDL_Log("Effects::loadSounds: failed to load move sound '%s': %s",
//                     invalidSoundPath.c_str(), Mix_GetError());
//         } else {
//             anyLoaded = true;
//         }
//     }
//     if (!checkSoundPath.empty()) {
//         s_checkSound = Mix_LoadWAV(checkSoundPath.c_str());
//         if (!s_checkSound) {
//             SDL_Log("Effects::loadSounds: failed to load capture sound '%s': %s",
//                     checkSoundPath.c_str(), Mix_GetError());
//         } else {
//             anyLoaded = true;
//         }
//     }
//     s_bgMusic = Mix_LoadMUS(bgmPath.c_str());
//     if (!s_bgMusic) {
//         SDL_Log("Effects::loadBackgroundMusic failed: %s", Mix_GetError());
//     } else {
//         anyLoaded = true;
//     }

//     return anyLoaded;
// }

// void EFFECTS::playMove() {
//     if (!s_initialized) {
//         SDL_Log("Effects::playMove called before initAudio()");
//         return;
//     }
//     if (s_moveSound) {
//         Mix_PlayChannel(-1, s_moveSound, 0);
//     }
// }

// void EFFECTS::playCapture() {
//     if (!s_initialized) {
//         SDL_Log("Effects::playCapture called before initAudio()");
//         return;
//     }
//     if (s_captureSound) {
//         Mix_PlayChannel(-1, s_captureSound, 0);
//     }
// }

// void EFFECTS::playInvalid()
// {
//     if (!s_initialized) {
//         SDL_Log("Effects::playCapture called before initAudio()");
//         return;
//     }
//     if (s_invalidSound) {
//         Mix_PlayChannel(-1, s_invalidSound, 0);
//     }
// }

// void EFFECTS::playCheck()
// {
//     if (!s_checkSound) {
//         SDL_Log("Effects::playCapture called before initAudio()");
//         return;
//     }
//     if (s_checkSound) {
//         Mix_PlayChannel(-1, s_checkSound, 0);
//     }
// }

// void EFFECTS::playBackground(int loops)
// {
//     if (!s_initialized) {
//         SDL_Log("Effects::playBackground called before initAudio()");
//         return;
//     }
//     if (s_bgMusic) {
//         if (Mix_PlayMusic(s_bgMusic, loops) < 0) {
//             SDL_Log("Mix_PlayMusic failed: %s", Mix_GetError());
//         }
//     }
// }

// void EFFECTS::stopBackground() { Mix_HaltMusic(); }

// void EFFECTS::setBackgroundVolumePercent(int percent)
// {
//     if (!s_initialized) return;
//     int vol = (percent * MIX_MAX_VOLUME) / 100;
//     Mix_VolumeMusic(vol);
// }

// void EFFECTS::setEffectsVolumePercent(int percent)
// {
//     if (!s_initialized) return;
//     int vol = (percent * MIX_MAX_VOLUME) / 100;
//     if (s_moveSound)    Mix_VolumeChunk(s_moveSound, vol);
//     if (s_captureSound) Mix_VolumeChunk(s_captureSound, vol);
//     if (s_invalidSound) Mix_VolumeChunk(s_invalidSound, vol);
//     if (s_checkSound)   Mix_VolumeChunk(s_checkSound, vol);
// }

// void EFFECTS::cleanup() {
//     if (s_bgMusic) {
//         Mix_HaltMusic();
//         Mix_FreeMusic(s_bgMusic);
//         s_bgMusic = nullptr;
//     }
//     // free chunks...
//     if (s_moveSound) { Mix_FreeChunk(s_moveSound); s_moveSound = nullptr; }
//     if (s_captureSound) { Mix_FreeChunk(s_captureSound); s_captureSound = nullptr; }
//     if (s_invalidSound) { Mix_FreeChunk(s_invalidSound); s_invalidSound = nullptr; }
//     if (s_checkSound) { Mix_FreeChunk(s_checkSound); s_checkSound = nullptr; }
//     if (s_initialized) {
//         Mix_CloseAudio();
//         Mix_Quit();
//         s_initialized = false;
//     }
// }