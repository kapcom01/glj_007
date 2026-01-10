#include <stdint.h>

#include "raylib.h"
#include "map.h"

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

MapTile Map[MAP_GRID_X][MAP_GRID_Y] = {0};

typedef struct  {
    Rectangle mask;
    uint16_t x_in_tiles;
    uint16_t y_in_tiles;
    uint8_t width_in_tiles;
    uint8_t height_in_tiles;
} Room;

Room rooms[4] = {0};

const uint8_t room_mask_offset_in_tiles = 6; // only even number
const uint8_t room_min_width_in_tiles   = 4;
const uint8_t room_min_height_in_tiles  = 4;
const uint8_t room_max_width_in_tiles   = 12;
const uint8_t room_max_height_in_tiles  = 12;

void initialize_tiles(void) {
    for(uint16_t i=0; i<MAP_GRID_X; ++i) {
        for (uint16_t j=0; j<MAP_GRID_Y; ++j) {
            Map[i][j].rec.x      = (float) MAP_TILE_SIZE * i;
            Map[i][j].rec.y      = (float) MAP_TILE_SIZE * j;
            Map[i][j].rec.width  = (float) MAP_TILE_SIZE;
            Map[i][j].rec.height = (float) MAP_TILE_SIZE;
            Map[i][j].type       = kWall;
            Map[i][j].id         = 0;
            Map[i][j].direction  = 0;
        }
    }
}

void set_room_tiles(Room room, const uint8_t id) {
    const uint16_t x_start = room.x_in_tiles;
    const uint16_t y_start = room.y_in_tiles;
    const uint16_t x_end   = x_start + room.width_in_tiles;
    const uint16_t y_end   = y_start + room.height_in_tiles;

    for(uint16_t i=(x_start); i<(x_end); ++i) {
        for(uint16_t j=(y_start); j<(y_end); ++j) {
            if (i == x_start && j == y_start) {
                Map[i][j].direction = kNorth + kWest;
            }
            else if (i == x_end-1 && j == y_end-1) {
                Map[i][j].direction = kSouth + kEast;
            }
            else if (j == y_start && i == x_end-1) {
                Map[i][j].direction = kNorth + kEast;
            }
            else if (i == x_start && j == y_end-1) {
                Map[i][j].direction = kSouth + kWest;
            }
            else if (i == x_start) {
                Map[i][j].direction = kWest;
            }
            else if (i == x_end-1) {
                Map[i][j].direction = kEast;
            }
            else if (j == y_start) {
                Map[i][j].direction = kNorth;
            }
            else if (j == y_end-1) {
                Map[i][j].direction = kSouth;
            }
            Map[i][j].type = kRoom;
            Map[i][j].id   = id;
        }
    }
    Map[x_start+1][y_start+1].type = kDebugId;
}

void generate_rooms(void) {
    int debug_collisions = 0;
    for (uint16_t n=0; n<ARRAY_SIZE(rooms);) {
        if (debug_collisions > 300) break; // TODO(Manolis): Fix this INFINITE COLLISION BUG

        const uint16_t offset = (uint16_t) room_mask_offset_in_tiles;
        
        Room new_room = {0};
        
        new_room.x_in_tiles      = GetRandomValue(0, MAP_GRID_X) + offset/2;
        new_room.y_in_tiles      = GetRandomValue(0, MAP_GRID_Y) + offset/2;
        new_room.width_in_tiles  = GetRandomValue(room_min_width_in_tiles, room_max_width_in_tiles);
        new_room.height_in_tiles = GetRandomValue(room_min_height_in_tiles, room_max_height_in_tiles);

        new_room.mask.x      = (float) ((new_room.x_in_tiles      - offset/2) * MAP_TILE_SIZE);
        new_room.mask.y      = (float) ((new_room.y_in_tiles      - offset/2) * MAP_TILE_SIZE);
        new_room.mask.width  = (float) ((new_room.width_in_tiles  + offset)   * MAP_TILE_SIZE);
        new_room.mask.height = (float) ((new_room.height_in_tiles + offset)   * MAP_TILE_SIZE);

        // TODO(Manolis): Find a better solution to avoid collisions
        bool collision = false;
        for (uint16_t r = n; r > 0; --r) {
            if (CheckCollisionRecs(new_room.mask, rooms[r-1].mask)) {
                collision = true;
                debug_collisions++;
                break;
            }
        }
        if (new_room.mask.x + new_room.mask.width > WINDOW_WIDTH
                || new_room.mask.y + new_room.mask.height > WINDOW_HEIGHT) {
            collision = true;
            debug_collisions++;
        }
        if (collision) continue;
        rooms[n] = new_room;
        TraceLog(LOG_DEBUG, "ROOM %d collisions: %d, (x:%d, y:%d), width: %d, height: %d",
            n+1,
            debug_collisions,
            rooms[n].x_in_tiles,
            rooms[n].y_in_tiles,
            rooms[n].width_in_tiles,
            rooms[n].height_in_tiles);
        set_room_tiles(rooms[n], n+1);
        n++;
        debug_collisions = 0;
    }
}

TileDirection get_relative_direction(const uint16_t room_src, const uint16_t room_dst) {
    const uint16_t src_x = rooms[room_src].x_in_tiles;
    const uint16_t src_y = rooms[room_src].y_in_tiles;
    const uint16_t dst_x = rooms[room_dst].x_in_tiles;
    const uint16_t dst_y = rooms[room_dst].y_in_tiles;

    TileDirection direction = 0;

    if (src_y < dst_y) {
        direction += kSouth;
    }
    else if (src_y > dst_y) {
        direction += kNorth;
    }
    
    if (src_x < dst_x) {
        direction += kEast;
    }
    else if (src_x > dst_x) {
        direction += kWest;
    }

    return direction;
}

void generate_random_door(const uint16_t room_src, TileDirection direction) {
    const uint16_t x      = rooms[room_src].x_in_tiles;
    const uint16_t y      = rooms[room_src].y_in_tiles;
    const uint16_t width  = rooms[room_src].width_in_tiles;
    const uint16_t height = rooms[room_src].height_in_tiles;

    uint16_t door_x, door_y;

    switch (direction) {
    case kNorth:
    case kNorthWest:
        door_x = x + GetRandomValue(0, width-3) + 1;
        door_y = y;
        break;
    case kSouth:
    case kSouthEast:
        door_x = x + GetRandomValue(0, width-3) + 1;
        door_y = y + height-1;
        break;
    case kEast:
    case kNorthEast:
        door_x = x + width-1;
        door_y = y + GetRandomValue(0, height-3) + 1;
        break;
    case kWest:
    case kSouthWest:
        door_x = x;
        door_y = y + GetRandomValue(0, height-3) + 1;
    }

    // char *dir_str = {0};
    // switch (direction) {
    // case kNorth:
    //     dir_str = "North"; break;
    // case kSouth:
    //     dir_str = "South"; break;
    // case kEast:
    //     dir_str = "East"; break;
    // case kWest:
    //     dir_str = "West"; break;
    // case kNorthEast:
    //     dir_str = "NorthEast"; break;
    // case kNorthWest:
    //     dir_str = "NorthWest"; break;
    // case kSouthEast:
    //     dir_str = "SouthEast"; break;
    // case kSouthWest:
    //     dir_str = "SouthWest"; break;
    // }
    // TraceLog(LOG_DEBUG, "Door for room %d is %s (x:%d, y:%d)", room_src+1, dir_str, door_x, door_y);

    Map[door_x][door_y].type = kDoor;
}

void generate_passage(const uint16_t room_src, const uint16_t room_dst) {
    generate_random_door(room_src, get_relative_direction(room_src, room_dst));

}

void GenerateRandomMap(void) {
    initialize_tiles();
    generate_rooms();
    for (uint16_t n=0; n<ARRAY_SIZE(rooms)-1; ++n) {
        if(rooms[n].width_in_tiles == 0
            || rooms[n+1].width_in_tiles == 0) {
            break;
        }
        generate_passage(n, n+1);
    }
}
