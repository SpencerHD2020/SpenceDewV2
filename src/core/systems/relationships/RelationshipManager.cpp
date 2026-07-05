#include "core/systems/relationships/RelationshipManager.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <cstdio>

using json = nlohmann::json;

RelationshipManager &RelationshipManager::instance()
{
    static RelationshipManager s_instance;
    return s_instance;
}

int RelationshipManager::affection(const std::string &npcId) const
{
    auto it = m_data.find(npcId);
    return (it != m_data.end()) ? it->second.affection : 0;
}

void RelationshipManager::addAffection(const std::string &npcId, int delta)
{
    NpcRelationship &r = m_data[npcId];
    r.affection += delta;
    if (r.affection < 0)
        r.affection = 0;
}

void RelationshipManager::setAffection(const std::string &npcId, int value)
{
    m_data[npcId].affection = (value < 0) ? 0 : value;
}

bool RelationshipManager::hasFlag(const std::string &npcId, const std::string &flag) const
{
    auto it = m_data.find(npcId);
    if (it == m_data.end())
        return false;
    auto fit = it->second.flags.find(flag);
    return (fit != it->second.flags.end()) && fit->second;
}

void RelationshipManager::setFlag(const std::string &npcId, const std::string &flag, bool value)
{
    m_data[npcId].flags[flag] = value;
}

bool RelationshipManager::save(const std::string &path) const
{
    json root = json::object();
    for (const auto &[id, rel] : m_data)
    {
        json entry;
        entry["affection"] = rel.affection;
        entry["flags"] = json::object();
        for (const auto &[flag, val] : rel.flags)
            entry["flags"][flag] = val;
        root[id] = entry;
    }

    std::ofstream f(path);
    if (!f.is_open())
    {
        printf("[RelationshipManager] Cannot write: %s\n", path.c_str());
        return false;
    }
    f << root.dump(2);
    return true;
}

bool RelationshipManager::load(const std::string &path)
{
    std::ifstream f(path);
    if (!f.is_open())
        return false; // No save yet — not an error.

    json root = json::parse(f, nullptr, /*allow_exceptions=*/false);
    if (root.is_discarded())
    {
        printf("[RelationshipManager] JSON parse error: %s\n", path.c_str());
        return false;
    }

    m_data.clear();
    for (auto &[id, entry] : root.items())
    {
        NpcRelationship rel;
        rel.affection = entry.value("affection", 0);
        if (entry.contains("flags"))
            for (auto &[flag, val] : entry["flags"].items())
                rel.flags[flag] = val.get<bool>();
        m_data[id] = std::move(rel);
    }
    return true;
}
