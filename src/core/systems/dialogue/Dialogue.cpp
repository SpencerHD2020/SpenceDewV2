#include "core/systems/dialogue/Dialogue.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <cstdio>

using json = nlohmann::json;

std::string Dialogue::startNode(int tier) const
{
    for (int t = tier; t >= 1; --t)
    {
        std::string key = "start_tier" + std::to_string(t);
        if (nodes.find(key) != nodes.end())
            return key;
    }
    return start;
}

const DialogueNode *Dialogue::find(const std::string &id) const
{
    auto it = nodes.find(id);
    return (it != nodes.end()) ? &it->second : nullptr;
}

bool DialogueLoader::load(const std::string &path, Dialogue &out)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        printf("[DialogueLoader] Cannot open: %s\n", path.c_str());
        return false;
    }

    json data = json::parse(f, nullptr, /*allow_exceptions=*/false);
    if (data.is_discarded())
    {
        printf("[DialogueLoader] JSON parse error: %s\n", path.c_str());
        return false;
    }

    out.id = data.value("id", "");
    out.start = data.value("start", "start");
    out.nodes.clear();

    if (!data.contains("nodes"))
    {
        printf("[DialogueLoader] No 'nodes' in: %s\n", path.c_str());
        return false;
    }

    for (auto &[nodeId, nodeJson] : data["nodes"].items())
    {
        DialogueNode node;
        node.speaker = nodeJson.value("speaker", "");
        node.text = nodeJson.value("text", "");

        if (nodeJson.contains("choices"))
        {
            for (const auto &c : nodeJson["choices"])
            {
                DialogueChoice choice;
                choice.text = c.value("text", "...");
                choice.next = c.value("next", "END");
                choice.affection = c.value("affection", 0);
                choice.requiresAffection = c.value("requiresAffection", 0);
                choice.setFlag = c.value("setFlag", "");
                node.choices.push_back(std::move(choice));
            }
        }

        out.nodes[nodeId] = std::move(node);
    }

    return true;
}
