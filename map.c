#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <ncurses.h>
#include <string.h>


#define MAX_ELEMENTS 10
#define MIN_ROOMS 6
#define MAX_ROOMS 10
#define MAX_PILLARS_IN_EACH_ROOM 4
#define MAX_NUMBER_PASSWORD_DOORS 5
#define MAX_NUMBER_SECRET_DOORS 5
#define MAP_SIZEy 43
#define MAP_SIZEx 183
#define MIN_TRAPS 10
#define MAX_TRAPS 30
#define MIN_FOOD_IN_EACH_FLOOR 5
#define MAX_FOOD_IN_EACH_FLOOR 15
#define MAX_WEAPONS_IN_EACH_FLOOR 40
#define MIN_WEAPONS_IN_EACH_FLOOR 20
#define MAX_SPELLS_IN_EACH_FLOOR 10
#define MAX_GOLDS_IN_TREASURE_ROOM 50
#define MAX_TRAPS_IN_TREASURE_ROOM 20
#define MAX_DEMONS_IN_TREASURE_ROOM 20
#define MAX_DOORS 5

typedef struct {
    int center_x;
    int center_y;
    int width;
    int length;
    int doors[MAX_ELEMENTS][2];
    int num_doors;
    int pillars[MAX_ELEMENTS][2];
    int num_pillars;
    int windows[MAX_ELEMENTS][2];
    int num_windows;
    int type; //1:Treasure , 2:Enchant, 3:Night mare, 4:Regular room
    int demon_x, demon_y;
    int demon_type;
} Room;

typedef struct {
    int row, col;
} Pair;

typedef struct {
    int x, y;
} Door;



int random_10() {
    return rand() % 2;
}

int get_random_number(int min, int max) {
    return rand() % (max - min + 1) + min;
}

bool do_rooms_intersect(Room rm1, Room rm2) {
    int left1 = rm1.center_x - rm1.length / 2;
    int right1 = rm1.center_x + rm1.length / 2;
    int top1 = rm1.center_y - rm1.width / 2;
    int bottom1 = rm1.center_y + rm1.width / 2;

    int left2 = rm2.center_x - rm2.length / 2;
    int right2 = rm2.center_x + rm2.length / 2;
    int top2 = rm2.center_y - rm2.width / 2;
    int bottom2 = rm2.center_y + rm2.width / 2;

    return !(right1 + 6 < left2|| left1 > right2 + 6|| bottom1 + 6 < top2 || top1 > bottom2 + 6);
}

void room_builder(Room *rooms, int *num_rooms, char map[MAP_SIZEx][MAP_SIZEy]) {
    int room_count = get_random_number(MIN_ROOMS, MAX_ROOMS);
    int rooms_created = 0;

    while (room_count > 0 && rooms_created < MAX_ROOMS) {
        rooms[rooms_created].center_x = get_random_number(8, MAP_SIZEx - 8);
        rooms[rooms_created].center_y = get_random_number(8, MAP_SIZEy - 8);
        rooms[rooms_created].width = get_random_number(4, 20);
        rooms[rooms_created].length = get_random_number(4, 30);

        int intersection = 0;
        for (int j = 0; j < rooms_created; j++) {
            if (do_rooms_intersect(rooms[rooms_created], rooms[j])) {
                intersection = 1;
                break;
            }
        }

        if (!intersection) {
            int left = rooms[rooms_created].center_x - rooms[rooms_created].length / 2;
            int right = rooms[rooms_created].center_x + rooms[rooms_created].length / 2;
            int top = rooms[rooms_created].center_y - rooms[rooms_created].width / 2;
            int bottom = rooms[rooms_created].center_y + rooms[rooms_created].width / 2;

            // Assign a type (2 - Enchant, 3 - Nightmare, 4 - Regular) randomly
            rooms[rooms_created].type = get_random_number(2, 4);

            // room walls
            for (int x = left; x <= right; x++) {
                map[x][top] = '_';    // top wall
                map[x][bottom] = '_'; // bottom wall
            }
            for (int y = top + 1; y <= bottom; y++) {
                map[left][y] = '|';  // left wall
                map[right][y] = '|'; // right wall
            }

            // fill room with appropriate floor type
            char floor_char;
            switch (rooms[rooms_created].type) {
                case 2:
                    floor_char = ',';
                    break;
                case 3:
                    floor_char = '~';
                    break;
                case 4:
                    floor_char = '.';
                    break;
            }

            for (int x = left + 1; x < right; x++) {
                for (int y = top + 1; y < bottom; y++) {
                    map[x][y] = floor_char;
                }
            }

            rooms_created++;
            room_count--;
        }
    }
    *num_rooms = rooms_created;
}

void treasure_room (Room *rooms, int *num_rooms, char map[MAP_SIZEx][MAP_SIZEy]) {
    int intersection = 0;
    do {
        intersection = 0;  // Reset the intersection flag

        // Generate room characteristics
        int center_x = get_random_number(8, MAP_SIZEx - 4);
        int center_y = get_random_number(4, MAP_SIZEy - 4);
        int width = get_random_number(4, 15);
        int length = get_random_number(4, 30);

        // Check if the room intersects with any existing rooms
        for (int j = 0; j < *num_rooms; j++) {
            if (do_rooms_intersect((Room){center_x, center_y, width, length, 1}, rooms[j])) {
                intersection = 1;
                break;
            }
        }

        // If there's no intersection, create the room
        if (!intersection) {
            int left = center_x - length / 2;
            int right = center_x + length / 2;
            int top = center_y - width / 2;
            int bottom = center_y + width / 2;

            // Update map with room walls
            for (int x = left; x <= right; x++) {
                map[x][top] = '_';    // top wall
                map[x][bottom] = '_'; // bottom wall
            }
            for (int y = top + 1; y <= bottom; y++) {
                map[left][y] = '|';  // left wall
                map[right][y] = '|'; // right wall
            }

            // Update map with room floor
            for (int x = left + 1; x < right; x++) {
                for (int y = top + 1; y < bottom; y++) {
                    map[x][y] = '*';
                }
            }

            // Add the room to the rooms array with a unique type
            rooms[*num_rooms].center_x = center_x;
            rooms[*num_rooms].center_y = center_y;
            rooms[*num_rooms].width = width;
            rooms[*num_rooms].length = length;
            rooms[*num_rooms].type = 1; // Assign a different type (e.g., 2)
            (*num_rooms)++;
        }
    } while (intersection);  // Repeat if there's an intersection
}

void treasure(char map[MAP_SIZEx][MAP_SIZEy], Room *rooms, int *num_rooms) {
    int count  = 0;
    while (count < 30) {
        int x = get_random_number(1, MAP_SIZEx - 1);
        int y = get_random_number(1, MAP_SIZEy - 1);
        if (map[x][y] == '.' || map[x][y] == ',' || map[x][y] == '~') {
            map[x][y] = '*';
            count++;
        }
    }
}

void unvisible_doors(char map[MAP_SIZEx][MAP_SIZEy], char unvisible_door[MAP_SIZEx][MAP_SIZEy], Room *rooms, int *num_rooms) {
    int num_unvis_doors = 3;
    int n = 0;
    for (int y = 0; y < MAP_SIZEy; y++) {
        for (int x = 0; x < MAP_SIZEx; x++) {
            if (map[y][x] == '+') {
                if (n < num_unvis_doors) {
                    unvisible_door[y][x] = 1;

                    if (map[y - 1][x] == '_' || map[y + 1][x] == '_') {
                        map[y][x] = '_';
                        n++;
                    } else if (map[y][x - 1] == '|' || map[y][x + 1] == '|') {
                        map[y][x] = '|';
                        n++;
                    }
                }
            }
        }
    }
}

void window_builder(char map[MAP_SIZEx][MAP_SIZEy], Room *room) {
    room->num_windows = rand() % MAX_ELEMENTS;

    int left = room->center_x - room->length / 2;
    int right = room->center_x + room->length / 2;
    int top = room->center_y - room->width / 2;
    int bottom = room->center_y + room->width / 2;

    for (int n = 0; n < room->num_windows; n++) {
        if (random_10()) { // for horizontal walls
            int x = get_random_number(left, right);
            if (random_10()) { // top wall
                if (map[x][top] == '|' || map[x][top] == '_') { // replace walls only
                    map[x][top] = '=';
                    room->windows[n][0] = x;
                    room->windows[n][1] = top;
                }
            } else { // bottom wall
                if (map[x][bottom] == '|' || map[x][bottom] == '_') {
                    map[x][bottom] = '=';
                    room->windows[n][0] = x;
                    room->windows[n][1] = bottom;
                }
            }
        } else { // for vertical walls
            int y = get_random_number(top, bottom);
            if (random_10()) { // left wall
                if (map[left][y] == '|' || map[left][y] == '_') {
                    map[left][y] = '=';
                    room->windows[n][0] = left;
                    room->windows[n][1] = y;
                }
            } else { // right wall
                if (map[right][y] == '|' || map[right][y] == '_') {
                    map[right][y] = '=';
                    room->windows[n][0] = right;
                    room->windows[n][1] = y;
                }
            }
        }
    }
}

void pillar_builder(char map[MAP_SIZEx][MAP_SIZEy], Room *room) {
    room->num_pillars = get_random_number(1, MAX_PILLARS_IN_EACH_ROOM);

    int left = room->center_x - room->length / 2;
    int right = room->center_x + room->length / 2;
    int top = room->center_y - room->width / 2;
    int bottom = room->center_y + room->width / 2;

    int placed_pillars = 0;
    int max_attempts = 100;
    while (placed_pillars < room->num_pillars && max_attempts > 0) {
        max_attempts--;
        int x = get_random_number(left + 1, right - 1);
        int y = get_random_number(top + 1, bottom - 1);

        if (map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
            map[x][y] = 'O';
            room->pillars[placed_pillars][0] = x;
            room->pillars[placed_pillars][1] = y;
            placed_pillars++;
        }
    }
}

void staircase_builder(char map[MAP_SIZEx][MAP_SIZEy], bool* left_placed) {
    if (!(*left_placed)) {
        int x = get_random_number(1, MAP_SIZEx - 1);
        int y = get_random_number(1, MAP_SIZEy - 1);

        if (map[y][x] == '.' || map[y][x] == '-' || map[y][x] == '~' || map[y][x] == ',') {
            map[y][x] = '>';
            *left_placed = true;
        } else {
            staircase_builder(map, left_placed);
        }
    }
}

void path_and_door_builder(char map[MAP_SIZEx][MAP_SIZEy], Room *rooms, int *num_rooms) {
    const int MAX_ITERATIONS = 1000;

    for (int i = 0; i < *num_rooms - 1; i++) {
        if (rooms[i].length <= 0 || rooms[i].width <= 0 || rooms[i + 1].length <= 0 || rooms[i + 1].width <= 0) {
            continue;
        }

        int x1_pos = get_random_number(rooms[i].center_x - rooms[i].length / 2, rooms[i].center_x + rooms[i].length / 2);
        int y1_pos = get_random_number(rooms[i].center_y - rooms[i].width / 2, rooms[i].center_y + rooms[i].width / 2);
        int x2_pos = get_random_number(rooms[i + 1].center_x - rooms[i + 1].length / 2, rooms[i + 1].center_x + rooms[i + 1].length / 2);
        int y2_pos = get_random_number(rooms[i + 1].center_y - rooms[i + 1].width / 2, rooms[i + 1].center_y + rooms[i + 1].width / 2);

        if (x1_pos == rooms[i].center_x - rooms[i].length / 2 || x1_pos == rooms[i].center_x + rooms[i].length / 2) {
            y1_pos = get_random_number(rooms[i].center_y - rooms[i].width / 2, rooms[i].center_y + rooms[i].width / 2);
        } else {
            x1_pos = get_random_number(rooms[i].center_x - rooms[i].length / 2, rooms[i].center_x + rooms[i].length / 2);
        }

        if (x2_pos == rooms[i + 1].center_x - rooms[i + 1].length / 2 || x2_pos == rooms[i + 1].center_x + rooms[i + 1].length / 2) {
            y2_pos = get_random_number(rooms[i + 1].center_y - rooms[i + 1].width / 2, rooms[i + 1].center_y + rooms[i + 1].width / 2);
        } else {
            x2_pos = get_random_number(rooms[i + 1].center_x - rooms[i + 1].length / 2, rooms[i + 1].center_x + rooms[i + 1].length / 2);
        }


        map[x1_pos][y1_pos] = '+';
        map[x2_pos][y2_pos] = '+';

        if (rooms[i].num_doors < MAX_DOORS) {
            rooms[i].doors[rooms[i].num_doors][0] = x1_pos;
            rooms[i].doors[rooms[i].num_doors][1] = y1_pos;
            rooms[i].num_doors++;
        }

        if (rooms[i + 1].num_doors < MAX_DOORS) {
            rooms[i + 1].doors[rooms[i + 1].num_doors][0] = x2_pos;
            rooms[i + 1].doors[rooms[i + 1].num_doors][1] = y2_pos;
        }

        int index_x = x1_pos;
        int index_y = y1_pos;
        int iteration_count = 0;

        while ((index_x != x2_pos || index_y != y2_pos) && iteration_count < MAX_ITERATIONS) {
            iteration_count++;

            if (index_x != x2_pos) {
                if (index_y != y2_pos) {
                    index_y += (index_y < y2_pos) ? 1 : -1;
                } else {
                    index_x += (index_x < x2_pos) ? 1 : -1;
                }
            } else if (index_y != y2_pos) {
                index_y += (index_y < y2_pos) ? 1 : -1;
            }

            if (map[index_x][index_y] == ' ') {
                map[index_x][index_y] = '#';
            } else if (map[index_x][index_y] == '|' || map[index_x][index_y] == '_' || map[index_x][index_y] == '=') {
                map[index_x][index_y] = '+';
            }
        }
    }
}


void fix_map(char map[MAP_SIZEx][MAP_SIZEy]) {
    // First loop to fix the '+' to '.'
    for (int i = 0; i < MAP_SIZEx - 1; i++) {
        for (int j = 0; j < MAP_SIZEy - 1; j++) {
            if ((map[i][j] == '.' || map[i][j] == '-' || map[i][j] == '~' || map[i][j] == ',') && map[i][j + 1] == '+' && (map[i][j+2] == '.' || map[i][j+2] == '-' || map[i][j+2] == '~' || map[i][j+2] == ',')) {
                map[i][j + 1] = '.';  // Mark the position as '.'
            } else if ((map[i][j] == '.' || map[i][j] == '-' || map[i][j] == '~' || map[i][j] == ',') && map[i + 1][j] == '+' && (map[i+2][j] == '.' || map[i+2][j] == '-' || map[i+2][j] == '~' || map[i+2][j] == ',')) {
                map[i + 1][j] = '.';
            } else if ((map[i][j] == '.' || map[i][j] == '-' || map[i][j] == '~' || map[i][j] == ',') && map[i][j + 1] == '+' && map[i][j + 2] == '|') {
                map[i][j + 1] = '.';
            } else if ((map[i][j] == '.' || map[i][j] == '-' || map[i][j] == '~' || map[i][j] == ',') && map[i + 1][j] == '+' && map[i + 2][j] == '_') {
                map[i + 1][j] = '.';
            }
        }
    }

    //replace unconnected '+' with '=' (window)
    for (int i = 1; i < MAP_SIZEx- 1; i++) {
        for (int j = 1; j < MAP_SIZEy- 1; j++) {
            if (map[i][j] == '+') {
                if (!(map[i + 1][j] == '#' || map[i][j + 1] == '#' || map[i - 1][j] == '#' || map[i][j - 1] == '#')) {
                    map[i][j] = '=';
                }
            }
        }
    }

    for (int i = 0; i < MAP_SIZEx; i++) {
        for (int j = 0; j < MAP_SIZEy; j++) {
            if (map[i][j] == '=' && map [i + 1][j] == '=' && map [i + 2][j] == '=') {
                map[i + 1][j] = '_';
            } else if (map[i][j] == '=' && map[i][j + 1] == '=' && map[i][j + 2] == '=') {
                map[i][j + 1] = '|';
            }
        }
    }

    // for (int i = 0; i < MAP_SIZEx; i++) {
    //     for (int j = 0;  j < MAP_SIZEy; j++) {
    //         if (map[i][j] == '+') {
    //             if (map[i - 1][j] == '#' && map[i + 1][j] == ' ' || ) {

    //             } else if (map[i - 1][j] == ' ' || map[i][j - 1] == '#' || map[i + 1][j] == ' ' || map[i][j + 1] == ' ') {
    //                 map[i][j] == ' ';
    //             } else if (map[i - 1][j] == ' ' || map[i][j - 1] == ' ' || map[i + 1][j] == '#' || map[i][j + 1] == ' ') {
    //                 map[i][j] == ' ';
    //             } else if (map[i - 1][j] == ' ' || map[i][j - 1] == ' ' || map[i + 1][j] == ' ' || map[i][j + 1] == '#') {
    //                 map[i][j] == ' ';
    //             } else {
    //                 continue;
    //             }
    //         }
    //     }
    // }
}   

void food(char map[MAP_SIZEx][MAP_SIZEy]) {
    int num_food = get_random_number(MIN_FOOD_IN_EACH_FLOOR, MAX_FOOD_IN_EACH_FLOOR);
    int i = 0;
    char food[3] = {'f', 'a', 'j'};

    while (i < num_food) {
        int x = get_random_number(1, MAP_SIZEx - 1);
        int y = get_random_number(1, MAP_SIZEy - 1);
        int n = get_random_number(0, 2);

        if (map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
            map[x][y] = food[n]; // food
            i++;
        }
    }
}

void gold_y(char map[MAP_SIZEx][MAP_SIZEy]) { //yellow gold
    int num_gold = get_random_number(MIN_FOOD_IN_EACH_FLOOR, MAX_FOOD_IN_EACH_FLOOR);
    int i = 0;

    while (i < num_gold) {
        int x = get_random_number(1, MAP_SIZEx - 1);
        int y = get_random_number(1, MAP_SIZEy - 1);

        if (map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
            map[x][y] = '$'; // gold
            i++;
        }
    }
}

void gold_b(char map[MAP_SIZEx][MAP_SIZEy]) {
    int num_gold = get_random_number(MIN_FOOD_IN_EACH_FLOOR / 2, MAX_FOOD_IN_EACH_FLOOR / 2); // less abundant
    int i = 0;

    while (i < num_gold) {
        int x = get_random_number(1, MAP_SIZEx - 2);
        int y = get_random_number(1, MAP_SIZEy - 2);

        // Ensure gold is placed inside rooms ('.', '-', '~', ',')
        if (map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
            map[x][y] = 'g'; // gold
            i++;
        }
    }
}

void weapon(char map[MAP_SIZEx][MAP_SIZEy]) {
    int num_weapons = get_random_number(MIN_WEAPONS_IN_EACH_FLOOR, MAX_WEAPONS_IN_EACH_FLOOR);
    int i = 0;

    while (i < num_weapons) {
        int x = get_random_number(1, MAP_SIZEx - 2);
        int y = get_random_number(1, MAP_SIZEy - 2);

        if (map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
            int choice = get_random_number(0, 4); // Select between different weapons

            switch (choice) {
                case 0:
                    // Mace,
                    break;
                case 1:
                    map[x][y] = '!'; // Dagger
                    break;
                case 2:
                    map[x][y] = '\\'; // Magic Wand
                    break;
                case 3:
                    map[x][y] = ')'; // Normal Arrow
                    break;
                case 4:
                    map[x][y] = '('; // Sword
                    break;
            }
            i++;
        }
    }
}

int find_room(int x, int y, char map[MAP_SIZEx][MAP_SIZEy], int *num_rooms, Room *rooms) {
    for (int i = 0; i < *num_rooms; i++) {
        if (x >= rooms[i].center_x - rooms[i].length / 2 &&
            x <= rooms[i].center_x + rooms[i].length / 2 &&
            y >= rooms[i].center_y - rooms[i].width / 2 &&
            y <= rooms[i].center_y + rooms[i].width / 2) {
            return i;
        }
    }
    return -1;
}

void spell (char map[MAP_SIZEx][MAP_SIZEy], Room *rooms,  int *num_rooms) {
    int num_spells = get_random_number(1, MAX_WEAPONS_IN_EACH_FLOOR);
    char choices[] = "hds"; //health, damage, speed
    int i = 0, j = 0;

    while (i < num_spells) {
        int choice = get_random_number(0, 2); //to choose type of spell
        int x = get_random_number(1, MAP_SIZEx - 1);
        int y = get_random_number(1, MAP_SIZEy - 1);

        if (map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
            map[x][y] = choices[choice];
            i++;
        }
    }

    int num = 0;
    while (j < num_spells * 2) {
        int x = get_random_number(1, MAP_SIZEx - 1);
        int y = get_random_number(1, MAP_SIZEy - 1);
        for (int n = 0; n < *num_rooms; n++) {
            if (rooms[n].type == 2) {
                int choice = get_random_number(0, 2);
                if (map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
                map[x][y] = choices[choice];
                j++;
                }
            }
        }
    }

    int n = 0;
    for (int i = 0; i < *num_rooms; i++) {
        if (rooms[i].type == 2) {
            int n_spell = get_random_number(3, 10);
            int n = 0;
            while (n < n_spell) {
                int x = get_random_number(1, MAP_SIZEx - 1);
                int y = get_random_number(1, MAP_SIZEy - 1);
                int choice = get_random_number(0, 2);
                int curr_room = find_room(x, y, map, num_rooms, rooms);
                if (curr_room == i) {
                    map[x][y] = choices[choice];
                    n++;
                }
            }
        }
    }
}

void password_doors(char map[MAP_SIZEx][MAP_SIZEy], int *num_rooms, Room *rooms) {
    int num_pass_doors = get_random_number(5, MAX_NUMBER_PASSWORD_DOORS);
    int i = 0;

    while(i < num_pass_doors) {
        int x = get_random_number(1, MAP_SIZEx - 1);
        int y = get_random_number(1, MAP_SIZEy - 1);

        if (map[x][y] == '+') {
            map[x][y] = '@';

            int room_number = find_room(x, y, map, num_rooms, rooms);
            int corner = get_random_number(1, 4);
            i++;

            switch (corner) {
                case 1: // Top-right corner
                    map[rooms[room_number].center_x + rooms[room_number].length / 2 - 1][rooms[room_number].center_y + rooms[room_number].width / 2 - 1] = '&';
                    break;
                case 2: // Top-left corner
                    map[rooms[room_number].center_x - rooms[room_number].length / 2 + 1][rooms[room_number].center_y + rooms[room_number].width / 2 - 1] = '&';
                    break;
                case 3: // Bottom-right corner
                    map[rooms[room_number].center_x + rooms[room_number].length / 2 - 1][rooms[room_number].center_y - rooms[room_number].width / 2 + 1] = '&';
                    break;
                case 4: // Bottom-left corner
                    map[rooms[room_number].center_x - rooms[room_number].length / 2 + 1][rooms[room_number].center_y - rooms[room_number].width / 2 + 1] = '&';
                    break;
            }
        }
    }
}

void master_key(char map[MAP_SIZEx][MAP_SIZEy]) {
    int x = get_random_number(1, MAP_SIZEx - 1);
    int y = get_random_number(1, MAP_SIZEy - 1);

    if (map[x][y] == '#' || map[x][y] == '.' || map[x][y] == '-' || map[x][y] == '~' || map[x][y] == ',') {
        map[x][y] = 'K';
    }
}

void demons_location (char map[MAP_SIZEx][MAP_SIZEy], int i, Room *rooms) {
    char dem_type[5] = {'D', 'F', 'G', 'S', 'U'};

    rooms[i].demon_x = get_random_number(rooms[i].center_x - rooms[i].length / 2 + 1, rooms[i].center_x + rooms[i].length / 2 - 1);
    rooms[i].demon_y = get_random_number(rooms[i].center_y - rooms[i].width / 2 + 1, rooms[i].center_y + rooms[i].width / 2 - 1);

    rooms[i].demon_type = get_random_number(0, 4);
    map[rooms[i].demon_x][rooms[i].demon_y] = dem_type[rooms[i].demon_type];
}

void save_rooms(const char *filename, Room rooms[], int *num_rooms, char traps[MAP_SIZEx][MAP_SIZEy], char unvisible_door[MAP_SIZEx][MAP_SIZEy]) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < *num_rooms; i++) {
        fprintf(file, "Room %d:\n", i + 1);
        fprintf(file, "Center: (%d, %d)\n", rooms[i].center_x, rooms[i].center_y);
        fprintf(file, "Dimensions: %dx%d\n", rooms[i].width, rooms[i].length);
        fprintf(file, "Type: %d\n", rooms[i].type);

        fprintf(file, "Doors:");
        for (int j = 0; j < rooms[i].num_doors; j++) {
            fprintf(file, " (%d, %d)", rooms[i].doors[j][0], rooms[i].doors[j][1]);
        }
        fprintf(file, "\n");

        fprintf(file, "Pillars:");
        for (int j = 0; j < rooms[i].num_pillars; j++) {
            fprintf(file, " (%d, %d)", rooms[i].pillars[j][0], rooms[i].pillars[j][1]);
        }
        fprintf(file, "\n");

        fprintf(file, "Windows:");
        for (int j = 0; j < rooms[i].num_windows; j++) {
            fprintf(file, " (%d, %d)", rooms[i].windows[j][0], rooms[i].windows[j][1]);
        }
        fprintf(file, "\n");
        fprintf(file, "Visibility: 0\n");
        fprintf(file, "Demon: %d (%d, %d)\n", rooms[i].demon_type, rooms[i].demon_x, rooms[i].demon_y);

        fprintf(file, "\n");
    }

    fprintf(file, "\nUnvisible Doors:");
    for (int y = 0; y < MAP_SIZEx; y++) {
        for (int x = 0; x < MAP_SIZEy; x++) {
            if (unvisible_door[x][y] == 1) {
                fprintf(file, " (%d, %d)", y, x);
            }
        }
    }
    
    fclose(file);
}

void room2_builder(Room *rooms, char map[MAP_SIZEx][MAP_SIZEy]) {
    rooms[0].center_x = MAP_SIZEx / 2;
    rooms[0].center_y = MAP_SIZEy / 2;
    rooms[0].width = 130;
    rooms[0].length = 30; 

    int left = rooms[0].center_x - rooms[0].width/ 2;
    int right = rooms[0].center_x + rooms[0].width / 2;
    int top = rooms[0].center_y - rooms[0].length / 2;
    int bottom = rooms[0].center_y + rooms[0].length / 2;
    rooms[0].type = 1;
    for (int x = left; x <= right; x++) {
        map[x][top] = '_';    // top wall
        map[x][bottom] = '_'; // bottom wall
    }
    for (int y = top + 1; y <= bottom; y++) {
        map[left][y] = '|';  // left wall
        map[right][y] = '|'; // right wall
    }

    int num_golds_in_treasure_room = get_random_number(20, MAX_GOLDS_IN_TREASURE_ROOM);
    int num_traps_in_treasure_room = get_random_number(1, MAX_TRAPS_IN_TREASURE_ROOM);
    int num_demons_in_treasure_room = get_random_number(1, MAX_DEMONS_IN_TREASURE_ROOM);
    int n_gold = 0, n_trap = 0, n_demon = 0;

    while (n_gold < num_golds_in_treasure_room) {
        int x = get_random_number(left, right);
        int y = get_random_number(top, bottom);
        if (map[x][y] == ' ') {
            map[x][y] = '$';
            n_gold++;
        }
    }
    while (n_trap < num_traps_in_treasure_room) {
        int x = get_random_number(left, right);
        int y = get_random_number(top, bottom);
        if (map[x][y] == ' ') {
            map[x][y] = '^';
            n_trap++;
        }
    }
    while (n_demon < num_demons_in_treasure_room) {
        int x = get_random_number(left, right);
        int y = get_random_number(top, bottom);
        if (map[x][y] == ' ') {
            char demon[] = {'D', 'F', 'G', 'S', 'U'};
            int type = get_random_number(0, 4);
            map[x][y] = demon[type];
            n_demon++;
        }
    }
}

void treasure_room_builder() {
    Room room[1];
    char filename[50];
    char treasure_room[MAP_SIZEx][MAP_SIZEy];
    memset(room, 0, sizeof(room));
    for (int i = 0; i < MAP_SIZEx; i++) {
        for (int j = 0; j < MAP_SIZEy; j++) {
            treasure_room[i][j] = ' ';
        }
    }

    room2_builder(room, treasure_room);

    sprintf(filename, "Treasure_Room.txt");
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        for (int y = 0; y < MAP_SIZEy; y++) {
            for (int x = 0; x < MAP_SIZEx; x++) {
                fprintf(file, "%c", treasure_room[x][y]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    }
}

void floor_builder(int floor) {
    char map[MAP_SIZEx][MAP_SIZEy];
    char traps[MAP_SIZEx][MAP_SIZEy];
    char unvisible_door[MAP_SIZEx][MAP_SIZEy];
    for (int i = 0; i < MAP_SIZEx; i++) {
        for (int j = 0; j < MAP_SIZEy; j++) {
            unvisible_door[i][j] = 0;
            traps[i][j] = 0;
        }
    }
    Room rooms[MAX_ROOMS]= {0};
    char filename[20];
    char infofile[20];
    int num_rooms = 0;

    //initialize the map with empty spaces
    for (int i = 0; i < MAP_SIZEx; i++) {
        for (int j = 0; j < MAP_SIZEy; j++) {
            map[i][j] = ' ';
        }
    }

    room_builder(rooms, &num_rooms, map);
    if (floor == 4) {
        treasure_room(rooms, &num_rooms, map);
        treasure(map, rooms, &num_rooms);
    }

    for (int i = 0; i < num_rooms; i++) {
        window_builder(map, &rooms[i]);
        pillar_builder(map, &rooms[i]);
        demons_location(map, i, rooms);
    }
    
    path_and_door_builder(map, rooms, &num_rooms);

    fix_map(map);
    food(map);
    gold_y(map);
    gold_b(map);
    weapon(map);
    spell(map, rooms, &num_rooms);

    bool left_placed = false;

    staircase_builder(map, &left_placed);

    password_doors(map, &num_rooms, rooms);
    for (int i = 0;  i < 20 ; i++) {
        master_key(map);
    }
    unvisible_doors(map, unvisible_door, rooms, &num_rooms);

    sprintf(filename, "map%d.txt", floor);
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        for (int y = 0; y < MAP_SIZEy; y++) {
            for (int x = 0; x < MAP_SIZEx; x++) {
                fprintf(file, "%c", map[x][y]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    }

    //correctly call save_rooms function
    sprintf(infofile, "rooms_info_%d.txt", floor);
    save_rooms(infofile, rooms, &num_rooms, traps, unvisible_door);
}


int main() {
    setlocale(LC_ALL, "");
    srand(time(NULL));
    int floor = 0;

    floor_builder(1);
    floor_builder(2);
    //floor_builder(3);
    //floor_builder(4);
    treasure_room_builder();

    return 0;
}
