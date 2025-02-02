#ifndef GAME_MENU_H
#define GAME_MENU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <ncurses.h>
#include <locale.h>
#include "ScoreTable.h"
#include "music.h"
#include "Move.h"
#include "Map.h"

#define MAX_PROFILE_SIZE 256 

Mix_Music *current_bgm = NULL;
bool is_music_playing = false;  //flag to track whether music is currently playing


struct game {
    int level;
    int color;
};
typedef struct game Game;

int menu(WINDOW *win, int COLS, int LINES, char *choices[], int n_choices, char *title) {
    int choice = 0;

    keypad(win, TRUE);

    while (1) {
        wclear(win);
        box(win, 0, 0);
    
        mvwprintw(win, LINES / 2 - 3, COLS / 2 - strlen(title) / 2, "%s", title);

        for (int i = 0; i < n_choices; ++i) {
            if (i == choice)
                wattron(win, A_REVERSE);
            mvwprintw(win, LINES / 2 - 1 + i, COLS / 2 - strlen(choices[i]) / 2, "%s", choices[i]);
            if (i == choice)
                wattroff(win, A_REVERSE);
        }

        wrefresh(win);
        int ch = wgetch(win);
        if (ch == KEY_UP)
            choice = (choice == 0) ? n_choices - 1 : choice - 1;
        else if (ch == KEY_DOWN)
            choice = (choice == n_choices - 1) ? 0 : choice + 1;
        else if (ch == 10)  // Enter key
            break;
    }
    return choice;
}



int display_game_menu(char username[]) {
    int color_choice = 0;

    // Initialize SDL and SDL_mixer
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        printf("Mix_OpenAudio error: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    WINDOW *win = newwin(LINES, COLS, 0, 0);
    refresh();

    char *choices1[] = {"\U0001F464 Profile \U0001F464", "\U0001F9ED Start a New Game \U0001F9ED", "\U0001F4CD Continue the Last Game \U0001F4CD", "\U0001F3C6 Score's Table \U0001F3C6", " \U00002699 Settings \U00002699", "\U0001F6AA Exit \U0001F6AA"};
    char *choices2[] = {"\U0001F7E2 Easy \U0001F7E2", "\U0001F7E0 Medium \U0001F7E0", "\U0001F534 Hard \U0001F534", "\U000021A9 Back \U000021A9"};
    char *choices3[] = {"White", "Blue", "Green", "Red", "Yellow", "Magenta", "Cyan", "\U000021A9 Back \U000021A9"};
    char *choices4[] = {"\U0001F3B5 Track 1 \U0001F3B5", "\U0001F3B5 Track 2 \U0001F3B5", "\U0001F3B5 Track 3 \U0001F3B5", "\U0001F3B5 Track 4 \U0001F3B5", "\U0001F507 Turn Off \U0001F507", "\U000021A9 Back \U000021A9"};
    char *choices6[] = {"\U0001F4C8 Game Difficulty Level \U0001F4C8", "\U0001F3A8 Character Appearance \U0001F3A8", "\U0001F3A7 Music \U0001F3A7", "\U000021A9 Back \U000021A9"};
    
    char *title1 = "\U0001F3AE Game Menu \U0001F3AE";
    char *title2 = "\U0001F4C8 Game Levels \U0001F4C8";
    char *title3 = "\U0001F3A8 Character Appearance \U0001F3A8";
    char *title4 = "\U0001F3BC Music \U0001D122";
    char *title5 = "\U0001F464 Profile \U0001F464";
    char *title6 = "\U00002699 Settings \U00002699";

    Game game = {0};
    Mix_Music* current_music = NULL;
    int choice1;

    do {
        // Main menu
        choice1 = menu(win, COLS, LINES, choices1, 6, title1);

        switch (choice1) {
            case 0:  // Profile
            {
                int choice0;
                char *choices5[4];  // Array of pointers to strings
                char choice_strings[4][256];  // Buffers to hold the choice strings
                char profile_path[512];
                snprintf(profile_path, sizeof(profile_path), "/home/selma/Desktop/Project/Phase1/Users' Info/user_data.txt");
                FILE *profile_file = fopen(profile_path, "r");

                if (profile_file) {
                    char line[512];
                    while (fgets(line, sizeof(line), profile_file)) {
                        char file_username[100], email[100], password[100];
                        sscanf(line, "%s %s %s", file_username, email, password);

                        if (strcmp(username, file_username) == 0) {
                            snprintf(choice_strings[0], sizeof(choice_strings[0]), "Username: %s", file_username);
                            snprintf(choice_strings[1], sizeof(choice_strings[1]), "Email Address: %s", email);
                            snprintf(choice_strings[2], sizeof(choice_strings[2]), "Password: %s", password);
                            snprintf(choice_strings[3], sizeof(choice_strings[3]), "Back");

                            for (int i = 0; i < 4; i++) {
                                choices5[i] = choice_strings[i];
                            }
                            break;
                        }
                    }
                    fclose(profile_file);
                }

                do {
                    choice0 = menu(win, COLS, LINES, choices5, 4, "Profile");
                } while (choice0 != 3);
                break;
            }

            case 1:  // Start a New Game
                clear();
                mvwprintw(win, LINES / 2, (COLS - 25) / 2, "Starting a New Game...");
                refresh();
                getch();
                map_generator(game.level);
                start_game(game.color, username, 0, game.level);
                break;

            case 2:  // Continue the Last Game
                clear();
                mvwprintw(win, LINES / 2, (COLS - 25) / 2, "Loading the Last Game...");
                refresh();
                getch();
                start_game(game.color, username, 1, game.level);
                break;

            case 3:// Scores Table
                {
                    const char *file_path = "/home/selma/Desktop/Project/Phase1/Users' Info/user_data.txt";
                    const char *highlighted_username = username;
                    display_score_table(file_path, highlighted_username);
                }
                break;

            case 4:  // Settings
                {
                    int choice4;
                    do {
                        choice4 = menu(win, COLS, LINES, choices6, 4, title6);
                        if (choice4 == 3) break;  // If "Back" is selected, return to the main menu.

                        switch (choice4) {
                            case 0: // Game Difficulty Level
                                {
                                    int difficulty;
                                    do {
                                        difficulty = menu(win, COLS, LINES, choices2, 4, title2);
                                        if (difficulty == 3) break;  // "Back" in difficulty
                                        switch (difficulty) {
                                            case 0: 
                                                game.level = 3; // easy
                                                break;
                                            case 1:
                                                game.level = 2; // medium
                                                break;
                                            case 2:
                                                game.level = 1; // hard
                                                break;
                                        }
                                    } while (difficulty != 3); // back to settings
                                }
                                break;
                            case 1: // Character Appearance (Color)
                                {
                                    do {
                                        color_choice = menu(win, COLS, LINES, choices3, 8, title3);
                                        if (color_choice == 7) break; // "Back" in appearance menu
                                        switch (color_choice) {
                                            case 0: game.color = 0;
                                                break;
                                            case 1: game.color = 1;
                                                break;
                                            case 2: game.color = 2;
                                                break;
                                            case 3: game.color = 3;
                                                break;
                                            case 4: game.color = 4;
                                                break;
                                            case 5: game.color = 5;
                                                break;
                                            case 6: game.color = 6;
                                                break;
                                        }
                                    } while (color_choice != 7); // back to settings
                                } 
                                break;
                            case 2:  // Music
                            {
                                int music_choice;
                                do {
                                    music_choice = menu(win, COLS, LINES, choices4, 6, title4);
                                    if (music_choice == 5) break;  // Exit music menu (back)
                                    char* music_file = NULL;  // Initialize to NULL

                                    switch (music_choice) {
                                        case 0:
                                            music_file = "alexander-nakarada-chase(chosic.com).mp3";
                                            break;
                                        case 1:
                                            music_file = "Distant-Sky-Epic-Hybrid-Music-chosic.com_.mp3";
                                            break;
                                        case 2:
                                            music_file = "Hitman(chosic.com).mp3";
                                            break;
                                        case 3:
                                            music_file = "Russell Velázquez.TMNT (2003) Main Theme.mp3";
                                            break;
                                        case 4:
                                            // Turn off music
                                            music_file = NULL;
                                            break;
                                    }

                                    if (music_choice == 4) {
                                        if (current_music != NULL) {
                                            Mix_HaltMusic();  // Stop current music
                                            Mix_FreeMusic(current_music);  // Free current music
                                            current_music = NULL;
                                            printf("Music turned off\n");  // Debugging info
                                        }
                                    } else {
                                        if (music_file) {
                                            if (current_music != NULL) {
                                                Mix_HaltMusic();  // Stop current music
                                                Mix_FreeMusic(current_music);  // Free current music
                                            }
                                            current_music = Mix_LoadMUS(music_file);
                                            if (current_music == NULL) {
                                                printf("Mix_LoadMUS error: %s\n", Mix_GetError());
                                            } else {
                                                if (Mix_PlayMusic(current_music, -1) == -1) {
                                                    printf("Mix_PlayMusic error: %s\n", Mix_GetError());
                                                }
                                            }
                                        }
                                    }
                                } while (music_choice != 5);  // back to settings
                            }
                            break;

                        }
                    } while (choice4 != 3);  // Return to main menu
                }
                break;

            case 5:  // Exit Program
                clear();
                mvwprintw(win, LINES / 2, (COLS - 25) / 2, "Exiting Program...");
                refresh();
                getch();
                choice1 = 5;
                break;
        }
    } while (choice1 != 5);


    if (current_music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(current_music);
    }
    Mix_CloseAudio();
    SDL_Quit();
    endwin();
}


#endif