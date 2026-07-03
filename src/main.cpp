#include "raylib.h"
#include "core/entities/player/Player.h"
#include "core/systems/tilemap/Tilemap.h"

int main()
{
    // Window
    InitWindow(1280, 720, "SpenceDew");
    SetTargetFPS(60);

    // Player
    Player player;
    player.init();

    // Tilemap
    Tilemap tilemap = Tilemap::createTestRoom();
    player.walls = &tilemap.walls;

    // Camera follows the player
    Camera2D camera = {};
    camera.zoom = 2.0f; // Pixel-art scale; adjust as needed
    camera.offset = {GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};

    while (!WindowShouldClose())
    {
        const float delta = GetFrameTime();

        // --- Update ---
        player.update(delta);

        // Centre camera on player
        camera.target = player.position;

        // --- Draw ---
        BeginDrawing();
        ClearBackground({30, 30, 30, 255}); // Dark grey world

        BeginMode2D(camera);
        tilemap.draw();

        player.draw();

        // Draw a simple grid for spatial reference during development
        for (int x = -20; x <= 20; x++)
        {
            DrawLine(x * 32, -640, x * 32, 640, {50, 50, 50, 255});
        }
        for (int y = -20; y <= 20; y++)
        {
            DrawLine(-640, y * 32, 640, y * 32, {50, 50, 50, 255});
        }
        EndMode2D();

        // --- HUD ---
        DrawRectangle(10, 10, 200, 18, {0, 0, 0, 120});
        DrawRectangle(10, 10,
                      static_cast<int>(200.0f * (player.currentHealth / player.maxHealth)),
                      18, RED);
        DrawText(TextFormat("HP  %.0f / %.0f", player.currentHealth, player.maxHealth),
                 14, 11, 14, WHITE);

        DrawText(TextFormat("Pos  %.0f, %.0f", player.position.x, player.position.y),
                 10, 36, 14, LIGHTGRAY);

        DrawFPS(10, 56);

        DrawText("WASD Move  |  Z Attack  |  Ctrl Dodge  |  Shift Sprint",
                 10, GetScreenHeight() - 24, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
