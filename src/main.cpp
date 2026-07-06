#include "raylib.h"
#include "core/entities/player/Player.h"
#include "core/entities/enemies/Enemy.h"
#include "core/entities/friends/Friend.h"
#include "core/entities/friends/PotentialPartner.h"
#include "core/systems/tilemap/Tilemap.h"
#include "core/systems/tilemap/LdtkLoader.h"
#include "core/systems/combat/Projectile.h"
#include "core/systems/navigation/Pathfinder.h"
#include "core/systems/farming/FarmController.h"
#include "core/systems/ui/DamageNumbers.h"
#include "core/systems/ui/DialogueUI.h"
#include "core/systems/dialogue/Dialogue.h"
#include "core/systems/dialogue/DialogueRunner.h"
#include "core/systems/relationships/RelationshipManager.h"
#include "core/Input.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <cfloat>

// -----------------------------------------------------------------------
// Set to true once you have placed assets/levels/level_01.ldtk
// -----------------------------------------------------------------------
static constexpr bool USE_LDTK = true;

int main()
{
    InitWindow(1280, 720, "SpenceDew");
    SetTargetFPS(60);

    // ----------------------------------------------------------------
    // Load all levels once (kept alive so we can warp between them).
    // ----------------------------------------------------------------
    std::vector<LdtkLevel> levels;
    if (USE_LDTK)
        LdtkLoader::load("assets/levels/level_01.ldtk", levels);

    // ----------------------------------------------------------------
    // Persistent systems (survive across level warps)
    // ----------------------------------------------------------------
    Tilemap tilemap;
    Pathfinder pathfinder;
    DamageNumbers damageNumbers;
    FarmController farm;

    Player player;
    player.init();
    player.walls = &tilemap.walls;
    player.onDamageVisual = [&damageNumbers](Vector2 pos, float dmg)
    {
        damageNumbers.spawn(pos, dmg, WHITE);
    };

    std::vector<Projectile> projectiles;
    projectiles.reserve(32);
    player.onFireProjectile = [&projectiles, &player](Vector2 pos, Vector2 dir)
    {
        Projectile bolt;
        bolt.init(pos, dir, 420.0f, 20.0f, 230.0f, &player);
        projectiles.push_back(bolt);
    };

    // Enemies + NPCs are (re)spawned per level.
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Friend>> friends;

    auto addPartner = [&](Vector2 pos, const std::string &id,
                          const std::string &name, const std::string &dialogueId)
    {
        auto p = std::make_unique<PotentialPartner>();
        p->init(pos, id, name, dialogueId);
        p->walls = &tilemap.walls;
        friends.push_back(std::move(p));
    };

    // ----------------------------------------------------------------
    // Dialogue + relationships
    // ----------------------------------------------------------------
    RelationshipManager::instance().load("relationships.sav");
    std::unordered_map<std::string, Dialogue> dialogueCache;
    DialogueRunner dialogue;
    auto getDialogue = [&](const std::string &id) -> const Dialogue *
    {
        auto it = dialogueCache.find(id);
        if (it != dialogueCache.end())
            return &it->second;
        Dialogue d;
        if (!DialogueLoader::load("assets/dialogue/" + id + ".json", d))
            return nullptr;
        auto [ins, ok] = dialogueCache.emplace(id, std::move(d));
        return &ins->second;
    };

    // ----------------------------------------------------------------
    // Level activation — (re)build everything tied to a specific level.
    // Called at startup and whenever the player warps (Backspace).
    // ----------------------------------------------------------------
    int currentLevel = 0;

    auto buildPathfinder = [&]()
    {
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        for (const auto &w : tilemap.walls)
        {
            minX = std::min(minX, w.x);
            minY = std::min(minY, w.y);
            maxX = std::max(maxX, w.x + w.width);
            maxY = std::max(maxY, w.y + w.height);
        }
        if (maxX > minX && maxY > minY)
            pathfinder.buildGrid(tilemap.walls, {minX, minY, maxX - minX, maxY - minY});
    };

    auto activateLevel = [&](int idx)
    {
        dialogue.stop();
        enemies.clear();
        friends.clear();

        if (levels.empty())
        {
            // No LDtk data — fall back to the hardcoded test room.
            tilemap.unloadVisuals();
            tilemap = Tilemap::createTestRoom();
            player.position = {0.0f, 0.0f};
            buildPathfinder();
            farm.clear();
            currentLevel = 0;
            return;
        }

        idx = ((idx % (int)levels.size()) + (int)levels.size()) % (int)levels.size();
        currentLevel = idx;
        const LdtkLevel &lvl = levels[idx];

        // --- Tilemap visuals + collision ---
        tilemap.unloadVisuals();
        tilemap.walls = lvl.walls;
        tilemap.loadVisuals(lvl, "assets/tilesets/tilemap.png");

        // --- Spawn markers ---
        Vector2 playerSpawn = {0.0f, 0.0f};
        std::vector<Vector2> enemySpawns, partnerSpawns;
        for (const auto &ent : lvl.entities)
        {
            if (ent.identifier == "PlayerSpawn")
                playerSpawn = ent.position;
            else if (ent.identifier == "EnemySpawn")
                enemySpawns.push_back(ent.position);
            else if (ent.identifier == "PartnerSpawn")
                partnerSpawns.push_back(ent.position);
        }

        player.position = playerSpawn;
        player.velocity = {0.0f, 0.0f};

        buildPathfinder();

        // --- Enemies ---
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

        // --- Demo romanceable NPC on the starting level ---
        if (idx == 0)
        {
            Vector2 wpos = partnerSpawns.empty()
                               ? Vector2{playerSpawn.x + 64.0f, playerSpawn.y}
                               : partnerSpawns[0];
            addPartner(wpos, "willow", "Willow", "willow");
        }

        // --- Farmable tiles for this level ---
        farm.build(lvl);
    };

    activateLevel(0);

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

        // ---- Debug level warp (Backspace) ----
        // Rebuilds the whole level in place. Do NOT `continue` here: skipping
        // EndDrawing() would also skip raylib's input polling, leaving Backspace
        // stuck "just pressed" and re-warping every frame. All entity/farm state
        // below is computed *after* this, so it safely uses the new level.
        if (!dialogue.isActive() && Input::warpJustPressed())
            activateLevel(currentLevel + 1);

        // ---- Tile the player would act on: the cell just in front of them ----
        Vector2 farmTarget = {
            player.position.x + player.facing.x * (float)farm.gridSize(),
            player.position.y + player.facing.y * (float)farm.gridSize()};

        // ---- Find the nearest interactable NPC in range (for prompt + input) ----
        Friend *nearbyNpc = nullptr;
        {
            float best = FLT_MAX;
            for (auto &fptr : friends)
            {
                if (!fptr->playerInRange(player.position))
                    continue;
                float d = (fptr->position.x - player.position.x) * (fptr->position.x - player.position.x) +
                          (fptr->position.y - player.position.y) * (fptr->position.y - player.position.y);
                if (d < best)
                {
                    best = d;
                    nearbyNpc = fptr.get();
                }
            }
        }

        if (dialogue.isActive())
        {
            // ---- World is frozen while talking; route input to the dialogue ----
            if (Input::dialogueUpJustPressed())
                dialogue.moveSelection(-1);
            if (Input::dialogueDownJustPressed())
                dialogue.moveSelection(+1);
            if (Input::dialogueConfirmJustPressed())
                dialogue.confirm();
        }
        else
        {
            // ---- Start a conversation on interact ----
            if (nearbyNpc && Input::interactJustPressed())
            {
                if (const Dialogue *d = getDialogue(nearbyNpc->dialogueId))
                {
                    int aff = RelationshipManager::instance().affection(nearbyNpc->npcId);
                    dialogue.start(d, nearbyNpc->npcId, nearbyNpc->displayName,
                                   nearbyNpc->dialogueTier(aff));
                }
            }

            // ---- Farming: one button tills → plants → waters the target tile ----
            if (Input::farmActionJustPressed())
                farm.interact(farmTarget);

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

            // ---- Update NPCs (idle / sprite clock) ----
            for (auto &fptr : friends)
                fptr->update(delta);

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
        }

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

        // Farm tiles + the currently targeted cell
        farm.draw();
        {
            Rectangle tr = farm.cellRect(farmTarget);
            Color hl = farm.isFarmable(farmTarget) ? Fade(YELLOW, 0.9f) : Fade(RED, 0.45f);
            DrawRectangleLinesEx(tr, 2.0f, hl);
        }

        // Draw enemies
        for (const auto &e : enemies)
            e->draw();

        // Draw NPCs
        for (const auto &fptr : friends)
            fptr->draw();

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
        DrawText(TextFormat("Level: %s   |  E Talk  |  F Till/Plant/Water  |  Backspace Warp",
                            levels.empty() ? "test" : levels[currentLevel].identifier.c_str()),
                 10, GetScreenHeight() - 42, 14, DARKGRAY);

        // --- Interaction prompt + dialogue box (screen space) ---
        if (dialogue.isActive())
        {
            DialogueUI::draw(dialogue, GetScreenWidth(), GetScreenHeight());
        }
        else if (nearbyNpc)
        {
            DialogueUI::drawInteractionPrompt(nearbyNpc->displayName,
                                              GetScreenWidth(), GetScreenHeight());
        }

        EndDrawing();
    }

    RelationshipManager::instance().save("relationships.sav");

    CloseWindow();
    return 0;
}
