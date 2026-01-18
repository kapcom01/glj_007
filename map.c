#include <stdint.h>

#include "raylib.h"
#include "map.h"

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

MapTile Map[MAP_GRID_X][MAP_GRID_Y] = {0};

typedef struct {
    uint16_t x_in_tiles;
    uint16_t y_in_tiles;
} Door;

typedef struct  {
    Rectangle mask;
    Door north_door;
    Door south_door;
    Door east_door;
    Door west_door;
    uint16_t x_in_tiles;
    uint16_t y_in_tiles;
    uint8_t width_in_tiles;
    uint8_t height_in_tiles;
} Room;

Room rooms[5] = {0};

const uint8_t room_mask_offset_in_tiles = 6; // only even number
const uint8_t room_min_width_in_tiles   = 5;
const uint8_t room_min_height_in_tiles  = 5;
const uint8_t room_max_width_in_tiles   = 14;
const uint8_t room_max_height_in_tiles  = 12;

void initialize_tiles(void) {
    for(uint16_t i=0; i<MAP_GRID_X; ++i) {
        for (uint16_t j=0; j<MAP_GRID_Y; ++j) {
            Map[i][j].rec.x      = (float) MAP_TILE_SIZE * i;
            Map[i][j].rec.y      = (float) MAP_TILE_SIZE * j;
            Map[i][j].rec.width  = (float) MAP_TILE_SIZE;
            Map[i][j].rec.height = (float) MAP_TILE_SIZE;
            Map[i][j].type       = 0;
            Map[i][j].room_id    = 0;
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
            Map[i][j].room_id   = id;
        }
    }
    Map[x_start+1][y_start+1].type = kDebugId;
}

uint16_t generate_rooms(void) {
    int debug_collisions = 0;
    uint16_t n=0;
    for (n=0; n<ARRAY_SIZE(rooms);) {
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
    return n;
}

TileDirection get_relative_direction(const uint16_t room_src, const uint16_t room_dst) {
    const uint16_t src_x = rooms[room_src].x_in_tiles;
    const uint16_t src_y = rooms[room_src].y_in_tiles;
    const uint16_t dst_x = rooms[room_dst].x_in_tiles;
    const uint16_t dst_y = rooms[room_dst].y_in_tiles;
    
    const uint8_t  src_h = rooms[room_src].height_in_tiles;
    const uint8_t  src_w = rooms[room_src].width_in_tiles;
    const uint8_t  dst_h = rooms[room_dst].height_in_tiles;
    const uint8_t  dst_w = rooms[room_dst].width_in_tiles;

    TileDirection direction = 0;

    // src_w > dst_w
    if ( (src_x >= dst_x && src_x <= dst_x+dst_w)
        || (src_x+src_w >= dst_x && src_x+src_w <= dst_x+dst_w) ) {
        if (src_y <= dst_y) {
            return kSouth;
        }
        else {
            return kNorth;
        }
    }

    // src_w < dst_w
    if ( (dst_x >= src_x && dst_x <= src_x+src_w)
        || (dst_x+dst_w >= src_x && dst_x+dst_w <= src_x+src_w) ) {
        if (src_y <= dst_y) {
            return kSouth;
        }
        else {
            return kNorth;
        }
    }
    
    // src_h > dst_h
    if ( (src_y >= dst_y && src_y <= dst_y+dst_h)
        || (src_y+src_h >= dst_y && src_y+src_h <= dst_y+dst_h) ) {
        if (src_x <= dst_x) {
            return kEast;
        }
        else {
            return kWest;
        }
    }

    // src_h < dst_h
    if ( (dst_y >= src_y && dst_y <= src_y+src_h)
        || (dst_y+dst_h >= src_y && dst_y+dst_h <= src_y+src_h) ) {
        if (src_x <= dst_x) {
            return kEast;
        }
        else {
            return kWest;
        }
    }

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

TileDirection generate_random_door(const uint16_t room_src, TileDirection direction) {
    const uint16_t x      = rooms[room_src].x_in_tiles;
    const uint16_t y      = rooms[room_src].y_in_tiles;
    const uint16_t width  = rooms[room_src].width_in_tiles;
    const uint16_t height = rooms[room_src].height_in_tiles;

    uint16_t door_x, door_y;

    switch (direction) {
    case kNorth:
    case kNorthWest:
        if ( rooms[room_src].north_door.x_in_tiles ) return kNorth;
        door_x = x + GetRandomValue(0, width-3) + 1;
        door_y = y;
        break;
    case kSouth:
    case kSouthEast:
        if ( rooms[room_src].south_door.x_in_tiles ) return kSouth;
        door_x = x + GetRandomValue(0, width-3) + 1;
        door_y = y + height-1;
        break;
    case kEast:
    case kNorthEast:
        if ( rooms[room_src].east_door.x_in_tiles ) return kEast;
        // door_x = x + GetRandomValue(0, width-3) + 1;
        door_x = x + width-1;
        door_y = y + GetRandomValue(0, height-3) + 1;
        break;
    case kWest:
    case kSouthWest:
        if ( rooms[room_src].west_door.x_in_tiles ) return kWest;
        // door_x = x + GetRandomValue(0, width-3) + 1;
        door_x = x;
        door_y = y + GetRandomValue(0, height-3) + 1;
    }

    switch (direction) {
    case kNorth:
    case kNorthWest:
        rooms[room_src].north_door.x_in_tiles = door_x;
        rooms[room_src].north_door.y_in_tiles = door_y;
        break;
    case kSouth:
    case kSouthEast:
        rooms[room_src].south_door.x_in_tiles = door_x;
        rooms[room_src].south_door.y_in_tiles = door_y;
        break;
    case kEast:
    case kNorthEast:
        rooms[room_src].east_door.x_in_tiles = door_x;
        rooms[room_src].east_door.y_in_tiles = door_y;
        break;
    case kWest:
    case kSouthWest:
        rooms[room_src].west_door.x_in_tiles = door_x;
        rooms[room_src].west_door.y_in_tiles = door_y;
    }

    Map[door_x][door_y].type = kDoor;
    return Map[door_x][door_y].direction;
}

uint16_t set_passage_for_doors(const uint16_t room_src, const uint16_t room_dst) {
    const uint8_t  width = rooms[room_src].width_in_tiles;
    const uint8_t  height = rooms[room_src].height_in_tiles;
    const uint16_t dx = rooms[room_dst].x_in_tiles;
    const uint16_t dy = rooms[room_dst].y_in_tiles;
    const uint8_t  dw = rooms[room_dst].width_in_tiles;
    const uint8_t  dh = rooms[room_dst].height_in_tiles;

    TileDirection door_direction = generate_random_door(
            room_src, get_relative_direction(room_src, room_dst)
        );

    uint16_t x = 0;
    uint16_t y = 0;

    switch (door_direction) {
    case kNorth:
        x = rooms[room_src].north_door.x_in_tiles;
        y = rooms[room_src].north_door.y_in_tiles;
        while (y > dy+dh) {
            if ( Map[x][--y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (x > dx+dw-2) {
            if ( Map[--x][y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (x < dx+1) {
            if ( Map[++x][y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        --y;
        break;
    case kSouth:
        x = rooms[room_src].south_door.x_in_tiles;
        y = rooms[room_src].south_door.y_in_tiles;
        while (y < dy-1) {
            if ( Map[x][++y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (x > dx+dw-2) {
            if ( Map[--x][y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (x < dx+1) {
            if ( Map[++x][y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        ++y;
        break;
    case kEast:
        x = rooms[room_src].east_door.x_in_tiles;
        y = rooms[room_src].east_door.y_in_tiles;
        while (x < dx-1) {
            if ( Map[++x][y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (y > dy+dh-2) {
            if ( Map[x][--y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (y < dy+1) {
            if ( Map[x][++y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        ++x;
        break;
    case kWest:
        x = rooms[room_src].west_door.x_in_tiles;
        y = rooms[room_src].west_door.y_in_tiles;
        while (x > dx+dw) {
            if ( Map[--x][y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (y > dy+dh-2) {
            if ( Map[x][--y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        while (y < dy+1) {
            if ( Map[x][++y].room_id ) goto end_switch;
            Map[x][y].type = kPassage;
        }
        --x;
        break;
    }

    end_switch:

    Map[x][y].type = kDoor;
    switch (Map[x][y].direction) {
    case kNorth:
        rooms[room_dst].north_door.x_in_tiles = x;
        rooms[room_dst].north_door.y_in_tiles = y;
        break;
    case kSouth:
        rooms[room_dst].south_door.x_in_tiles = x;
        rooms[room_dst].south_door.y_in_tiles = y;
        break;
    case kEast:
        rooms[room_dst].east_door.x_in_tiles = x;
        rooms[room_dst].east_door.y_in_tiles = y;
        break;
    case kWest:
        rooms[room_dst].west_door.x_in_tiles = x;
        rooms[room_dst].west_door.y_in_tiles = y;
        break;
    }

    TraceLog(LOG_DEBUG, "passage (%d, %d) -> %d", room_src+1, room_dst+1, Map[x][y].room_id);
    return Map[x][y].room_id-1;
}

void GenerateRandomMap(void) {
    initialize_tiles();
    uint16_t rooms_count = generate_rooms();

    bool rooms_with_passage[ARRAY_SIZE(rooms)] = {false};

    uint16_t connected_rooms = 0;
    rooms_with_passage[0] = true;
    while ( rooms_count != connected_rooms ) {
        static uint16_t src = 0;
        static uint16_t dst = 1;
        src = set_passage_for_doors(src, dst);
        if (src == dst) dst++;

        rooms_with_passage[src] = true;
        connected_rooms = 0;
        for (uint16_t i=0; i<rooms_count; i++) {
            if ( rooms_with_passage[i] == true ) connected_rooms++;
        }
    }
}
