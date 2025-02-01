#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <locale.h>
#include <wchar.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "music.h"


#define MAX_TRAPS 100
#define MAX_ROOMS 15
#define MAX_HUNGER 30
#define TIME_INTERVAL 2
#define HUNGER_RATE 30
#define LIFE 100
#define MAX_GOLDS_IN_TREASURE_ROOM 50
#define MAX_TRAPS_IN_TREASURE_ROOM 50
#define MAX_DEMONS_IN_TREASURE_ROOM 10
#define BORDER_X 30
#define BORDER_Y 70
#define FOOD_CONVERSION_RATE 3
#define COLOR_PURPLE 70
#define COLOR_DARK_PINK 60
#define COLOR_ORANGE 80
#define COLOR_GREEN_PASS 90
#define COLOR_GOLD 100
#define NAME_WIDTH 30

Mix_Music *current_bgm = NULL;
bool is_music_playing = false; 


char temp_map[50][184];

typedef struct {
    int food;
    int gold;
    //weapons
    int mace, dagger, magic_wand, normal_arrow, sword;
    int weapon_type;  //1: Mace, 2: Dagger, 3: Magic Wand, 4: Normal Arrow, 5: Sword

    int life; //Joon
    int velocity;
    int weapon_damage;

    //spells
    int health, damage, speed;

    int masterkey, password_door;

    char username[10];
    char password[50];
    char email[100];
    int score;
    int number_of_finished_games;
    int experience;
    int rank;
    char special_name[50];
} user;


typedef struct {
    int damage;
    int distance;
} weapon;

const char *food_names[] = {"Normal Food", "Superior Food", "Magical Food"};
int food_counts[3];
int time_food[2]; //special foods will be converted to normal food

const char *spell_names[] = {"Health", "Speed", "Damage"};
int spell_count[] = {0, 0, 0};
int spell_life[] = {2, 1, 1};
int spell_speed[] = {1, 2, 1};
int spell_damage[] = {1, 1, 2};

const char *music_files[4] = {
    "alexander-nakarada-chase(chosic.com).mp3",
    "Distant-Sky-Epic-Hybrid-Music-chosic.com_.mp3",
    "Hitman(chosic.com).mp3",
    "Russell Velázquez.TMNT (2003) Main Theme.mp3"
};

Mix_Music *current_music;


typedef struct {
    chtype character;
    int row;    
    int col; 
} ScreenChar;


typedef struct {
    int x, y;
} Point;

Point player;

typedef struct {
    int room_number;
    Point center;
    Point dimensions; // width x height
    int type;
    Point doors[10]; // Assuming max 10 doors for simplicity
    int num_doors;
    Point pillars[10]; // Assuming max 10 pillars
    int num_pillars;
    Point windows[10]; // Assuming max 10 windows
    int num_windows;
    int visibility;
    struct {
        int type;
        int life;
        Point position;
        int damage;
    } demon;
} Room;

typedef struct {
    Point position;
    int life;
    char type;
} DEMON;


int get_random_number(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void load_floor(char next_map[LINES][COLS], int current_floor) {
    char filename[20];
    char map[LINES][COLS];
    char temp_map[LINES][COLS];
    sprintf(filename, "map%d.txt", current_floor + 1);
    FILE *new_file = fopen(filename, "r");

    for (int i = 0; i < LINES; i++) { // Start from the 2nd line, to allocate space at top for game messages
        for (int j = 0; j < COLS; j++) {
            int ch = fgetc(new_file);
            if (ch == EOF) {
                map[i][j] = ' ';
                temp_map[i][j] = ' ';
            } else {
                map[i][j] = (char) ch;
                temp_map[i][j] = (char) ch;
            }
        }
    }

    fclose(new_file);

    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            mvaddch(i + 2, j, map[i + 2][j]);
        }
    }

    // Randomly set the player's position on the new floor
    int valid = 0;
    int new_x, new_y;
    while (!valid) {
        new_x = get_random_number(1, COLS - 1);
        new_y = get_random_number(1, LINES - 1);
        if (map[new_y][new_x] != ' ' && map[new_y][new_x] != '_' && map[new_y][new_x] != '|' && map[new_y][new_x] != 'O') {
            valid = 1;
        }
    }

    mvprintw(new_y, new_x, "@");  // Place the player on the new floor
    temp_map[new_y][new_x] = map[new_y][new_x];  // Update the temporary map
    refresh();
}

void reverse_password(char *password) {
    for (int i = 0; i < 2; i++) {
        char temp = password[i];
        password[i] = password[3 - i];
        password[3 - i] = temp;
    }
}

void int_to_string(int number, char *password) {
    snprintf(password, 5, "%04d", number);
}

int pass_screen(int password) {
    int attempts = 3;
    int entered_pass = 0;
    char input[5];
    int color_pair;

    char password_str[5];
    int_to_string(password, password_str);

    initscr();
    noecho();
    cbreak();
    start_color();

    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);

    int is_reverse = get_random_number(0, 1);
    if (is_reverse) {
        reverse_password(password_str);
    }
    
    while (attempts > 0) {
        clear();
        mvprintw(LINES / 2, (COLS - 25) / 2, "Enter 4-digit password: ");
        refresh();

        echo();
        mvgetnstr(LINES / 2, (COLS + 25) / 2, input, 4);
        noecho();
        input[4] = '\0';

        if (strcmp(input, password_str) == 0) {
            mvprintw(LINES / 2 + 1, (COLS - 25) / 2, "Password correct!");
            refresh();
            sleep(2);
            endwin();
            return 1;
        } else {
            attempts--;
            if (attempts == 2) {
                color_pair = 1; // Yellow
            } else if (attempts == 1) {
                color_pair = 3; // Magenta
            } else {
                color_pair = 2; // Red
            }

            attron(COLOR_PAIR(color_pair));
            if (attempts > 0) {
                mvprintw(LINES / 2 + 1, (COLS - 25) / 2, "Incorrect password. Try again. Attempts left: %d", attempts);
            } else {
                mvprintw(LINES / 2 + 1, (COLS - 25) / 2, "Incorrect password. No attempts left.");
                attroff(COLOR_PAIR(color_pair));
                refresh();
                sleep(2);
                endwin();
                return 0;
            }
            attroff(COLOR_PAIR(color_pair));
            refresh();
            sleep(2);
        }
    }

    endwin();
    return 0;
}

void save_screen(ScreenChar ***screenData, int *rows, int *cols) {
    *screenData = (ScreenChar **)malloc(*rows * sizeof(ScreenChar *));
    for (int i = 0; i < *rows; i++) {
        (*screenData)[i] = (ScreenChar *)malloc(*cols * sizeof(ScreenChar));
    }

    // Capture the entire screen data
    for (int y = 0; y < *rows; y++) {
        for (int x = 0; x < *cols; x++) {
            (*screenData)[y][x].character = mvinch(y, x);
            (*screenData)[y][x].row = y;
            (*screenData)[y][x].col = x;
        }
    }
}

void display_screen(ScreenChar **screenData, int rows, int cols) {
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            mvprintw(y, x, "%c", screenData[y][x].character & A_CHARTEXT);
        }
    }
}

void display_password(int pass, time_t start_time) {
    start_color();
    init_pair(3, COLOR_MAGENTA, COLOR_WHITE);
    attron(COLOR_PAIR(3));
    mvprintw(0, COLS / 2, "Password: %d", pass);
    attroff(COLOR_PAIR(3));
    refresh();
}

void clear_password_display() {
    mvprintw(0, COLS / 2, "              "); // Clear password display
    refresh();
}

void color (char map[LINES][COLS], char pass_doors_locked[LINES][COLS]) {
    start_color();

    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_PURPLE, 600, 0, 600);
    init_color(COLOR_ORANGE, 900, 500, 0);
    init_color(COLOR_DARK_PINK, 600, 0, 450);
    init_color(COLOR_GREEN_PASS, 124, 252, 0);
    init_color(COLOR_GOLD, 800, 600, 200);

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN_PASS, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_DARK_PINK, COLOR_BLACK);
    init_pair(7, COLOR_PURPLE, COLOR_BLACK);
    init_pair(8, COLOR_GOLD, COLOR_BLACK);

    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            if (map[i][j] == '@') {
                if (pass_doors_locked[i][j] == 0) {
                    attron(COLOR_PAIR(2));
                    mvprintw(i, j, "%c", map[i][j]);
                    attroff(COLOR_PAIR(2));
                } else {
                    attron(COLOR_PAIR(1));
                    mvprintw(i, j, "%c", map[i][j]);
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[i][j] == 'g' || map[i][j] == '$' || map[i][j] == '*') {
                    attron(COLOR_PAIR(3));
                    mvprintw(i, j, "%c", map[i][j]);
                    attroff(COLOR_PAIR(3));
            } else if (map[i][j] == '&') {
                    attron(A_BOLD);
                    mvprintw(i, j, "%c", map[i][j]);
                    attroff(A_BOLD);                       
            } else if (map[i][j] == 'f') {
                    attron(COLOR_PAIR(5));
                    mvprintw(i, j, "%c", map[i][j]);
                    attroff(COLOR_PAIR(5));
            } else if (map[i][j] == '!' || map[i][j] == '\\' || map[i][j] == '(' || map[i][j] == ')') {
                attron(COLOR_PAIR(6));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(6));
            } else if (map[i][j] == 'h' || map[i][j] == 'd' || map[i][j] == 's') {
                attron(COLOR_PAIR(7));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(7));
            } else if (map[i][j] == 'K') {
                attron(COLOR_PAIR(8));
                mvprintw(i, j, "%c", map[i][j]);
                attroff(COLOR_PAIR(8));
            }
            else {
                mvprintw(i, j, "%c", map[i][j]);
            }
        }
    }
    refresh();
}

int find_room(int x, int y, char map[LINES][COLS], int num_rooms, Room *rooms) {
    for (int i = 0; i < num_rooms; i++) {
        if (x >= rooms[i].center.y - rooms[i].dimensions.x / 2 &&
            x <= rooms[i].center.y + rooms[i].dimensions.x / 2 &&
            y >= rooms[i].center.x - rooms[i].dimensions.y / 2 &&
            y <= rooms[i].center.x + rooms[i].dimensions.y / 2) {
            return i;
        }
    }
    return -1;
}

int weapon_menu(WINDOW *win, int COLS, int LINES, weapon weapons[], int n_choices, user player) {
    int choice = 0;
    keypad(win, TRUE);

    curs_set(0);

    wclear(win);
    box(win, 0, 0);
    char title[20] = "Weapon Menu";
    attron(A_BLINK);
    mvwprintw(win, LINES / 2 - 4, COLS / 2 - strlen(title) / 2, "%s", title);
    attroff(A_BLINK);

    mvwprintw(win, LINES / 2 - 2, COLS / 2 - 30, "1. Mace            - Count: Infinite!| Damage: 5 | Distance: 1");
    mvwprintw(win, LINES / 2 - 1, COLS / 2 - 30, "2. Sword           - Count:    %d     | Damage: 10| Distance: 1", player.sword);
    mvwprintw(win, LINES / 2 + 1, COLS / 2 - 30, "3. Normal Arrow    - Count:    %d     | Damage: 5 | Distance: 5", player.normal_arrow);
    mvwprintw(win, LINES / 2 + 2, COLS / 2 - 30, "4. Dagger          - Count:    %d     | Damage: 12| Distance: 5", player.dagger);
    mvwprintw(win, LINES / 2 + 4, COLS / 2 - 30, "5. Magic Wand      - Count:    %d     | Damage: 15| Distance: 10", player.magic_wand);

    getch();
    curs_set(1);
    return 0;
}

int food_menu(WINDOW *win, int COLS, int LINES, int n_choices, user player) {
    int choice = 0;
    int highlight = 0;
    keypad(win, TRUE);
    curs_set(0);

    while (1) {
        wclear(win);
        box(win, 0, 0);
        char title[20] = "Food Menu";
        wattron(win, A_BOLD);
        mvwprintw(win, LINES / 2 - 4, COLS / 2 - 5, "%s", title);
        wattroff(win, A_BOLD);

        // Display food options
        for (int i = 0; i < n_choices; i++) {
            if (i == highlight) {
                wattron(win, A_REVERSE);
            }

            mvwprintw(win, LINES / 2 - 2 + i, COLS / 2 - 20, "%d. %-*.*s", i + 1, NAME_WIDTH, NAME_WIDTH, food_names[i]);
            mvwprintw(win, LINES / 2 - 2 + i, COLS / 2 + 10, "count: %3d |", food_counts[i]);

            if (i == highlight) {
                wattroff(win, A_REVERSE);
            }
        }

        int c = wgetch(win);
        switch (c) {
            case KEY_UP:
                if (highlight > 0) {
                    highlight--;
                }
                break;
            case KEY_DOWN:
                if (highlight < n_choices - 1) {
                    highlight++;
                }
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }

        if (c == 10) {
            break;
        }

        wrefresh(win);
    }

    curs_set(1);
    return choice;
}

int spell_menu(WINDOW *win, int COLS, int LINES, int n_choices, user player) {
    int choice = 0;
    int highlight = 0;
    keypad(win, TRUE);
    curs_set(0);

    while (1) {
        // Clear window and draw box
        wclear(win);
        box(win, 0, 0);
        char title[20] = "Spell Menu";
        wattron(win, A_BOLD);
        mvwprintw(win, LINES / 2 - 4, COLS / 2 - 5, "%s", title);
        wattroff(win, A_BOLD);

        
        for (int i = 0; i < n_choices; i++) {
            if (i == highlight) {
                wattron(win, A_REVERSE);
            }

            mvwprintw(win, LINES / 2 - 2 + i, COLS / 2 - 15, "%d. %10s     - Count: %2d", i + 1, spell_names[i], spell_count[i]);
            wattroff(win, A_REVERSE);
        }

        int c = wgetch(win);
        switch (c) {
            case KEY_UP:
                if (highlight > 0) {
                    highlight--;
                }
                break;
            case KEY_DOWN:
                if (highlight < n_choices - 1) {
                    highlight++;
                }
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }

        if (c == 10) {
            break;
        }

        wrefresh(win);
    }
    
    curs_set(1);
    return choice;
}

void demon_room(char treasure_room_map[LINES][COLS], int x_ply, int y_ply, user *player, DEMON *demon, int num_demons, int *lost) {
    int distance_limit = 10;

    // Move demons that are within 10 cells from the player
    for (int n = 0; n < num_demons; n++) {
        int distance = sqrt(pow(demon[n].position.y - x_ply, 2) + pow(demon[n].position.x - y_ply, 2));

        if (distance <= distance_limit) {
            // Print space at demon's previous position
            treasure_room_map[demon[n].position.y][demon[n].position.x] = ' ';
            mvprintw(demon[n].position.x, demon[n].position.y, " ");

            //Move the demon closer to the player
            if (demon[n].position.y < y_ply) {
                demon[n].position.x++;
            } else if (demon[n].position.x > y_ply) {
                demon[n].position.x--;
            }

            if (demon[n].position.y < x_ply) {
                demon[n].position.y++;
            } else if (demon[n].position.y > x_ply) {
                demon[n].position.y--;
            }

            // Update the demon's new position on the map
            treasure_room_map[demon[n].position.y][demon[n].position.x] = demon[n].type;
            mvprintw(demon[n].position.x, demon[n].position.y, "%c", demon[n].type);

            // Check if the player is within attack range
            if (distance <= 2) {
                player->life -= 5;
                if (player->life <= 0) {
                    *lost = 1;
                }
            }
        }
    }
    refresh();
}

void move_player2(char ch, int *x, int *y, char map[LINES][COLS], user *player, char fallen_weapons[LINES][COLS], int *lost) {
    int new_x = *x, new_y = *y;

    if (player->velocity == 2) {
        switch (ch) {
            case 'Y': // NW
            case 'y':
                new_x -= 2;
                new_y -= 2;
                break;
            case 'B': // SW
            case 'b':
                new_x -= 2;
                new_y += 2;
                break;
            case 'J': // N
            case 'j':
                new_y -= 2;
                break;
            case 'H': // W
            case 'h':
                new_x -= 2;
                break;
            case 'L': // E
            case 'l':
                new_x += 2;
                break;
            case 'K': // S
            case 'k':
                new_y += 2;
                break;
            case 'U': // NE
            case 'u':
                new_x += 2;
                new_y -= 2;
                break;
            case 'N': // SE
            case 'n':
                new_x += 2;
                new_y += 2;
                break;
        }
    } else {
        switch (ch) {
            case 'Y': // NW
            case 'y':
                new_x--;
                new_y--;
                break;
            case 'B': // SW
            case 'b':
                new_x--;
                new_y++;
                break;
            case 'J': // N
            case 'j':
                new_y--;
                break;
            case 'H': // W
            case 'h':
                new_x--;
                break;
            case 'L': // E
            case 'l':
                new_x++;
                break;
            case 'K': // S
            case 'k':
                new_y++;
                break;
            case 'U': // NE
            case 'u':
                new_x++;
                new_y--;
                break;
            case 'N': // SE
            case 'n':
                new_x++;
                new_y++;
                break;
        }
    }

    // Check if the new position is within bounds and valid (not a wall, etc.)

    if (map[new_y][new_x] != '|' && map[new_y][new_x] != '_' && map[new_y][new_x]) {
        // Regular movement
        mvprintw(*y, *x, "%c", map[*y][*x] == '@' ? '.' : map[*y][*x]);  // Update previous position
        map[*y][*x] = map[*y][*x] == '@' ? '.' : map[*y][*x];  // Reset old position

        *x = new_x;
        *y = new_y;

        mvprintw(*y, *x, "@");  // Print player in new position
        char ch2;
        if (map[new_y][new_x] == '$') {
            ch2 = getch();
            if (ch2 == 'y') {
                player->gold++;
                map[new_y][new_x] = ' ';
                attron(COLOR_PAIR(2));
                move(1, 0);
                clrtoeol();
                mvprintw(0, 5, "%-30s", "You picked up a golden coin!");
                refresh();
                attroff(COLOR_PAIR(1));
            }
        } else if (map[new_y][new_x] == '^') {
            player->life--;
            if (player->life <= 0) {
                *lost = 1;
            }
        }
        
    }
}

void display_hunger_level(int hunger) {
    curs_set(0);
    char bar[MAX_HUNGER + 1];
    for (int i = 0; i < MAX_HUNGER; i++) {
        if (i < hunger) {
            bar[i] = '#';
        } else {
            bar[i] = ' ';
        }
    }
    bar[MAX_HUNGER] = '\0';
    attron(A_REVERSE);
    mvprintw(LINES - 1, COLS - 50, "Hunger Level: [%s]", bar);
    attroff(A_REVERSE);
    refresh();
}

void handle_short_range_action(int x, int y, int *weapon_count, int damage, DEMON *demons, int num_demons, int type, int spell_effect) {
    bool demon_nearby = false;
    for (int i = 0; i < num_demons; i++) {
        int distance = sqrt(pow(demons[i].position.y - x, 2) + pow(demons[i].position.x - y, 2));

        if (distance <= 1) {
            if (type == 1) {
                (*weapon_count)--;
            }
            if (spell_effect) {
                demons[i].life -= (damage * 2);
            } else {
                demons[i].life -= damage;
            }
            attron(COLOR_RED && A_BOLD);
            mvprintw(LINES - 1, COLS - 100,"Demon hit: Type=%c, Life=%d", demons[i].type, demons[i].life);
            attroff(COLOR_RED && A_BOLD);
            refresh();
        } else {
            
            refresh();
        }

        if (demons[i].life <= 0) {
        mvprintw(0, COLS / 2, "Demon killed!");
        demons[i].position.x = 0;
        demons[i].position.y = 0;
        demons[i].life = 0;
        }
        if (demon_nearby) break;
    }
    
    if (!demon_nearby) {
        (*weapon_count)--;
    }
}

void handle_long_range_action(int x, int y, char map[LINES][COLS], DEMON *demons, int num_demons, user *player, int damage, int range, char weapon, char prev_dir[2], int repeat, char fallen_weapons[LINES][COLS], int is_magic_wand, int spell_effect) {
    int dx = 0, dy = 0;
    int demon_hit = 0;
    int *weapon_count_ptr;
    start_color();
    init_pair(100, COLOR_BLACK, COLOR_RED);

    // Direction input
    if (!repeat) {
        int dir = getch();
        switch (dir) {
            case 'y': dx = -1; dy = -1; break;
            case 'u': dx = 1; dy = -1; break;
            case 'h': dx = -1; dy = 0; break;
            case 'j': dx = 0; dy = 1; break;
            case 'k': dx = 0; dy = -1; break;
            case 'l': dx = 1; dy = 0; break;
            case 'b': dx = -1; dy = 1; break;
            case 'n': dx = 1; dy = 1; break;
            default: return;
        }
        prev_dir[0] = dx;
        prev_dir[1] = dy;
    } else {
        dx = prev_dir[0];
        dy = prev_dir[1];
    }

    // Weapon decrement logic
    switch (player->weapon_type) {
        case 3: player->normal_arrow--; weapon_count_ptr = &player->normal_arrow; break;
        case 4: player->dagger--; weapon_count_ptr = &player->dagger; break;
        case 5: player->magic_wand--; weapon_count_ptr = &player->magic_wand; is_magic_wand = 1; break;
        default: return;
    }
    int nx, ny;
    // Projectile movement
    for (int step = 1; step <= range; step++) {
        nx = x + dx * step;
        ny = y + dy * step;

        // Stop at walls
        if (map[ny][nx] == '_' || map[ny][nx] == '|' || map[ny][nx] == '=' || map[ny][nx] == '+') {
            nx = x + dx * (step - 1);
            ny = y + dy * (step - 1);
            map[ny][nx] = weapon;
            mvprintw(ny, nx, "%c", map[ny][nx]);
            break;
        }

        // Check if the projectile hits a demon
        for (int i = 0; i < num_demons; i++) {
            if (demons[i].position.y == nx && demons[i].position.x == ny) {
                if (spell_effect) {
                    demons[i].life -= (damage * 2); // Spell effect doubles damage
                } else {
                    demons[i].life -= damage;
                }
                if (is_magic_wand) {
                    demons[i].life = 0; // Magic wand instant kill
                }
                demon_hit = 1;

                // Display message for successful hit
                attron(A_BLINK | A_BOLD);
                mvprintw(0, COLS / 2 - 30, "%40s", "Demon successfully hit! Great Job!");
                attroff(A_BLINK | A_BOLD);

                if (demons[i].life <= 0) {
                    demons[i].position.x = 0;
                    demons[i].position.y = 0;
                    demons[i].life = 0; // Demon removed from play
                }

                attron(COLOR_PAIR(100));
                mvprintw(LINES - 1, COLS - 100,"Demon hit: Type=%c, Life=%2d", demons[i].type, demons[i].life);
                attroff(COLOR_PAIR(100));
                refresh();
                return; // Stop checking further
            }
        }
    }
    mvprintw(ny, nx, "%c", weapon);
    refresh();
}

int win_check(char treasure_room_map[LINES][COLS]) {
    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            if (!isspace(treasure_room_map[i][j]) && treasure_room_map[i][j] != '^' && treasure_room_map[i][j] != '|' && treasure_room_map[i][j] != '_') {
                return 0;
            }
        }
    }
    return 1;
}

void treasure_room(char treasure_room_map[LINES][COLS], user *player, weapon *wpn, char username[6]) {
    initscr();
    keypad(stdscr, TRUE); // Enable keypad for capturing special keys
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    clear();
    char fallen_weapons[LINES][COLS];
    int hunger_count = 0;
    int hungerLevel = MAX_HUNGER;
    int speed_effect_count  = 0;
    int damage_effect_count = 0;
    int repeat = 0;
    int win = 0;
    int lost = 0;
    char prev_dir[2];
    int life_decrease = 0;
    init_pair(200, COLOR_GREEN, COLOR_BLACK);
    init_pair(100, COLOR_RED, COLOR_BLACK);
    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            fallen_weapons[i][j] = 0;
        }
    }

    int num_demons = 20;
    DEMON *demons = (DEMON *)malloc(num_demons * sizeof(DEMON));
    memset(demons, 0, num_demons * sizeof(DEMON));
    num_demons = 0;
   
    // Print the map
    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            if (treasure_room_map[i][j] != ' ') {
                color(treasure_room_map, NULL);
                if (treasure_room_map[i][j] == 'D' || treasure_room_map[i][j] == 'S' || treasure_room_map[i][j] == 'U' || treasure_room_map[i][j] == 'G' || treasure_room_map[i][j] == 'F') {
                    demons[num_demons].position.x = i;
                    demons[num_demons].position.y = j;
                    if (treasure_room_map[i][j] == 'D') {
                        demons[num_demons].life = 5;
                        demons[num_demons].type = 'D';
                    } else if (treasure_room_map[i][j] == 'F') {
                        demons[num_demons].life = 10;
                        demons[num_demons].type = 'F';
                    } else if (treasure_room_map[i][j] == 'G') {
                        demons[num_demons].life = 15;
                        demons[num_demons].type = 'G';
                    } else if (treasure_room_map[i][j] == 'S') {
                        demons[num_demons].life = 20;
                        demons[num_demons].type = 'S';
                    } else if (treasure_room_map[i][j] == 'U') {
                        demons[num_demons].life = 30;
                        demons[num_demons].type = 'U';
                    }
                    num_demons++;
                }
            }
        }
    }

    mvprintw(LINES / 2, COLS / 2, "@");  // Initial player position
    refresh();

    int x = COLS / 2, y = LINES / 2;
    char ch;

    while (!win && !lost) {
        ch = getch();
        if (ch == 'n' || ch == 'b' || ch == 'l' || ch == 'k' || ch =='j' || ch =='h' || ch == 'u' || ch == 'y') {
            move_player2(ch, &x, &y, treasure_room_map, player, fallen_weapons, &lost);
            life_decrease++;
        }
        demon_room(treasure_room_map, x, y, player, demons, num_demons, &lost);
        attron(COLOR_PAIR(2));
        mvprintw(LINES - 1, 1, "Life: %d      Food: %d      Gold: %d      Weapons: %d     Health: %d",
                player->life, player->food, player->gold, player->mace + player->dagger+ player->magic_wand+ player->normal_arrow+ player->sword, player->health);
        refresh();
        attroff(COLOR_PAIR(1));

        hunger_count++; //each move decreases user's hunger level

        if (hunger_count == HUNGER_RATE) {
            hungerLevel--;
            display_hunger_level(hungerLevel);
            hunger_count = 0; //reset
            if (life_decrease == 2 * HUNGER_RATE) {
                player->life--;
                life_decrease = 0;
            }

        } else if (ch == 'i') {
            ScreenChar **screendata = NULL;
            save_screen(&screendata, &LINES, &COLS);
            weapon_menu(stdscr, COLS, LINES, wpn, 5, *player);
            display_screen(screendata, LINES, COLS);

        } else if (ch == 'e') {
            ScreenChar **screendata = NULL;
            save_screen(&screendata, &LINES, &COLS);
            int food_num = food_menu(stdscr, COLS, LINES, 3, *player); //food to eat
            food_counts[food_num]--;
            player->food--;
            hungerLevel = MAX_HUNGER;
            player->life += 10;
            display_screen(screendata, LINES, COLS);

        } else if (speed_effect_count) {
            if (speed_effect_count < 10) {
                speed_effect_count++;
            } else {
                speed_effect_count = 0; //reset
            }
            
        }

        if (damage_effect_count > 0) {
            if (damage_effect_count < 10) {
                damage_effect_count++;
            } else {
                damage_effect_count = 0; // reset
                player->weapon_damage = 1; // reset weapon damage
            }
        }

        if (speed_effect_count > 0) {
            if (speed_effect_count < 10) {
                speed_effect_count++;
            } else {
                speed_effect_count = 0; // reset
                player->velocity = 1; // reset velocity
            }
        }

        if (ch == 'o') {
            ScreenChar **screendata = NULL;
            save_screen(&screendata, &LINES, &COLS);
            int spell_num = spell_menu(stdscr, COLS, LINES, 3, *player);

            // spell to eat
            spell_count[spell_num]--; // used

            if (spell_life[spell_num] == 2) {
                player->life = LIFE; // full life
            } else if (spell_speed[spell_num] == 2) {
                player->velocity = 2;
                speed_effect_count = 1;
            } else if (spell_damage[spell_num] == 2) {
                player->weapon_damage = 2;
                damage_effect_count = 1;
            }

            display_screen(screendata, LINES, COLS);
            
        //weapon changing
        } else if (ch == 'w') {
            player->weapon_type = 0;
            attron(A_BOLD);
            mvprintw(0, COLS / 2 - 30, "Weapon safely placed back in your bag!                                             ");
            attroff(A_BOLD);
        } else if (ch == '1') {
            if (!player->weapon_type) {
                player->weapon_type = 1;
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Successfully switched to Mace!                                                    ");
                attroff(A_BOLD);
            } else {
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                attroff(A_BOLD);
            }
        } else if (ch == '2') {
            if (!player->weapon_type) {
                player->weapon_type = 2;
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Successfully switched to Sword!                                                   ");
                attroff(A_BOLD);
            } else {
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                attroff(A_BOLD);
            }
        } else if (ch == '3') {
            if (!player->weapon_type) {
                player->weapon_type = 3;
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Successfully switched to Normal Arrow!                                            ");
                attroff(A_BOLD);
            } else {
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                attroff(A_BOLD);
            }
        } else if (ch == '4') {
            if (!player->weapon_type) {
                player->weapon_type = 4;
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Successfully switched to Dagger!                                                  ");
                attroff(A_BOLD);
            } else {
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                attroff(A_BOLD);
            }
        } else if (ch == '5') {
            if (!player->weapon_type) {
                player->weapon_type = 5;
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Successfully switched to Magic Wand!                                              ");
                attroff(A_BOLD);
            } else {
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                attroff(A_BOLD);
            }

        // Fight logic for space key press
        } else if (ch == ' ') {
            if (player->weapon_type == 1) { // Mace
                handle_short_range_action(x, y, &player->mace, 5, demons, num_demons, 1, damage_effect_count);
            } else if (player->weapon_type == 2) { // Sword;
                handle_short_range_action(x, y, &player->sword, 10, demons, num_demons, 2, damage_effect_count);
            } else if (player->weapon_type == 3) { // Normal Arrow
                handle_long_range_action(x, y, treasure_room_map, demons, num_demons, player, 5, 5, ')', prev_dir, 0, fallen_weapons, 0, damage_effect_count);
            } else if (player->weapon_type == 4) { // Dagger
                handle_long_range_action(x, y, treasure_room_map, demons, num_demons, player, 12, 5, '!', prev_dir, 0, fallen_weapons, 0, damage_effect_count);
            } else if (player->weapon_type == 5) { // Magic Wand
                handle_long_range_action(x, y, treasure_room_map, demons, num_demons, player, 15, 10, '\\', prev_dir, 0, fallen_weapons, 1, damage_effect_count);
            }
            repeat = 1;
        } else if (ch == 'a' && repeat == 1) {
            if (player->weapon_type == 1) { // Mace
                handle_short_range_action(x, y, &player->mace, 5, demons, num_demons, 1, damage_effect_count);
            } else if (player->weapon_type == 2) { // Sword;
                handle_short_range_action(x, y, &player->sword, 10, demons, num_demons, 2, damage_effect_count);
            } else if (player->weapon_type == 3) { // Normal Arrow
                handle_long_range_action(x, y, treasure_room_map, demons, num_demons, player, 5, 5, ')', prev_dir, 0, fallen_weapons, 0, damage_effect_count);
            } else if (player->weapon_type == 4) { // Dagger
                handle_long_range_action(x, y, treasure_room_map, demons, num_demons, player, 12, 5, '!', prev_dir, 0, fallen_weapons, 0, damage_effect_count);
            } else if (player->weapon_type == 5) { // Magic Wand
                handle_long_range_action(x, y, treasure_room_map, demons, num_demons, player, 15, 10, '\\', prev_dir, 0, fallen_weapons, 1, damage_effect_count);
            }
        }
        if (win_check(treasure_room_map)) {
            clear();
            attron(COLOR_PAIR(200));
            mvprintw(LINES / 2, COLS / 2 - 10, "You Winned! Great Job!");
            attroff(COLOR_PAIR(200));
            refresh();
            getch();
            win = 1;

            //save info to file
            int score, gold, n_games; 
            char experience[50];
            char info_filepath[100];
            snprintf(info_filepath, sizeof(info_filepath), "/home/selma/Desktop/Project/Phase1/Users' Info/%s/info.txt", username);
            FILE *file2 = fopen(info_filepath, "r");
            fscanf(file2, "%s %d %d %d %63[^\n]", username, &score, &gold, &n_games, experience);
            score += 1000;
            gold += 1000;
            n_games++;
            fclose(file2);

            FILE *file3 = fopen(info_filepath, "w");

            fprintf(file3, "%s %d %d %d %s", username, score, gold, n_games, experience);
            fclose(file3);

            clear();
            attron(COLOR_PAIR(200));
            mvprintw(LINES / 2 - 2, COLS / 2 - 5, "%10s", username);
            mvprintw(LINES / 2 - 1, COLS / 2 - 5, "%10d", score);
            mvprintw(LINES / 2, COLS / 2 - 5, "%10d", gold);
            attroff(COLOR_PAIR(200));
            refresh();
            getch();

            break;
        }
        if (lost) {
            clear();
            attron(COLOR_PAIR(100));
            mvprintw(LINES / 2, COLS / 2 - 10, "You Lost!");
            attroff(COLOR_PAIR(100));
            refresh();

            lost = 1;

            //save info to file
            int score, gold, n_games; 
            char experience[50];
            char info_filepath[100];
            snprintf(info_filepath, sizeof(info_filepath), "/home/selma/Desktop/Project/Phase1/Users' Info/%s/info.txt", username);
            FILE *file2 = fopen(info_filepath, "r");
            fscanf(file2, "%s %d %d %d %63[^\n]", username, &score, &gold, &n_games, experience);
            score += player->score;
            gold += player->gold;
            n_games++;
            fclose(file2);

            FILE *file3 = fopen(info_filepath, "w");

            fprintf(file3, "%s %d %d %d %s", username, score, gold, n_games, experience);
            fclose(file3);

            clear();
            attron(COLOR_PAIR(100));
            mvprintw(LINES / 2 - 2, COLS / 2 - 5, "%10s", username);
            mvprintw(LINES / 2 - 1, COLS / 2 - 5, "%10d", score);
            mvprintw(LINES / 2, COLS / 2 - 5, "%10d", gold);
            attroff(COLOR_PAIR(100));
            refresh();
            getch();
            break;
        }
    }

    endwin();
}

void handle_music_change(char map[LINES][COLS], int new_x, int new_y, const char *music_files[], int num_music_files) {
    if (map[new_y][new_x] == '+') { // the music must change
        int music = get_random_number(0, num_music_files - 1);
        const char *music_file = music_files[music];

        if (current_music != NULL) {
            Mix_HaltMusic();
            Mix_FreeMusic(current_music);
        }
        current_music = Mix_LoadMUS(music_file);
        if (current_music == NULL) {
            //printf("Mix_LoadMUS error: %s\n", Mix_GetError());
        } else {
            if (Mix_PlayMusic(current_music, -1) == -1) {
                //printf("Mix_PlayMusic error: %s\n", Mix_GetError());
            }
        }
    }
}

void battle_room() {
    initscr();
    clear();
    char message[] = "Welcome to the battle room!";
    mvprintw(LINES / 2, COLS / 2 - strlen(message) / 2, "%s", message);

    refresh();
    getch();
    endwin(); 
}

void move_player(Mix_Music* current_music, char username[6], Room *rooms, int num_rooms, int *next, int *prev, int *door_password, char temp_map[LINES][COLS], char map[LINES][COLS], char traps[LINES][COLS], int *x, int *y, int ch, user *player, int current_floor, char pass_doors_locked[LINES][COLS], char fallen_weapons[LINES][COLS], int spell_effect_count, char treasure_room_map[LINES][COLS], char unvisible_door[LINES][COLS], weapon *wpn, int *lost, int *is_in_enchant_room) {
    static time_t password_start_time;
    int new_x = *x, new_y = *y;
    char ch2;
    start_color();
    init_pair(2, COLOR_CYAN, COLOR_BLUE);
    int room = find_room(new_x, new_y, map, num_rooms, rooms);

    if (password_start_time != 0 && difftime(time(NULL), password_start_time) >= 30) {
        clear_password_display();
        password_start_time = 0;
    }

    if (player->velocity == 2) {
        switch (ch) {
            case 'Y': // NW
            case 'y':
                new_x -= 2;
                new_y -= 2;
                break;
            case 'B': // SW
            case 'b':
                new_x -= 2;
                new_y += 2;
                break;
            case 'J': // N
            case 'j':
                new_y -= 2;
                break;
            case 'H': // W
            case 'h':
                new_x -= 2;
                break;
            case 'L': // E
            case 'l':
                new_x += 2;
                break;
            case 'K': // S
            case 'k':
                new_y += 2;
                break;
            case 'U': // NE
            case 'u':
                new_x += 2;
                new_y -= 2;
                break;
            case 'N': // SE
            case 'n':
                new_x += 2;
                new_y += 2;
                break;
        }
    } else {
        switch (ch) {
            case 'Y': // NW
            case 'y':
                new_x--;
                new_y--;
                break;
            case 'B': // SW
            case 'b':
                new_x--;
                new_y++;
                break;
            case 'J': // N
            case 'j':
                new_y--;
                break;
            case 'H': // W
            case 'h':
                new_x--;
                break;
            case 'L': // E
            case 'l':
                new_x++;
                break;
            case 'K': // S
            case 'k':
                new_y++;
                break;
            case 'U': // NE
            case 'u':
                new_x++;
                new_y--;
                break;
            case 'N': // SE
            case 'n':
                new_x++;
                new_y++;
                break;
        }
    }

    if (traps[new_y][new_x == 1]) {
        player->life--;
        if (player->life <= 0) {
            *lost = 1;
        }
        map[new_y][new_x] = '^'; //marked as trap
        refresh();
    }
    //check if the new position is valid and not a wall or other block
    if (new_x >= 0 && new_x < COLS && new_y >= 0 && new_y < LINES) {
        if (unvisible_door[new_y][new_x] == 1) {
            // Pass through the door
            mvprintw(*y, *x, "%c", temp_map[*y][*x] == '@' ? '.' : temp_map[*y][*x]);
            temp_map[*y][*x] = map[*y][*x];
            *x = new_x;
            *y = new_y;
            mvprintw(*y, *x, "\U0001FBC6");
        } else if ((map[new_y][new_x] == '|' || map[new_y][new_x] == '_') && (map[new_y + 1][new_x] == '#' || map[new_y - 1][new_x] == '#' || map[new_y][new_x - 1] == '#' || map[new_y][new_x + 1] == '#')) {
            mvprintw(*y, *x, "%c", temp_map[*y][*x] == '@' ? '.' : temp_map[*y][*x]);
            map[new_y][new_x] = '?';
            temp_map[*y][*x] = map[*y][*x];
            *x = new_x;
            *y = new_y;
            mvprintw(*y, *x, "\U0001FBC6");
            refresh();
        }
             
        else if (map[new_y][new_x] != '|' && map[new_y][new_x] != '_' && map[new_y][new_x] != 'O' 
                && map[new_y][new_x] != '=' && map[new_y][new_x] != ' ' && map[new_y][new_x] != '@') {
            // Regular movement
            mvprintw(*y, *x, "%c", temp_map[*y][*x] == '@' ? '.' : temp_map[*y][*x]);
            temp_map[*y][*x] = map[*y][*x];
            *x = new_x;
            *y = new_y;
            mvprintw(*y, *x, "\U0001FBC6");

            //treasure room
            if (map[new_y][new_x] == '*') {
                treasure_room(treasure_room_map, player, wpn, username);

            } else if (map[new_y][new_x] == '+') { //the music must change
                handle_music_change(map, new_x, new_y, music_files, 4);
                int room_num = find_room(new_y, new_x, map, num_rooms, rooms);
                if (rooms[room_num].type == 2) {
                    *is_in_enchant_room = 1;
                }
            }
            //collecting items
            else if (map[new_y][new_x] == 'f' && rooms[room].type != 3) {
                ch2 = getch();
                if (ch2 != 'g') {
                    if (player->food < 5) {
                        food_counts[0]++;
                        player->food++;
                        map[new_y][new_x] = '.';
                        attron(COLOR_PAIR(2));
                        move(1, 0);
                        clrtoeol();
                        mvprintw(0, 5, "%-30s", "You picked up some food!");
                        refresh();
                        attroff(COLOR_PAIR(1));
                    }
                }
            }  else if (map[new_y][new_x] == 'a' && rooms[room].type != 3) {
                ch2 = getch();
                if (ch2 != 'g') {
                    if (player->food < 5) {
                        food_counts[1]++;
                        time_food[0] = 1;
                        player->food++;
                        map[new_y][new_x] = '.';
                        attron(COLOR_PAIR(2));
                        move(1, 0);
                        clrtoeol();
                        mvprintw(0, 5, "%-30s", "You picked up some food!");
                        refresh();
                        attroff(COLOR_PAIR(1));
                    }
                }
            } else if (map[new_y][new_x] == 'j' && rooms[room].type != 3) {
                ch2 = getch();
                if (ch2 != 'g') {
                    if (player->food < 5) {
                        food_counts[2]++;
                        time_food[1] = 1;
                        player->food++;
                        map[new_y][new_x] = '.';
                        attron(COLOR_PAIR(2));
                        move(1, 0);
                        clrtoeol();
                        mvprintw(0, 5, "%-30s", "You picked up some food!");
                        refresh();
                        attroff(COLOR_PAIR(1));
                    }
                }
            } else if (map[new_y][new_x] == '$' && rooms[room].type != 3) {
                ch2 = getch();
                if (ch2 != 'g') {
                    player->gold++;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a golden coin!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == 'g' && rooms[room].type != 3) {
                ch2 = getch();
                if (ch2 != 'g') {
                    player->gold += 5; // Each black gold is equal to 5 yellow golds!
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a black golden coin!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == '!' && rooms[room].type != 3) { // Dagger
                ch2 = getch();
                if (ch2 != 'g') {
                    player->dagger += (fallen_weapons[new_y][new_x] == 1) ? 1 : 10;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a dagger!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == '\\' && rooms[room].type != 3) { // Magic wand
                ch2 = getch();
                if (ch2 != 'g') {
                    player->magic_wand += (fallen_weapons[new_y][new_x] == 1) ? 1 : 8;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a magic wand!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == ')' && rooms[room].type != 3) { // Normal arrow
                ch2 = getch();
                if (ch2 != 'g') {
                    player->normal_arrow += (fallen_weapons[new_y][new_x] == 1) ? 1 : 20;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a normal arrow!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == '(' && rooms[room].type != 3) { // Sword
                ch2 = getch();
                if (ch2 != 'g') {
                    if (!player->sword) {
                        player->sword = 1;
                    } 
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a sword!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == '>') {
                char ch3 = getch();
                if (ch3 == '>') { //right arrow
                    *prev = 1;
                } else if (ch3 == '<') { //left arrow
                    *next = 1;
                }
            }
            
            else if (map[new_y][new_x] == 'h' && rooms[room].type != 3) { // Health
                ch2 = getch();
                if (ch2 != 'g') {
                    player->health++;
                    spell_count[0]++;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a health!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == 'd' && rooms[room].type != 3) { // Damage
                ch2 = getch();
                if (ch2 != 'g') {
                    player->damage++;
                    spell_count[2]++;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a damage!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == 's' && rooms[room].type != 3) { // Speed
                ch2 = getch();
                if (ch2 != 'g') {
                    player->speed++;
                    spell_count[1]++;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a speed!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                    refresh();
                }
            } else if (map[new_y][new_x] == 'K') { // Master Key
                ch2 = getch();
                if (ch2 != 'g') {
                    player->masterkey++;
                    map[new_y][new_x] = '.';
                    attron(COLOR_PAIR(2));
                    move(1, 0);
                    clrtoeol();
                    mvprintw(0, 5, "%-30s", "You picked up a master key!");
                    refresh();
                    attroff(COLOR_PAIR(1));
                }
            } else if (map[new_y][new_x] == '&') { // Password generator for door
                *door_password = get_random_number(1000, 9999);
                display_password(*door_password, time(NULL));
                password_start_time = time(NULL);  // Start the timer for displaying the password
            }
            
            if (traps[new_y][new_x] == 1) {
                player->health--;
                ScreenChar **screendata = NULL;
                save_screen(&screendata, &LINES, &COLS);
                battle_room();
                display_screen(screendata, LINES, COLS);
            }


        } else if (map[new_y][new_x] == '@') { //password door
           //if the door is locked and the user has not attempted to open it before
            if (pass_doors_locked[new_y][new_x] == 1) {
                ScreenChar **screendata = NULL;
                save_screen(&screendata, &LINES, &COLS);

                //check if the user has master key
                if (player->masterkey) { //user can pass
                    //there is a 10% chance the key might break!
                    int chance = get_random_number(1, 10);
                    if (chance < 10 && chance >= 0) {
                        mvprintw(*y, *x, "%c", temp_map[*y][*x]);
                        temp_map[*y][*x] = map[*y][*x];
                        *x = new_x;
                        *y = new_y;
                        pass_doors_locked[new_y][new_x] = 0;
                        player->masterkey--; //used
                        attron(COLOR_PAIR(2));
                        move(1, 0);
                        clrtoeol();
                        mvprintw(0, 5, "%-30s", "You used your master key!");
                        refresh();
                        attroff(COLOR_PAIR(1));
                    } else {
                        pass_doors_locked[new_y][new_x] = 1;
                        display_screen(screendata, LINES, COLS);
                        attron(COLOR_PAIR(2));
                        move(1, 0);
                        clrtoeol();
                        mvprintw(0, 5, "%-30s", "Key breaked while using!");
                        refresh();
                        attroff(COLOR_PAIR(1));
                    }
                } else {
                    if (pass_screen(*door_password)) { 
                        display_screen(screendata, LINES, COLS);
                        pass_doors_locked[new_y][new_x] = 0; //unlock the door
                        mvprintw(*y, *x, "%c", temp_map[*y][*x]);
                        temp_map[*y][*x] = map[*y][*x];
                        *x = new_x;
                        *y = new_y;
                        attron(COLOR_PAIR(2));
                        move(1, 0);
                        clrtoeol();
                        mvprintw(0, 5, "%-30s", "Door unlocked successfully!");
                        refresh();
                        attroff(COLOR_PAIR(1));
                    } else {
                        pass_doors_locked[new_y][new_x] = 2; //permanently locked
                        display_screen(screendata, LINES, COLS);
                        attron(COLOR_PAIR(2));
                        move(1, 0);
                        clrtoeol();
                        mvprintw(0, 5, "%-30s", "Door locked permenantly!");
                        refresh();
                        attroff(COLOR_PAIR(1));
                    }
                }

            } else if (pass_doors_locked[new_y][new_x] == 0) {
                //if the door is already unlocked, allow passage
                mvprintw(*y, *x, "%c", temp_map[*y][*x]);
                temp_map[*y][*x] = map[*y][new_x];
                *x = new_x;
                *y = new_y;
                attron(COLOR_PAIR(2));
                move(1, 0);
                clrtoeol();
                mvprintw(0, 5, "%s", "Password door already open!");
                refresh();
                attroff(COLOR_PAIR(1));
            } else { //always locked door
            }
        }

    } else if (unvisible_door[new_y][new_x] == 1) {
        //pass the door
        mvprintw(*y, *x, "%c", temp_map[*y][*x] == '@' ? '.' : temp_map[*y][*x]);
        temp_map[*y][*x] = map[*y][*x];
        *x = new_x;
        *y = new_y;
    }

    for (int n = 0; n < 3; n++) {
        if (time_food[n] != 0) { // If special food exists
            time_food[n]++;
            mvprintw(5, 1, "%d", time_food[n]);

            // Convert special food to normal food after some time (e.g., 10 turns)
            if (time_food[n] >= 10 && food_counts[n] > 0) {
                mvprintw(6, 1, "YES");
                food_counts[n + 1]--;  // Remove special food
                food_counts[0]++;  // Increase normal food count
                time_food[n] = 0;  // Reset the timer
            }

            mvprintw(0, COLS / 2 + n, "%d ", time_food[n]); // Display timer
        }
    }

    refresh();
}

void move_player_continuously(char temp_map[LINES][COLS], char map[LINES][COLS], char traps[LINES][COLS], int *x, int *y, int ch, user *player, int current_floor, char pass_doors_locked[LINES][COLS]) {
    int new_x = *x, new_y = *y;
    static int door_password = 0;
    static time_t password_start_time = 0;

    while (1) {
        switch (ch) {
            case 'Y': // NW
            case 'y':
                new_x--;
                new_y--;
                break;
            case 'B': // SW
            case 'b':
                new_x--;
                new_y++;
                break;
            case 'J': // N
            case 'j':
                new_y--;
                break;
            case 'H': // W
            case 'h':
                new_x--;
                break;
            case 'L': // E
            case 'l':
                new_x++;
                break;
            case 'K': // S
            case 'k':
                new_y++;
                break;
            case 'U': // NE
            case 'u':
                new_x++;
                new_y--;
                break;
            case 'N': // SE
            case 'n':
                new_x++;
                new_y++;
                break;
        }

        if (new_x >= 0 && new_x < COLS && new_y >= 0 && new_y < LINES &&
            map[new_y][new_x] != '|' && map[new_y][new_x] != '_' && map[new_y][new_x] != 'O' &&
            map[new_y][new_x] != '=' && map[new_y][new_x] != ' ') {
                
            mvprintw(*y, *x, "%c", temp_map[*y][*x]);
            temp_map[*y][*x] = map[*y][*x];
            *x = new_x;
            *y = new_y;

            if (map[new_y][new_x] == 'F') {
                if (player->food < 5) {
                    player->food++;
                    map[new_y][new_x] = '.';
                }
            } else if (map[new_y][new_x] == 'g') {
                player->gold++;
                map[new_y][new_x] = '.';
            } else if (map[new_y][new_x] == 'G') {
                player->gold += 5;
            } else if (map[new_y][new_x] == 'M') {
                player->mace++;
                map[new_y][new_x] = '.';
            } else if (map[new_y][new_x] == 'D') {
                player->dagger++;
                map[new_y][new_x] = '.';
            } else if (map[new_y][new_x] == 'W') {
                player->magic_wand++;
                map[new_y][new_x] = '.';
            } else if (map[new_y][new_x] == 'N') {
                player->normal_arrow++;
                map[new_y][new_x] = '.';
            } else if (map[new_y][new_x] == 'R') {
                player->sword++;
                map[new_y][new_x] = '.';
            } else if (map[new_y][new_x] == '<') {
                char next_map[LINES][COLS];
                load_floor(next_map, current_floor);
            } else if (map[new_y][new_x] == 'h') {
                player->health++;
            } else if (map[new_y][new_x] == 'd') {
                player->damage++;
            } else if (map[new_y][new_x] == 's') {
                player->speed++;
            } 

            else if (map[new_y][new_x] == '&') {
                door_password = get_random_number(1000, 9999);
                display_password(door_password, time(NULL));
                password_start_time = time(NULL);
                map[new_y][new_x] = '.';
            }

            if (traps[new_y][new_x] == 1) {
                player->health--;
            }

            if (map[new_y][new_x] == '@') {
                if (pass_doors_locked[new_y][new_x] == 1) {
                    ScreenChar **screendata = NULL;
                    save_screen(&screendata, &LINES, &COLS);

                    if (player->masterkey) {
                        mvprintw(*y, *x, "%c", temp_map[*y][*x]);
                        temp_map[*y][*x] = map[*y][*x];
                        *x = new_x;
                        *y = new_y;
                        pass_doors_locked[new_y][new_x] = 0;
                        player->masterkey--;
                    } else {
                        if (pass_screen(door_password)) {
                            display_screen(screendata, LINES, COLS);
                            pass_doors_locked[new_y][new_x] = 0;
                            mvprintw(*y, *x, "%c", temp_map[*y][*x]);
                            temp_map[*y][*x] = map[*y][*x];
                            *x = new_x;
                            *y = new_y;
                        } else {
                            pass_doors_locked[new_y][new_x] = 2;
                            display_screen(screendata, LINES, COLS);
                        }
                    }

                } else if (pass_doors_locked[new_y][new_x] == 0) {
                    mvprintw(*y, *x, "%c", temp_map[*y][*x]);
                    temp_map[*y][*x] = map[*y][*x];
                    *x = new_x;
                    *y = new_y;
                } else {
                }
            }
        } else {
            break;
        }
    }
}

void read_room_info(const char *filename, Room *rooms, int *num_rooms, char traps[LINES][COLS], char unvisible_door[LINES][COLS]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[256];
    int room_index = -1;
    *num_rooms = 0;

    // Initialize traps and unvisible_door arrays to all zeros
    memset(traps, 0, sizeof(char) * LINES * COLS);
    memset(unvisible_door, 0, sizeof(char) * LINES * COLS);

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Room")) {
            room_index++;
            sscanf(line, "Room %d:", &rooms[room_index].room_number);
            rooms[room_index].num_doors = 0;
            rooms[room_index].num_pillars = 0;
            rooms[room_index].num_windows = 0;
            (*num_rooms)++;
        } else if (strstr(line, "Center")) {
            sscanf(line, "Center: (%d, %d)", &rooms[room_index].center.y, &rooms[room_index].center.x);
        } else if (strstr(line, "Dimensions")) {
            sscanf(line, "Dimensions: %dx%d", &rooms[room_index].dimensions.y, &rooms[room_index].dimensions.x);
        } else if (strstr(line, "Type")) {
            sscanf(line, "Type: %d", &rooms[room_index].type);
        } else if (strstr(line, "Visibility")) {
            sscanf(line, "Visibility: %d", &rooms[room_index].visibility);
        } else if (strstr(line, "Demon")) {
            sscanf(line, "Demon: %d (%d, %d)", &rooms[room_index].demon.type, &rooms[room_index].demon.position.x, &rooms[room_index].demon.position.y);
        } else if (strstr(line, "Traps:")) {
            int x, y;
            char *token = strtok(line, " ");
            while (token != NULL) {
                if (sscanf(token, "(%d, %d)", &x, &y) == 2) {
                    traps[x][y] = 1;
                }
                token = strtok(NULL, " ");
            }
        } else if (strstr(line, "Unvisible Doors:")) {
            int x, y;
            char *token = strtok(line, " ");
            while (token != NULL) {
                if (sscanf(token, "(%d, %d)", &x, &y) == 2) {
                    unvisible_door[x][y] = 1;
                }
                token = strtok(NULL, " ");
            }
        }
    }

    fclose(file);

    // Refresh to display all printed details
    refresh();
}

void demon(char map[LINES][COLS], Room *rooms, int x_ply, int y_ply, int i, user *player, int count) {
    char original_char = map[rooms[i].demon.position.y][rooms[i].demon.position.x];
    char demons[5] = {'D', 'F', 'G', 'S', 'U'};

    if (rooms[i].type == 2) {
        original_char = ',';
    } else if (rooms[i].type == 3) {
        original_char = '~';
    } else if (rooms[i].type == 4) {
        original_char = '.';
    }

    // Restore the original character on the map
    map[rooms[i].demon.position.y][rooms[i].demon.position.x] = original_char;
    mvprintw(rooms[i].demon.position.y, rooms[i].demon.position.x, "%c", original_char);

    // Update the demon’s new position
    if (count < 5 && rooms[i].demon.life > 0) {
        if (rooms[i].demon.position.x < x_ply) {
            rooms[i].demon.position.x++;
        } else if (rooms[i].demon.position.x > x_ply) {
            rooms[i].demon.position.x--;
        }

        if (rooms[i].demon.position.y < y_ply) {
            rooms[i].demon.position.y++;
        } else if (rooms[i].demon.position.y > y_ply) {
            rooms[i].demon.position.y--;
        }
    }
    // Place the demon’s character at the new position
    map[rooms[i].demon.position.y][rooms[i].demon.position.x] = demons[rooms[i].demon.type];
    mvprintw(rooms[i].demon.position.y, rooms[i].demon.position.x, "%c", demons[rooms[i].demon.type]);

    int distance = 2;
    if (sqrt(pow((x_ply - rooms[i].demon.position.x), 2) + pow((y_ply - rooms[i].demon.position.y), 2)) <= distance) {
        player->life -= rooms[i].demon.damage; 
    }
}

void weapon_properties(user *player, weapon *wpn) {
    switch (player->weapon_type) {
        case 1:
            wpn->damage = 5;
            wpn->distance = 1;
            break;
        case 2:
            wpn->damage = 12;
            wpn->distance = 5;
            break;
        case 3:
            wpn->damage = 15;
            wpn->distance = 10;
            break;
        case 4:
            wpn->damage = 5;
            wpn->distance = 5;
            break;
        case 5:
            wpn->damage = 10;
            wpn->distance = 1;
            break;
    }
}

void demon_properties (Room *rooms, int num_rooms) {
    for (int  i = 0 ; i < num_rooms; i++) {
        switch (rooms[i].demon.type) {
            case 1:
                rooms[i].demon.life = 5;
                rooms[i].demon.damage = 1;
                break;
            case 2:
                rooms[i].demon.life = 10;
                rooms[i].demon.damage = 2;
                break;
            case 3:
                rooms[i].demon.life = 15;
                rooms[i].demon.damage = 2;
                break;
            case 4:
                rooms[i].demon.life = 20;
                rooms[i].demon.damage = 2;
                break;
            case 5:
                rooms[i].demon.life = 30;
                rooms[i].demon.damage = 3;
                break;
        }
    }
}

void handle_short_range_weapon(int x, int y, int *weapon_count, int damage, Room rooms[], int i, int type, int spell_effect) {
    bool demon_nearby = false;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (rooms[i].demon.position.y == y + dy && rooms[i].demon.position.x == x + dx) {
                if (type == 1) {
                    (*weapon_count)--;
                }
                if (spell_effect) {
                    rooms[i].demon.life -= (damage * 2);
                } else {
                    rooms[i].demon.life -= damage;
                }
                demon_nearby = true;
                break;
            } 
        }
        if (demon_nearby) break;
    }
    if (!demon_nearby) {
        (*weapon_count)--;
    }

    if (rooms[i].demon.life <= 0) {
        rooms[i].demon.position.x = 0;
        rooms[i].demon.position.y = 0;
        rooms[i].demon.life = 0;
    }
}

void handle_long_range_weapon(int x, int y, char map[LINES][COLS], Room rooms[], int i, user *player, int damage, int range, char weapon, char prev_dir[2], int repeat, char fallen_weapons[LINES][COLS], int is_magic_wand, int spell_effect) {
    int dx = 0, dy = 0;
    int demon_hit = 0;
    int *weapon_count_ptr;
    
    // Direction input
    if (!repeat) {
        int dir = getch();
        switch (dir) {
            case 'y': dx = -1; dy = -1; break;
            case 'u': dx = 1; dy = -1; break;
            case 'h': dx = -1; dy = 0; break;
            case 'j': dx = 0; dy = 1; break;
            case 'k': dx = 0; dy = -1; break;
            case 'l': dx = 1; dy = 0; break;
            case 'b': dx = -1; dy = 1; break;
            case 'n': dx = 1; dy = 1; break;
            default: return;
        }

        prev_dir[0] = dx;
        prev_dir[1] = dy;
    } else {
        dx = prev_dir[0];
        dy = prev_dir[1];
    }

    // Decrement based on weapon type
    switch (player->weapon_type) {
        case 3: player->normal_arrow--; weapon_count_ptr = &player->normal_arrow; break;
        case 4: player->dagger--; weapon_count_ptr = &player->dagger; break;
        case 5: 
            player->magic_wand--; 
            weapon_count_ptr = &player->magic_wand; 
            is_magic_wand = 1; // Setting flag indicating magic wand selected
            break;
        default: return;
    }

    // Demon hit check
    for (int dist = 0; dist <= range; dist++) {
        int nx = x + dx * dist;
        int ny = y + dy * dist;
        if (rooms[i].demon.position.x == nx && rooms[i].demon.position.y == ny) {
            if (spell_effect) {
                rooms[i].demon.life -= (damage * 2);
            } else {
                rooms[i].demon.life -= damage;
            }
            if (is_magic_wand) { // Immediately kills demon with magic wand
                rooms[i].demon.life = 0;
            }
            demon_hit = 1;
            break;
        }
    }

    // Weapon fallout on miss
    if (!demon_hit) {
        for (int dist = 1; dist <= range; dist++) {
            int nx = x + dx * dist;
            int ny = y + dy * dist;
            if (map[ny][nx] == '_' || map[ny][nx] == '|' || map[ny][nx] == '=' || map[ny][nx] == '+') {
                nx = x + dx * (dist - 1);
                ny = y + dy * (dist - 1);
                map[ny][nx] = weapon;
                mvprintw(ny, nx, "%c", map[ny][nx]);
                break;
            } else if (dist == range) {
                map[ny][nx] = weapon;
                fallen_weapons[ny][nx] = 1;
                mvprintw(ny, nx, "%c", map[ny][nx]);
            }
        }
    } else { // Demon hit message
        attron(A_BLINK | A_BOLD);
        mvprintw(0, COLS / 2 - 30, "%40s", "Demon successfully hit! Great Job!");
        attroff(A_BLINK | A_BOLD);
    }

    if (rooms[i].demon.life <= 0) {
        rooms[i].demon.type = -1;
        rooms[i].demon.position = (Point){-1, -1};
    }
}

void display_map_with_unicode(char map[LINES][COLS]) {
    setlocale(LC_ALL, "");

    const char *ancient_key = "\U000025B3";

    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            if (map[i][j] == 'K') {
                mvaddstr(i, j, ancient_key);
            } else {
                mvprintw(i, j, "%c", map[i][j]);
            }
        }
    }

    refresh();
}

void visibility(Room *rooms, int x, int y, int num_rooms, char map[LINES][COLS], char explored_map[LINES][COLS]) {
    int current_room = find_room(x, y, map, num_rooms, rooms);
    if (current_room != -1) {
        rooms[current_room].visibility = 1;

    }
    for (int i =0; i < num_rooms; i++) {
        if (rooms[i].visibility) {
            mvprintw(3, i, "%d", i);
            int start_x = rooms[i].center.x - rooms[i].dimensions.y / 2;
            int end_x = rooms[i].center.x + rooms[i].dimensions.y / 2;
            int start_y = rooms[i].center.y - rooms[i].dimensions.x / 2;
            int end_y = rooms[i].center.y + rooms[i].dimensions.x / 2;

            for (int x = start_x; x <= end_x; x++) {
                for (int y = start_y; y <= end_y; y++) {
                    //mvaddch(x, y, map[x][y]);
                    explored_map[x][y] = map[x][y];
                }
            }
        }
    }
     
    refresh(); 
}

void print_explored_rooms(Room *rooms, int num_rooms, char map[LINES][COLS], int x_ply, int y_ply, char pass_doors_locked[LINES][COLS], char explored_map[LINES][COLS]) {
    start_color();

    init_color(COLOR_YELLOW, 1000, 1000, 0);
    init_color(COLOR_PURPLE, 600, 0, 600);
    init_color(COLOR_ORANGE, 900, 500, 0);
    init_color(COLOR_DARK_PINK, 600, 0, 450);

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_DARK_PINK, COLOR_BLACK);
    init_pair(7, COLOR_PURPLE, COLOR_BLACK);

    // Check if the player is in a room
    int num = find_room(x_ply, y_ply, map, num_rooms, rooms);
    char direction[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    if (num == -1) {
        for (int i = 0; i < 4; i++) {
            if (map[y_ply + direction[i][1]][x_ply + direction[i][0]] == '#') {
                for (int n = 0; n < 6; n++) {
                    if (direction[i][1] == 0) {
                        if (map[y_ply + direction[i][1]][x_ply + direction[i][0] + n] == '#') {
                            explored_map[y_ply + direction[i][1]][x_ply + direction[i][0] + n] = map[y_ply + direction[i][1]][x_ply + direction[i][0] + n];
                        }
                    } else {
                        if (map[y_ply + direction[i][1] + n][x_ply + direction[i][0]] == '#') {
                            explored_map[y_ply + direction[i][1]][x_ply + direction[i][0]] = map[y_ply + direction[i][1] + n][x_ply + direction[i][0]];
                        }
                    }
                }
            }
        }

    } else {
        rooms[num].visibility = 1;
    }

    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            if (explored_map[i][j] == '@') {
                if (pass_doors_locked[i][j] == 0) {
                    attron(COLOR_PAIR(2));
                    mvprintw(i, j, "%c", explored_map[i][j]);
                    attroff(COLOR_PAIR(2));
                } else {
                    attron(COLOR_PAIR(1));
                    mvprintw(i, j, "%c", explored_map[i][j]);
                    attroff(COLOR_PAIR(1));
                }
            } else if (explored_map[i][j] == 'g' || explored_map[i][j] == '$' || explored_map[i][j] == '*') {
                attron(COLOR_PAIR(3));
                mvprintw(i, j, "%c", explored_map[i][j]);
                attroff(COLOR_PAIR(3));
            } else if (explored_map[i][j] == '&') {
                attron(A_BOLD);
                mvprintw(i, j, "%c", explored_map[i][j]);
                attroff(A_BOLD);                       
            } else if (explored_map[i][j] == 'f') {
                attron(COLOR_PAIR(5));
                mvprintw(i, j, "%c", explored_map[i][j]);
                attroff(COLOR_PAIR(5));
            } else if (explored_map[i][j] == '!' || explored_map[i][j] == '\\' || explored_map[i][j] == '(' || explored_map[i][j] == ')') {
                attron(COLOR_PAIR(6));
                mvprintw(i, j, "%c", explored_map[i][j]);
                attroff(COLOR_PAIR(6));
            } else if (explored_map[i][j] == 'h' || explored_map[i][j] == 'd' || explored_map[i][j] == 's') {
                attron(COLOR_PAIR(7));
                mvprintw(i, j, "%c", explored_map[i][j]);
                attroff(COLOR_PAIR(7));
            } else {
                mvprintw(i, j, "%c", explored_map[i][j]);
            }
        }
    }
    refresh();
}

int main() {
    setlocale(LC_ALL, "");
    srand(time(0));
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    char map[LINES][COLS];
    char treasure_room_map[LINES][COLS];

    user player = {0, 0};  // Initialize player
    weapon wpn;
    Mix_Music* current_music = NULL;

    weapon_properties(&player, &wpn);
    char username[]="Selma";

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_GREEN);
    init_pair(100, COLOR_RED, COLOR_BLACK);

    int current_floor = 1;
    int keep_running = 1;
    int hungerLevel = MAX_HUNGER;
    int speed_effect_count = 0;
    int damage_effect_count = 0;
    player.life = LIFE;
    char treasure_room_filepath[50];
    int win = 0, lost = 0;
    int count_move = 0;
    int limited_move = 0;

    for (int i = 0; i < 3; i++) {
        food_counts[i] = 0;
        time_food[i] = -1;
    }
    

    sprintf(treasure_room_filepath, "Treasure_Room.txt");
    FILE *fptr = fopen(treasure_room_filepath, "r");
    
    for (int i = 0; i < LINES; i++) { 
        for (int j = 0; j < COLS; j++) {
            int ch = fgetc(fptr);
            if (ch == EOF) {
                treasure_room_map[i][j] = ' ';
            } else {
                treasure_room_map[i][j] = (char) ch;
            }
        }
    }

    for (int floor = 1; floor <= 4;  floor++) {
        if (floor > 1) {
            attron(A_REVERSE);
            mvprintw(0, COLS / 2 - 10, "You entered next floor!");
            attroff(A_REVERSE);
        }
        clear();
        int hunger_count = 0;
        int repeat = 0;
        char prev_dir[2];
        int next = 0;
        int prev = 0;
        int is_in_enchant_room = 0;
        int count_enchant = 0;

        Room rooms[MAX_ROOMS];
        memset(rooms, 0, sizeof(rooms));
        int num_rooms = 0;
        char file_name1[50], file_name2[50];
        char demons2[LINES][COLS];
        char traps[LINES][COLS];
        char fallen_weapons[LINES][COLS];
        char unvisible_door[LINES][COLS];
        char explored_map[LINES][COLS];

        for (int i = 0; i < LINES; i++) {
            for (int j = 0; j < COLS; j++) {
                explored_map[i][j] = ' ';
            }
        }


        for (int i = 0; i < LINES; i++) {
            for (int j = 0; j < COLS; j++) {
                traps[i][j] = 0;
            }
        }

        
        sprintf(file_name1, "rooms_info_%d.txt", floor);
        read_room_info(file_name1, rooms, &num_rooms, traps, unvisible_door);

        demon_properties(rooms, num_rooms);


        sprintf(file_name2, "map%d.txt", floor);
        FILE *fptr = fopen(file_name2, "r");
        
        for (int i = 0; i < LINES; i++) { 
            for (int j = 0; j < COLS; j++) {
                int ch = fgetc(fptr);
                if (ch == EOF) {
                    map[i][j] = ' ';
                    temp_map[i][j] = ' ';
                } else {
                    map[i][j] = (char) ch;
                    temp_map[i][j] = (char) ch;
                }
            }
        }

        fclose(fptr);

        //display_map_with_unicode(map);

        // for (int i = 0; i < LINES; i++) {
        //     for (int j = 0; j < COLS; j++) {
        //         mvaddch(i + 2, j, map[i + 2][j]);
        //     }
        // }

        int valid = 0;
        int x, y;

        while (!valid) {
            x = get_random_number(1, COLS - 1);
            y = get_random_number(1, LINES - 1);
            if (map[y][x] != ' ' && map[y][x] != '_' && map[y][x] != '|' && map[y][x] != 'O') { 
                valid = 1;
            }
        }

        mvprintw(y, x, "\U0001FBC6");
        temp_map[y][x] = map[y][x]; 
        refresh();

        char pass_doors_locked[LINES][COLS];
        for (int i = 0; i < LINES; i++) {
            for (int j = 9; j < COLS; j++) {
                pass_doors_locked[i][j] = 1;
            }
        }
        
        player.weapon_type = 1;

        int ch;
        int door_password;
        int life_decrease = 0;
    
        while ((ch = getch()) != 'q') { // 'q' for quit
            //visibility(rooms, x, y, num_rooms, map, explored_map);
            //print_explored_rooms(rooms, num_rooms, map, x, y, pass_doors_locked, explored_map);
            color(map, pass_doors_locked);
            if (ch == 'f') { //continuous movements
                ch = getch();
                move_player_continuously(temp_map, map, traps, &x, &y, ch, &player, current_floor, pass_doors_locked);
            } else if (ch == 'i') {
                ScreenChar **screendata = NULL;
                save_screen(&screendata, &LINES, &COLS);
                weapon_menu(stdscr, COLS, LINES, &wpn, 5, player);
                display_screen(screendata, LINES, COLS);

            } else if (ch == 'e') {
                ScreenChar **screendata = NULL;
                save_screen(&screendata, &LINES, &COLS);
                int food_num = food_menu(stdscr, COLS, LINES, 3, player); // Food selection menu
                display_screen(screendata, LINES, COLS);
                getch();

                if (food_counts[food_num] > 0) {
                    food_counts[food_num]--; // Decrease selected food count
                    player.food--;

                    // Apply effects based on food type
                    if (food_num == 1) {
                        player.life = LIFE;
                        damage_effect_count = 1;
                    } else if (food_num == 2) {
                        player.life = LIFE;
                        speed_effect_count = 1;
                    } else if (food_num == 0) {
                        int chance = get_random_number(1, 10);
                        if (chance >= 0 && chance <= 8) {
                            player.life = LIFE;
                        } else {

                            player.life--;
                            attron(COLOR_PAIR(100));
                            mvprintw(0, COLS / 2 - 10, "%30s", "Oops! you ate rotten food!");
                            attron(COLOR_PAIR(100));
                        }
                    }

                    hungerLevel = MAX_HUNGER; // Reset hunger
                    refresh();
                }

            } else if (speed_effect_count) {
                if (speed_effect_count < 10) {
                    speed_effect_count++;
                } else {
                    speed_effect_count = 0; //reset
                }
                
            }

            if (damage_effect_count > 0) {
                if (damage_effect_count < 10) {
                    damage_effect_count++;
                } else {
                    damage_effect_count = 0; // reset
                    player.weapon_damage = 1; // reset weapon damage
                }
            }

            if (speed_effect_count > 0) {
                if (speed_effect_count < 10) {
                    speed_effect_count++;
                } else {
                    speed_effect_count = 0; // reset
                    player.velocity = 1; // reset velocity
                }
            }

            if (ch == 'o') {
                ScreenChar **screendata = NULL;
                save_screen(&screendata, &LINES, &COLS);
                int spell_num = spell_menu(stdscr, COLS, LINES, 3, player);

                // spell to eat
                spell_count[spell_num]--; // used

                if (spell_life[spell_num] == 2) {
                    player.life = LIFE; // full life
                } else if (spell_speed[spell_num] == 2) {
                    player.velocity = 2;
                    speed_effect_count = 1;
                } else if (spell_damage[spell_num] == 2) {
                    player.weapon_damage = 2;
                    damage_effect_count = 1;
                }

                display_screen(screendata, LINES, COLS);
                
            } else if (ch == 's') { // Save the game
                char map_filepath[100];
                char username[20] = "Selma"; // Adjust size as needed
                snprintf(map_filepath, sizeof(map_filepath), "/home/selma/Desktop/Project/Phase1/Users' Info/%s/map.txt", username);
                FILE *file1 = fopen(map_filepath, "w");
                
                for (int i = 0; i < LINES; i++) {
                    for (int j = 0; j < COLS; j++) {
                        fputc(map[i][j], file1);
                    }
                }
                fclose(file1);
                
                int score, gold, n_games; 
                char experience[50];
                char info_filepath[100];
                snprintf(info_filepath, sizeof(info_filepath), "/home/selma/Desktop/Project/Phase1/Users' Info/%s/info.txt", username);
                FILE *file2 = fopen(info_filepath, "r");
                fscanf(file2, "%s %d %d %d %63[^\n]", username, &score, &gold, &n_games, experience);
                score += player.score;
                gold += player.gold;
                n_games += player.number_of_finished_games;
                fclose(file2);

                FILE *file3 = fopen(info_filepath, "w");

                fprintf(file3, "%s %d %d %d %s", username, score, gold, n_games, experience);
                fclose(file3);

                sprintf(file_name1, "rooms_info_%d.txt", floor);
                read_room_info(file_name1, rooms, &num_rooms, traps, unvisible_door);
                FILE *file4 = fopen(file_name1, "w");
                // Write back the updated room information, including the visibility field
                for (int i = 0; i < num_rooms; i++) {
                    fprintf(file4, "Room %d:\n", rooms[i].room_number);
                    fprintf(file4, "Center: (%d, %d)\n", rooms[i].center.y, rooms[i].center.x);
                    fprintf(file4, "Dimensions: %dx%d\n", rooms[i].dimensions.y, rooms[i].dimensions.x);
                    fprintf(file4, "Type: %d\n", rooms[i].type);
                    fprintf(file4, "Visibility: %d\n", rooms[i].visibility);
                    fprintf(file4, "Demon: %d (%d, %d)\n", rooms[i].demon.type, rooms[i].demon.position.y, rooms[i].demon.position.x);

                    fprintf(file4, "\n");
                }

                fclose(file4);

                clear();
                mvprintw(LINES / 2, COLS / 2 - 20, "Game saved successfully!");
                refresh();
                getch();
                return 0;
            

            //weapon changing
            } else if (ch == 'w') {
                player.weapon_type = 0;
                attron(A_BOLD);
                mvprintw(0, COLS / 2 - 30, "Weapon safely placed back in your bag!                                             ");
                attroff(A_BOLD);
            } else if (ch == '1') {
                if (!player.weapon_type) {
                    player.weapon_type = 1;
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Successfully switched to Mace!                                                    ");
                    attroff(A_BOLD);
                } else {
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                    attroff(A_BOLD);
                }
            } else if (ch == '2') {
                if (!player.weapon_type) {
                    player.weapon_type = 2;
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Successfully switched to Sword!                                                   ");
                    attroff(A_BOLD);
                } else {
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                    attroff(A_BOLD);
                }
            } else if (ch == '3') {
                if (!player.weapon_type) {
                    player.weapon_type = 3;
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Successfully switched to Normal Arrow!                                            ");
                    attroff(A_BOLD);
                } else {
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                    attroff(A_BOLD);
                }
            } else if (ch == '4') {
                if (!player.weapon_type) {
                    player.weapon_type = 4;
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Successfully switched to Dagger!                                                  ");
                    attroff(A_BOLD);
                } else {
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                    attroff(A_BOLD);
                }
            } else if (ch == '5') {
                if (!player.weapon_type) {
                    player.weapon_type = 5;
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Successfully switched to Magic Wand!                                              ");
                    attroff(A_BOLD);
                } else {
                    attron(A_BOLD);
                    mvprintw(0, COLS / 2 - 30, "Please place your current weapon back in your bag before switching to another one!");
                    attroff(A_BOLD);
                }
            }
            

            // Fight logic for space key press
            else if (ch == ' ') {
                int i = find_room(x, y, map, num_rooms, rooms);
                if (i != -1) {
                    if (player.weapon_type == 1) { // Mace
                        handle_short_range_weapon(x, y, &player.mace, 5, rooms, i, 0, speed_effect_count);
                    } else if (player.weapon_type == 2) { // Sword
                        handle_short_range_weapon(x, y, &player.sword, 10, rooms, i, 1, speed_effect_count);
                    } else if (player.weapon_type == 3) { // Normal Arrow
                        handle_long_range_weapon(x, y, map, rooms, i, &player, 5, 5, ')', prev_dir, 0, fallen_weapons, 0, speed_effect_count);
                    } else if (player.weapon_type == 4) { // Dagger
                        handle_long_range_weapon(x, y, map, rooms, i, &player, 12, 5, '!', prev_dir, 0, fallen_weapons, 0, speed_effect_count);
                    } else if (player.weapon_type == 5) { // Magic Wand
                        handle_long_range_weapon(x, y, map, rooms, i, &player, 15, 10, '\\', prev_dir, 0, fallen_weapons, 1, speed_effect_count);
                    }
                }
                repeat = 1;
            } else if (ch == 'a' && repeat == 1) {
                int i = find_room(x, y, map, num_rooms, rooms);
                if (i != -1) {
                    if (player.weapon_type == 1) { // Mace
                        handle_short_range_weapon(x, y, &player.mace, 5, rooms, i, 0, speed_effect_count);
                    } else if (player.weapon_type == 2) { // Sword
                        handle_short_range_weapon(x, y, &player.sword, 10, rooms, i, 1, speed_effect_count);
                    } else if (player.weapon_type == 3) { // Normal Arrow
                        handle_long_range_weapon(x, y, map, rooms, i, &player, 5, 5, ')', prev_dir, 0, fallen_weapons, 0, speed_effect_count);
                    } else if (player.weapon_type == 4) { // Dagger
                        handle_long_range_weapon(x, y, map, rooms, i, &player, 12, 5, '!', prev_dir, 0, fallen_weapons, 0, speed_effect_count);
                    } else if (player.weapon_type == 5) { // Magic Wand
                        handle_long_range_weapon(x, y, map, rooms, i, &player, 15, 10, '\\', prev_dir, 0, fallen_weapons, 1, speed_effect_count);
                    }
                }
            } else if (ch == 'm'){
                for (int i = 0; i < LINES; i++) {
                    for (int j = 0; j < COLS; j++) {
                        mvaddch(i + 2, j, map[i + 2][j]);
                    }
                }
                getch();
                clear();
            }

            else { //single step movements
                move_player(current_music, username, rooms, num_rooms, &next, &prev, &door_password, temp_map, map, traps, &x, &y, ch, &player, current_floor, pass_doors_locked, fallen_weapons, speed_effect_count, treasure_room_map, unvisible_door, &wpn, &lost, &is_in_enchant_room);
                if (next == 1) {
                    mvprintw(1, 1, "NEXT");
                    floor++;
                    break;
                } else if (prev == 1) {
                    mvprintw(1, 1, "PREV");
                    floor--;
                    break;
                }
                life_decrease++;

                if (is_in_enchant_room) {
                    count_enchant++;
                } else {
                    count_enchant = 0;
                }

                if (count_enchant == 10) {
                    player.life--;
                }
            }

            //move demon when in room
            int current_room = find_room(x, y, map, num_rooms, rooms);
            mvprintw(1, 1, "%d", current_room);
            if (current_room != -1) {
                if (1) {
                    char *type_dem[5] = {"Deamon", "Fire Breathing Monster", "Giant", "Snake", "Undead"};
                    
                    init_pair(1, COLOR_RED, COLOR_BLACK);
                    attron(COLOR_PAIR(1) | A_REVERSE);
                    
                    mvprintw(LINES - 1, COLS - 90, "%s Life: %10d", type_dem[rooms[current_room].demon.type - 1], rooms[current_room].demon.life);
                    attroff(COLOR_PAIR(1) | A_REVERSE);

                    if (rooms[current_room].demon.type == 2 || rooms[current_room].demon.type == 4) {
                        count_move++;
                    } else {
                        count_move = 0;
                    }
                    demon(map, rooms, x, y, current_room, &player, count_move);
                    refresh();
                }
                rooms[current_room].visibility = 1;
            }

            mvprintw(y, x, "\U0001FBC6");
            attron(COLOR_PAIR(2));
            mvprintw(LINES - 1, 1, "Life: %d      Food: %d      Gold: %d      Weapons: %d     Health: %d      Master key: %d",
                    player.life, player.food, player.gold, player.mace + player.dagger+ player.magic_wand+ player.normal_arrow+ player.sword, player.health, player.masterkey);
            refresh();
            attroff(COLOR_PAIR(1));

            hunger_count++; //each move decreases user's hunger level
            display_hunger_level(hungerLevel);
            if (hunger_count == HUNGER_RATE) {
                hungerLevel--;
                hunger_count = 0; //reset
            } if (life_decrease == 2 * HUNGER_RATE) {
                player.life--;
                life_decrease = 0;
            }

            
            if (lost) {
                clear();
                attron(COLOR_PAIR(100));
                mvprintw(LINES / 2, COLS / 2 - 10, "You Lost!");
                attroff(COLOR_PAIR(100));
                refresh();
                getch();
                return 0;
            }
        }
    }
    endwin();
    return 0;
}
