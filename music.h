#ifndef MUSIC_H
#define MUSIC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <ncurses.h>
#include <stdio.h>

int initialize_systems() {
    // SDL Initialization
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        return 1;
    }

    // SDL_Mixer Initialization
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        return 1;
    }

    // ncurses Initialization
    initscr();
    timeout(10000000);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    return 0;
}

int play_music(const char* music_file) {
    // Load the music file
    Mix_Music* music = Mix_LoadMUS(music_file);
    if (music == NULL) {
        printf("Mix_LoadMUS error: %s\n", Mix_GetError());
        return -1;
    }

    // Play the music
    if (Mix_PlayMusic(music, -1) == -1) {
        printf("Mix_PlayMusic error: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        return -1;
    }

    return 0; // Success
}


void cleanup_systems() {
    Mix_HaltMusic();
    Mix_CloseAudio();
    SDL_Quit();
    endwin();

}

#endif // MUSIC_H
