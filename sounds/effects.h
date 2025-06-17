// #pragma once

// #include <SDL2/SDL_mixer.h>
// #include <string>

// /// Effects: manages loading and playing short sound effects (e.g., move, capture).
// /// Usage:
// ///   - After SDL and SDL_mixer are initialized (or via Effects::initAudio), call Effects::loadSounds(...).
// ///   - When a move happens: Effects::playMove();
// ///   - When a capture happens: Effects::playCapture();
// ///   - On shutdown: Effects::cleanup().
// class EFFECTS {
// public:
//     /// Initialize SDL_mixer audio subsystem.
//     /// Must be called after SDL_Init(SDL_INIT_AUDIO) (or include SDL_INIT_AUDIO in SDL_Init flags).
//     /// Returns true on success, false on failure.
//     static bool initAudio(int frequency = 44100, Uint16 format = MIX_DEFAULT_FORMAT,
//                           int channels = 2, int chunksize = 2048);

//     /// Load the move and capture sound files.
//     /// Provide file paths to your WAV/OGG/etc. If a path is empty or loading fails, the corresponding effect will be null.
//     /// Returns true if at least one sound loaded successfully; false if both failed.
//     static bool loadSounds(const std::string& moveSoundPath,
//                            const std::string& captureSoundPath,
//                            const std::string& invalidSoundPath,
//                            const std::string& checkSoundPath,
//                            const std::string& loopPath);

//     /// Play the “move” sound effect once (on first free channel). If not loaded, does nothing.
//     static void playMove();

//     /// Play the “capture” sound effect once. If not loaded, does nothing.
//     static void playCapture();

//     static void playInvalid();

//     static void playCheck();

//     static void playBackground(int loops = -1);

//     static void stopBackground();

//     static void setBackgroundVolumePercent(int percent);

//     static void setEffectsVolumePercent(int percent);

//     /// Frees loaded Mix_Chunk and closes audio.
//     static void cleanup();

// private:
//     // Prevent instantiation
//     EFFECTS() = delete;
//     ~EFFECTS() = delete;

//     static Mix_Chunk* s_moveSound;
//     static Mix_Chunk* s_captureSound;
//     static Mix_Chunk* s_invalidSound;
//     static Mix_Chunk* s_checkSound;
//     static Mix_Music* s_bgMusic;
//     static bool s_initialized;
// };