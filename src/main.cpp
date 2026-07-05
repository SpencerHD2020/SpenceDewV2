#include "raylib.h"
#include "core/entities/player/Player.h"
#include "core/entities/enemies/Enemy.h"
#include "core/systems/tilemap/Tilemap.h"
#include "core/systems/tilemap/LdtkLoader.h"
#include "core/systems/combat/Projectile.h"
#include "core/systems/navigation/Pathfinder.h"
#include "core/systems/ui/DamageNumbers.h"
#include <vector>
#include <memory>
#include <algorithm>

// -----------------------------------------------------------------------
// Set to true once you have placed assets/levels/level_01.ldtk
// -----------------------------------------------------------------------
static constexpr bool USE_LDTK = true;

int main()
{
    InitWindow(1280, 720, "SpenceDew");
    SetTargetFPS(60);

    // ----------------------------------------------------------------
    // Level / tilemap
    // ----------------------------------------------------------------
    Tilemap tilemap;
    Vector2 playerSpawn = {0.0f, 0.0f};
    std::vector<Vector2> enemySpawns;

    if (USE_LDTK)
    {
        std::vector<LdtkLevel> levels;
        if (LdtkLoader::load("assets/levels/level_01.ldtk", levels) && !levels.empty())
        {
            tilemap.walls = levels[0].walls;
            // Load the visual tiles. The .ldtk references the tileset via a path
            // outside the project, so we point at the copy shipped in assets/.
            tilemap.loadVisuals(levels[0], "assets/tilesets/tilemap.png");
            for (const auto &ent : levels[0].entities)
            {
                if (ent.identifier == "PlayerSpawn")
                    playerSpawn = ent.position;
                if (ent.identifier == "EnemySpawn")
                    enemySpawns.push_back(ent.position);
            }
        }
    }
    else
    {
        tilemap = Tilemap::createTestRoom();
        enemySpawns = {{-120.0f, -100.0f}, {100.0f, -100.0f}, {-120.0f, 100.0f}};
    }

    // ----------------------------------------------------------------
    // Pathfinder  (built once from the wall list)
    // ----------------------------------------------------------------
    Pathfinder pathfinder;
    {
        // Compute the navigable bounding box from all walls
        float minX = 1e9f, minY = 1e9f;
        float maxX = -1e9f, maxY = -1e9f;
        for (const auto &w : tilemap.walls)
        {
            if (w.x < minX)
                minX = w.x;
            if (w.y < minY)
                minY = w.y;
            if (w.x + w.width > maxX)
                maxX = w.x + w.width;
            if (w.y + w.height > maxY)
                maxY = w.y + w.height;
        }
        if (maxX > minX && maxY > minY)
        {
            pathfinder.buildGrid(tilemap.walls,
                                 {minX, minY, maxX - minX, maxY - minY});
        }
    }

    // ----------------------------------------------------------------
    // Player
    // ----------------------------------------------------------------
    DamageNumbers damageNumbers;

    Player player;
    player.init();
    player.position = playerSpawn;
    player.walls = &tilemap.walls;

    player.onDamageVisual = [&damageNumbers](Vector2 pos, float dmg)
    {
        damageNumbers.spawn(pos, dmg, WHITE);
    };

    // ----------------------------------------------------------------
    // Enemies
    // ----------------------------------------------------------------
    // Stored as unique_ptr so enemies are heap-allocated and never relocated;
    // this keeps fsm.enemy / state->enemy self-pointers permanently valid.
    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.reserve(enemySpawns.size());
    for (const auto &sp : enemySpawns)
    {
        auto &e = enemies.emplace_back(std::make_unique<Enemy>());
        e->init(sp);
        e->walls = &tilemap.walls;
        e->playerPos = &player.position;
        e->pathfinder = &pathfinder;
        e->onDamageVisual = [&damageNumbers](Vector2 pos, float dmg)
        {
            damageNumbers.spawn(pos, dmg, RED);
        };
    }

    // ----------------------------------------------------------------
    // Projectiles
    // ----------------------------------------------------------------
    std::vector<Projectile> projectiles;
    projectiles.reserve(32);

    player.onFireProjectile = [&projectiles, &player](Vector2 pos, Vector2 dir)
    {
        Projectile bolt;
        bolt.init(pos, dir, 420.0f, 20.0f, 230.0f, &player);
        projectiles.push_back(bolt);
    };

    // ----------------------------------------------------------------
    // Camera
    // ----------------------------------------------------------------
    Camera2D camera = {};
    camera.zoom = 2.0f;
    camera.offset = {GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};

    // ================================================================
    // Game loop
    // ================================================================
    while (!WindowShouldClose())
    {
        const float delta = GetFrameTime();

        // ---- Build hurtbox lists for this frame ----
        std::vector<Hurtbox *> enemyHurtboxes;
        enemyHurtboxes.reserve(enemies.size());
        for (auto &e : enemies)
            if (!e->markedForRemoval)
                enemyHurtboxes.push_back(&e->hurtbox);

        // ---- Update player ----
        player.update(delta);

        // ---- Player melee hitbox vs enemy hurtboxes ----
        if (player.meleeHitbox.isActive)
            player.meleeHitbox.checkOverlap(enemyHurtboxes);

        // ---- Update enemies + their hitboxes vs player ----
        std::vector<Hurtbox *> playerHurtbox = {&player.hurtbox};
        for (auto &e : enemies)
        {
            if (e->markedForRemoval)
                continue;
            e->update(delta);
            if (e->meleeHitbox.isActive)
                e->meleeHitbox.checkOverlap(playerHurtbox);
        }

        // ---- Update projectiles vs enemy hurtboxes ----
        for (auto &proj : projectiles)
            proj.update(delta, tilemap.walls, enemyHurtboxes);

        // ---- Remove dead projectiles and enemies ----
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                           [](const Projectile &p)
                           { return !p.active; }),
            projectiles.end());

        enemies.erase(
            std::remove_if(enemies.begin(), enemies.end(),
                           [](const std::unique_ptr<Enemy> &e)
                           { return e->markedForRemoval; }),
            enemies.end());

        // ---- Update damage numbers ----
        damageNumbers.update(delta);

        // ---- Camera ----
        camera.target = player.position;

        // ================================================================
        // Draw
        // ================================================================
        BeginDrawing();
        ClearBackground({30, 30, 30, 255});

        BeginMode2D(camera);

        tilemap.draw();

        // Dev grid (only when no real tiles are loaded, so it doesn't cover them)
        if (tilemap.tiles.empty())
        {
            for (int x = -20; x <= 20; x++)
                DrawLine(x * 32, -640, x * 32, 640, {50, 50, 50, 255});
            for (int y = -20; y <= 20; y++)
                DrawLine(-640, y * 32, 640, y * 32, {50, 50, 50, 255});
        }

        // Draw enemies
        for (const auto &e : enemies)
            e->draw();

        // Draw projectiles
        for (const auto &proj : projectiles)
            proj.draw();

        // Draw player (on top)
        player.draw();

        // Draw damage numbers (world-space)
        damageNumbers.draw();

        EndMode2D();

        // ================================================================
        // HUD
        // ================================================================

        // --- Player health bar ---
        const int barX = 10, barY = 10, barW = 220, barH = 20;
        DrawRectangle(barX, barY, barW, barH, {0, 0, 0, 160});
        int hpW = static_cast<int>(barW * (player.currentHealth / player.maxHealth));
        Color hpColor = (player.currentHealth > player.maxHealth * 0.4f)   ? GREEN
                        : (player.currentHealth > player.maxHealth * 0.2f) ? YELLOW
                                                                           : RED;
        DrawRectangle(barX, barY, hpW, barH, hpColor);
        DrawRectangleLinesEx({(float)barX, (float)barY, (float)barW, (float)barH},
                             1.5f, {200, 200, 200, 200});
        DrawText(TextFormat("HP  %.0f / %.0f",
                            player.currentHealth, player.maxHealth),
                 barX + 6, barY + 3, 14, WHITE);

        // --- I-frame indicator ---
        if (player.iFrameTimer > 0.0f)
            DrawText("I-FRAMES", barX, barY + 26, 12, GOLD);

        // --- Enemy count ---
        DrawText(TextFormat("Enemies: %d", (int)enemies.size()),
                 10, barY + 44, 14, LIGHTGRAY);

        // --- Position ---
        DrawText(TextFormat("Pos  %.0f, %.0f", player.position.x, player.position.y),
                 10, barY + 62, 14, DARKGRAY);

        DrawFPS(10, barY + 80);

        DrawText("WASD Move  |  Z Attack  |  C Cast  |  Ctrl Dodge  |  Shift Sprint",
                 10, GetScreenHeight() - 24, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
