#include "raylib.h"

int main() {
	InitWindow(640, 480, "Rogue-like game");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
