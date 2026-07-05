#include "core/systems/tilemap/LdtkLoader.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <cstdio>

using json = nlohmann::json;

bool LdtkLoader::load(const std::string &filePath, std::vector<LdtkLevel> &outLevels)
{
    std::ifstream f(filePath);
    if (!f.is_open())
    {
        printf("[LdtkLoader] Cannot open: %s\n", filePath.c_str());
        return false;
    }

    json root = json::parse(f, nullptr, /*allow_exceptions=*/false);
    if (root.is_discarded())
    {
        printf("[LdtkLoader] JSON parse error in: %s\n", filePath.c_str());
        return false;
    }

    if (!root.contains("levels"))
    {
        printf("[LdtkLoader] No 'levels' key in: %s\n", filePath.c_str());
        return false;
    }

    for (const auto &lvl : root["levels"])
    {
        LdtkLevel level;
        level.identifier = lvl.value("identifier", "");
        level.worldX = lvl.value("worldX", 0);
        level.worldY = lvl.value("worldY", 0);
        level.pxWidth = lvl.value("pxWid", 0);
        level.pxHeight = lvl.value("pxHei", 0);

        // LDtk may store layer data in a separate file per level
        // (externalRelPath) — not supported here; only inline layerInstances.
        if (!lvl.contains("layerInstances") || lvl["layerInstances"].is_null())
        {
            printf("[LdtkLoader] Level '%s' has no inline layerInstances (external levels not supported).\n",
                   level.identifier.c_str());
            outLevels.push_back(level);
            continue;
        }

        for (const auto &layer : lvl["layerInstances"])
        {
            const std::string type = layer.value("__type", "");
            const int gridSize = layer.value("__gridSize", 32);
            const int cWid = layer.value("__cWid", 0);

            // ---- IntGrid collision layer ----
            if (type == "IntGrid" && layer.contains("intGridCsv"))
            {
                const auto &csv = layer["intGridCsv"];
                for (int i = 0; i < static_cast<int>(csv.size()); ++i)
                {
                    if (csv[i].get<int>() == 0)
                        continue;
                    const int col = i % cWid;
                    const int row = i / cWid;
                    Rectangle wall{
                        static_cast<float>(level.worldX + col * gridSize),
                        static_cast<float>(level.worldY + row * gridSize),
                        static_cast<float>(gridSize),
                        static_cast<float>(gridSize)};
                    level.walls.push_back(wall);
                }
            }

            // ---- Entities layer ----
            if (type == "Entities" && layer.contains("entityInstances"))
            {
                for (const auto &ent : layer["entityInstances"])
                {
                    LdtkEntity entity;
                    entity.identifier = ent.value("__identifier", "");
                    if (ent.contains("px") && ent["px"].size() >= 2)
                    {
                        entity.position = {
                            static_cast<float>(level.worldX + ent["px"][0].get<int>()),
                            static_cast<float>(level.worldY + ent["px"][1].get<int>())};
                    }
                    level.entities.push_back(entity);
                }
            }
        }

        outLevels.push_back(std::move(level));
    }

    return true;
}
