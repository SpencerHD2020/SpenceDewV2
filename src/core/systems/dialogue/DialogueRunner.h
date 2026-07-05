#pragma once
#include "core/systems/dialogue/Dialogue.h"
#include <string>
#include <vector>

// ============================================================
//  DialogueRunner  —  drives one active conversation.
//
//  Owns no rendering and no input polling; it exposes state
//  (current speaker/text/choices/selection) that the UI reads,
//  and mutation methods (moveSelection / confirm) that the game
//  loop calls in response to input. This keeps entities and UI
//  decoupled per the project's architecture rule.
//
//  Effects (affection changes, flags) are applied through the
//  RelationshipManager singleton, keyed by the active npcId.
// ============================================================
class DialogueRunner
{
public:
    bool isActive() const { return m_active; }

    // Begin a conversation. `tier` selects the entry node (see Dialogue::startNode).
    void start(const Dialogue *dialogue,
               const std::string &npcId,
               const std::string &npcName,
               int tier);
    void stop();

    // --- Input hooks (called by the game loop) ---
    void moveSelection(int dir); // -1 = up, +1 = down (wraps)
    void confirm();              // pick highlighted choice / advance / close

    // --- View accessors (read by the UI) ---
    struct VisibleChoice
    {
        std::string text;
        int sourceIndex; // Index into the node's full choice list
    };

    std::string speaker() const;
    const std::string &text() const;
    const std::vector<VisibleChoice> &visibleChoices() const { return m_visible; }
    int selection() const { return m_selection; }

private:
    void enterNode(const std::string &nodeId);
    void rebuildVisibleChoices();

    const Dialogue *m_dialogue = nullptr;
    const DialogueNode *m_node = nullptr;
    std::string m_npcId;
    std::string m_npcName;
    std::vector<VisibleChoice> m_visible;
    int m_selection = 0;
    bool m_active = false;
};
