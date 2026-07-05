#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// ============================================================
//  Dialogue data model
//
//  A dialogue is a bag of named nodes. Each node shows one line
//  of text and offers 0+ choices. A choice points at the next
//  node id (or "END" to close the conversation) and may apply
//  effects (affection change, set a flag) and be gated by a
//  minimum affection.
//
//  JSON format (assets/dialogue/<id>.json):
//  {
//    "id": "willow",
//    "start": "greet",
//    "nodes": {
//      "greet": {
//        "speaker": "Willow",
//        "text": "Oh — hello. I didn't hear you come in.",
//        "choices": [
//          { "text": "Hi there!",  "next": "warm",  "affection": 1 },
//          { "text": "(Say nothing)", "next": "END" }
//        ]
//      }
//    }
//  }
//
//  Tiered greetings: name entry nodes "start_tier1", "start_tier2"
//  to give higher-affection partners different opening lines.
//  startNode(tier) falls back from the requested tier down to
//  "start".
// ============================================================

struct DialogueChoice
{
    std::string text;
    std::string next = "END";  // Target node id, or "END"
    int affection = 0;         // Applied to the NPC when this choice is picked
    int requiresAffection = 0; // Choice is hidden if affection < this
    std::string setFlag;       // Optional relationship flag to set when picked
};

struct DialogueNode
{
    std::string speaker; // Overrides the NPC's display name if set
    std::string text;
    std::vector<DialogueChoice> choices; // Empty → "continue" then END
};

struct Dialogue
{
    std::string id;
    std::string start = "start";
    std::unordered_map<std::string, DialogueNode> nodes;

    // Prefer "start_tier<tier>", falling back through lower tiers to "start".
    std::string startNode(int tier) const;

    const DialogueNode *find(const std::string &id) const;
};

class DialogueLoader
{
public:
    // Loads a dialogue file. Returns false (and logs) if it can't be read/parsed.
    static bool load(const std::string &path, Dialogue &out);
};
