#include <string>
#include <iostream>
#include <ctime>
#include "sound.h"
using std::string;

SDLSound::~SDLSound() {
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    for (unsigned int i = 0; i < sounds.size(); ++i) {
        Mix_FreeChunk(sounds[i]);
    }
    Mix_CloseAudio();
}

SDLSound::SDLSound() :
    volume(SDL_MIX_MAXVOLUME/4),
    currentSound(-1),
    music(NULL),
    audioRate(22050),
    audioChannels(2),
    audioBuffers(1024),
    sounds(),
    channels()
{

    if(Mix_OpenAudio(audioRate, MIX_DEFAULT_FORMAT, audioChannels,
                    audioBuffers)){
                        throw string("Unable to open audio!");
                    }
    music = Mix_LoadMUS("sound/theme.wav");
    // Need to install midi to play the following:
    // music = Mix_LoadMUS("sound/ballad2.mid");
    if (!music) throw string("Couldn't load background music!")+Mix_GetError();

    startMusic();

    sounds.push_back( Mix_LoadWAV("sound/primary.wav") );
    sounds.push_back( Mix_LoadWAV("sound/secondary2.wav") );
    sounds.push_back( Mix_LoadWAV("sound/destroyed.wav") );
    sounds.push_back( Mix_LoadWAV("sound/hit.wav") );
    sounds.push_back( Mix_LoadWAV("sound/overheat.wav") );
    sounds.push_back( Mix_LoadWAV("sound/cooldown.wav") );
    sounds.push_back( Mix_LoadWAV("sound/heal.wav") );
    sounds.push_back( Mix_LoadWAV("sound/playerhit.wav") );
    sounds.push_back( Mix_LoadWAV("sound/playerdestroyed.wav") );
    for (unsigned int i = 0; i < sounds.size(); ++i) channels.push_back(i);
}

void SDLSound::toggleMusic() {
    if( Mix_PausedMusic() ) {
        Mix_ResumeMusic();
    }
    else {
        Mix_PauseMusic();
    }
}

void SDLSound::operator[](int index) {
    if (currentSound == index && (currentSound == 0 || currentSound == 1) )
        Mix_HaltChannel(currentSound);
    currentSound = index;
    Mix_VolumeChunk(sounds[index], volume);
    channels[index] =
        Mix_PlayChannel(-1, sounds[index], 0);
}

void SDLSound::startMusic() {
    Mix_VolumeMusic(volume);
    Mix_PlayMusic(music, -1);
}

void SDLSound::stopMusic() {
    Mix_HaltMusic();
    Mix_FreeMusic(music);
}
