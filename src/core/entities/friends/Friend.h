#pragma once
#include "core/entities/NPC.h"
#include <string>

// ============================================================
//  Friend  —  a non-hostile, talk-to-able NPC.
//
//  Adds interaction + dialogue on top of NPC. A Friend stands
//  around (static for the MVP) and, when the player is within
//  interactionRadius and presses the interact key, starts the
//  dialogue named by dialogueId.
//
//  Identity is keyed by npcId — the same string used by the
//  RelationshipManager and (by convention) the dialogue file:
//    dialogueId "willow"  →  assets/dialogue/willow.json
// ============================================================
class Friend : public NPC
{
public:
    std::string npcId;       // Unique key (relationships, spawning, saves)
    std::string displayName; // Shown on the dialogue name label
    std::string dialogueId;  // Dialogue file id → assets/dialogue/<id>.json

    float interactionRadius = 44.0f;
    Color debugColor = SKYBLUE; // Used when no sprite is loaded

    void init(Vector2 spawnPos,
              const std::string &id,
              const std::string &name,
              const std::string &dialogue);

    // True when the player is close enough to interact.
    bool playerInRange(Vector2 playerPos) const;

    // Which dialogue tier to start at, given current affection.
    // Plain Friends only ever have tier 0; PotentialPartner overrides this.
    virtual int dialogueTier(int /*affection*/) const { return 0; }

    void update(float delta) override;
    void draw() const override;
};
