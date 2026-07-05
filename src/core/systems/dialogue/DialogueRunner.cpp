#include "core/systems/dialogue/DialogueRunner.h"
#include "core/systems/relationships/RelationshipManager.h"

void DialogueRunner::start(const Dialogue *dialogue,
                           const std::string &npcId,
                           const std::string &npcName,
                           int tier)
{
    if (!dialogue)
        return;

    m_dialogue = dialogue;
    m_npcId = npcId;
    m_npcName = npcName;
    m_active = true;

    // Mark that we've met this NPC the first time we talk to them.
    RelationshipManager::instance().setFlag(npcId, "met", true);

    enterNode(dialogue->startNode(tier));
}

void DialogueRunner::stop()
{
    m_active = false;
    m_dialogue = nullptr;
    m_node = nullptr;
    m_visible.clear();
    m_selection = 0;
}

void DialogueRunner::enterNode(const std::string &nodeId)
{
    if (!m_dialogue || nodeId == "END")
    {
        stop();
        return;
    }

    m_node = m_dialogue->find(nodeId);
    if (!m_node)
    {
        // Unknown node id — fail safe by ending the conversation.
        stop();
        return;
    }

    m_selection = 0;
    rebuildVisibleChoices();
}

void DialogueRunner::rebuildVisibleChoices()
{
    m_visible.clear();
    if (!m_node)
        return;

    const int affection = RelationshipManager::instance().affection(m_npcId);
    for (int i = 0; i < static_cast<int>(m_node->choices.size()); ++i)
    {
        const DialogueChoice &c = m_node->choices[i];
        if (affection >= c.requiresAffection)
            m_visible.push_back({c.text, i});
    }
}

void DialogueRunner::moveSelection(int dir)
{
    if (!m_active || m_visible.empty())
        return;
    const int n = static_cast<int>(m_visible.size());
    m_selection = ((m_selection + dir) % n + n) % n;
}

void DialogueRunner::confirm()
{
    if (!m_active || !m_node)
        return;

    // No choices → this is a "press to continue" line that just ends.
    if (m_visible.empty())
    {
        stop();
        return;
    }

    const VisibleChoice &vc = m_visible[m_selection];
    const DialogueChoice &choice = m_node->choices[vc.sourceIndex];

    // Apply effects.
    RelationshipManager &rel = RelationshipManager::instance();
    if (choice.affection != 0)
        rel.addAffection(m_npcId, choice.affection);
    if (!choice.setFlag.empty())
        rel.setFlag(m_npcId, choice.setFlag, true);

    enterNode(choice.next);
}

std::string DialogueRunner::speaker() const
{
    if (m_node && !m_node->speaker.empty())
        return m_node->speaker;
    return m_npcName;
}

const std::string &DialogueRunner::text() const
{
    static const std::string empty;
    return m_node ? m_node->text : empty;
}
