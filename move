#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

int get_random_number(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void move_player(char temp_map[LINES][COLS], char map[LINES][COLS], int *x, int *y, int ch) {
    int new_x = *x, new_y = *y;

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

    if (new_x >= 0 && new_x < COLS && new_y >= 0 && new_y < LINES && map[new_y][new_x] != '|' &&
        map[new_y][new_x] != '_' && map[new_y][new_x] != 'O' && map[new_y][new_x] != '=' && map[new_y][new_x] != ' ') {
        mvprintw(*y, *x, "%c", temp_map[*y][*x]);
        temp_map[*y][*x] = map[*y][*x];
        *x = new_x;
        *y = new_y;
    }
}

void move_player_continuous(char temp_map[LINES][COLS], char map[LINES][COLS], int *x, int *y, int ch) {
    int new_x = *x, new_y = *y;
    int move = 1;

    while (move) {
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

        if (new_x >= 0 && new_x < COLS && new_y >= 0 && new_y < LINES && map[new_y][new_x] != '|' &&
            map[new_y][new_x] != '_' && map[new_y][new_x] != 'O' && map[new_y][new_x] != '=' && map[new_y][new_x] != ' ') {
            mvprintw(*y, *x, "%c", temp_map[*y][*x]);
            temp_map[*y][*x] = map[*y][*x];
            *x = new_x;
            *y = new_y;
            mvprintw(*y, *x, "@");
            refresh();
        } else {
            move = 0;
        }
    }
}

int main() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    char map[LINES][COLS];
    char temp_map[LINES][COLS];

    FILE *fptr = fopen("map.txt", "r");

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

    for (int i = 0; i < LINES; i++) {
        for (int j = 0; j < COLS; j++) {
            mvaddch(i, j, map[i][j]);
        }
    }

    int valid = 0;
    int x, y;

    while (!valid) {
        x = get_random_number(1, COLS - 1);
        y = get_random_number(1, LINES - 1);
        if (map[y][x] != ' ' && map[y][x] != '_' && map[y][x] != '|' && map[y][x] != 'O') { //the character can only be in rooms and corridors
            valid = 1;
        }
    }

    int ch;
    mvprintw(y, x, "@");
    temp_map[y][x] = map[y][x];
    refresh();

    while ((ch = getch()) != 'q') { // 'q' for quit
        if (ch == 'f') {
            ch = getch();
            move_player_continuous(temp_map, map, &x, &y, ch);
        } else {
            move_player(temp_map, map, &x, &y, ch);
        }
        mvprintw(y, x, "@");
    }

    endwin();
    return 0;
}
