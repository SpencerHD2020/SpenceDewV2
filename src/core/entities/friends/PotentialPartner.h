#pragma once
#include "core/entities/friends/Friend.h"
#include <string>
#include <vector>

// ============================================================
//  PotentialPartner  —  a Friend who can be romanced.
//
//  Affection itself is stored centrally in RelationshipManager
//  (keyed by npcId) so it can be saved/loaded and read by any
//  system. This subclass only adds the romance *config*:
//    - the affection thresholds that separate dialogue tiers
//    - optional gift preferences
//
//  dialogueTier() maps a raw affection score to a tier index,
//  which the DialogueRunner uses to pick the matching entry node
//  (e.g. "start_tier2") in the NPC's dialogue file.
// ============================================================
class PotentialPartner : public Friend
{
public:
    // Affection needed to reach each tier. Tier 0 is the default (below tier1).
    int tier1Threshold = 10;
    int tier2Threshold = 25;

    // Optional: ids of gifts this partner especially likes (see the guide).
    std::vector<std::string> likedGifts;

    void init(Vector2 spawnPos,
              const std::string &id,
              const std::string &name,
              const std::string &dialogue);

    int dialogueTier(int affection) const override
    {
        if (affection >= tier2Threshold)
            return 2;
        if (affection >= tier1Threshold)
            return 1;
        return 0;
    }

    void draw() const override;
};
