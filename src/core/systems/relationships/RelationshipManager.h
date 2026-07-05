#pragma once
#include <string>
#include <unordered_map>

// ============================================================
//  RelationshipManager  —  central store of per-NPC social state.
//
//  Keyed by npcId (the same string a Friend/PotentialPartner uses).
//  Holds an affection score and arbitrary named boolean flags
//  (e.g. "met", "gave_flower", "confessed"). Dialogue choices
//  read and mutate this via the DialogueRunner.
//
//  Singleton so any system can reach it without plumbing a
//  pointer everywhere. Serialisable to JSON for save/load.
// ============================================================

struct NpcRelationship
{
    int affection = 0;
    std::unordered_map<std::string, bool> flags;
};

class RelationshipManager
{
public:
    static RelationshipManager &instance();

    int affection(const std::string &npcId) const;
    void addAffection(const std::string &npcId, int delta); // clamped at >= 0
    void setAffection(const std::string &npcId, int value);

    bool hasFlag(const std::string &npcId, const std::string &flag) const;
    void setFlag(const std::string &npcId, const std::string &flag, bool value = true);

    bool save(const std::string &path) const;
    bool load(const std::string &path);

private:
    RelationshipManager() = default;
    std::unordered_map<std::string, NpcRelationship> m_data;
};
