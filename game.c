#include <stdint.h>

#include "raylib.h"
#include "map.h"

const Color MapTileTypeColor[] = {
    RED, // Invalid room type
    DARKPURPLE,
    DARKBROWN,
    GREEN,
    ORANGE,
    BLUE,
};

void draw(void) {
    BeginDrawing();

    for(uint16_t i=0; i<MAP_GRID_X; ++i) {
        for (uint16_t j=0; j<MAP_GRID_Y; ++j) {
            if (Map[i][j].type == 0) continue; // Todo(Manolis): Invalid type
            DrawRectangleRec(Map[i][j].rec, MapTileTypeColor[Map[i][j].type]);
            DrawRectangleLinesEx(Map[i][j].rec, 1.0f, BLACK);
            #define DRAW_PARAMS Map[i][j].rec.x+2, Map[i][j].rec.y+1, 7, WHITE
            if (Map[i][j].type == kDebugId) DrawText(TextFormat("%d", Map[i][j].room_id), DRAW_PARAMS);
            switch (Map[i][j].direction) { 
            case kNorth:
                DrawText("N", DRAW_PARAMS);
                break;
            case kSouth:
                DrawText("S", DRAW_PARAMS);
                break;
            case kWest:
                DrawText("W", DRAW_PARAMS);
                break;
            case kEast:
                DrawText("E", DRAW_PARAMS);
                break;
            case kNorthEast:
                DrawText("NE", DRAW_PARAMS);
                break;
            case kSouthEast:
                DrawText("SE", DRAW_PARAMS);
                break;
            case kNorthWest:
                DrawText("NW", DRAW_PARAMS);
                break;
            case kSouthWest:
                DrawText("SW", DRAW_PARAMS);
                break;
            }
        }
    }
    // // TODO(Manolis): Debug room id
    // DrawText(TextFormat("%d",Map[i][j].id), Map[i][j].rec.x+2, Map[i][j].rec.y+1, 7, RED);

    EndDrawing();
}

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Rogue-like game");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_DEBUG);
    
    GenerateRandomMap();

    while (!WindowShouldClose()) {
        draw();
    }

    CloseWindow();

    return 0;
}
