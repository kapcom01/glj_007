#include <stdint.h>
#include "raylib.h"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define MAP_TILE_SIZE 10

typedef enum {
    MAP_TYPE_WALL,
    MAP_TYPE_ROOM,
    MAP_TYPE_PASSAGE_X,
    MAP_TYPE_PASSAGE_Y,
    MAP_TYPE_SIZE
} MapTileType;

const Color MapTileTypeColor[MAP_TYPE_SIZE] = {
    BLACK,
    WHITE,
    GRAY,
    GRAY,
};

typedef struct {
    Rectangle rec;
    MapTileType type;
} MapTile;

MapTile Map[WINDOW_WIDTH/MAP_TILE_SIZE][WINDOW_HEIGHT/MAP_TILE_SIZE] = {0};

MapTileType generate_map_tile_type(uint8_t i, uint8_t j) {
    if (i == 0 || j == 0) return MAP_TYPE_WALL;

    int dice = GetRandomValue(0,9);
    MapTileType top_type  = Map[i-1][j].type;
    MapTileType left_type = Map[i-1][j].type;
    MapTileType ret_type;

    switch (left_type) {
        case MAP_TYPE_WALL:
            if (dice < 6)       ret_type = MAP_TYPE_WALL;
            else if (dice < 7)  ret_type = MAP_TYPE_PASSAGE_X;
            else if (dice < 8)  ret_type = MAP_TYPE_PASSAGE_Y;
            else if (dice < 10) ret_type = MAP_TYPE_ROOM;
    }
    switch (top_type) {
        case MAP_TYPE_PASSAGE_Y:
            if (dice < 6)       ret_type = MAP_TYPE_PASSAGE_Y;
            else if (dice < 9)  ret_type = MAP_TYPE_ROOM;
            else if (dice < 10) ret_type = MAP_TYPE_WALL;
    }
    return ret_type;
}

void generate_map(void) {
    for(uint8_t i=0; i<WINDOW_WIDTH/MAP_TILE_SIZE; ++i) {
        for (uint8_t j=0; j<WINDOW_HEIGHT/MAP_TILE_SIZE; ++j) {
            Map[i][j].rec.x      = (float)i*MAP_TILE_SIZE;
            Map[i][j].rec.y      = (float)j*MAP_TILE_SIZE;
            Map[i][j].rec.width  = MAP_TILE_SIZE;
            Map[i][j].rec.height = MAP_TILE_SIZE;
            Map[i][j].type       = generate_map_tile_type(i, j);
        }
    }
}

void draw(void) {
    BeginDrawing();

    for(uint8_t i=0; i<WINDOW_WIDTH/MAP_TILE_SIZE; ++i) {
        for (uint8_t j=0; j<WINDOW_HEIGHT/MAP_TILE_SIZE; ++j) {
            DrawRectangleRec(Map[i][j].rec, MapTileTypeColor[Map[i][j].type]);
            DrawRectangleLinesEx(Map[i][j].rec, 1.0f, BLACK);
        }
    }

    EndDrawing();
}

int main() {
    InitWindow(640, 480, "Rogue-like game");
    SetTargetFPS(60);
    
    generate_map();

    while (!WindowShouldClose()) {
        draw();
    }

    CloseWindow();

    return 0;
}
