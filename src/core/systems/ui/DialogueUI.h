#pragma once
#include "raylib.h"
#include "core/systems/dialogue/DialogueRunner.h"
#include <string>
#include <vector>

// ============================================================
//  DialogueUI  —  draws the active conversation and the
//  "Press E to talk" interaction prompt.
//
//  Pure view: it only reads from a DialogueRunner. All input is
//  handled by the game loop, which then calls the runner's
//  moveSelection()/confirm(). Draw these AFTER EndMode2D() so
//  they render in screen space.
// ============================================================
namespace DialogueUI
{
    // Word-wrap `text` to fit within maxWidth px at the given font size.
    inline std::vector<std::string> wrapText(const std::string &text, int fontSize, int maxWidth)
    {
        std::vector<std::string> lines;
        std::string line;
        std::string word;

        auto flushWord = [&]()
        {
            if (word.empty())
                return;
            std::string candidate = line.empty() ? word : line + " " + word;
            if (MeasureText(candidate.c_str(), fontSize) > maxWidth && !line.empty())
            {
                lines.push_back(line);
                line = word;
            }
            else
            {
                line = candidate;
            }
            word.clear();
        };

        for (char ch : text)
        {
            if (ch == '\n')
            {
                flushWord();
                lines.push_back(line);
                line.clear();
            }
            else if (ch == ' ')
            {
                flushWord();
            }
            else
            {
                word += ch;
            }
        }
        flushWord();
        if (!line.empty())
            lines.push_back(line);
        return lines;
    }

    // Small prompt shown near the bottom when an NPC is interactable.
    inline void drawInteractionPrompt(const std::string &npcName, int screenW, int screenH)
    {
        std::string msg = "Press E to talk to " + npcName;
        const int fontSize = 18;
        int tw = MeasureText(msg.c_str(), fontSize);
        int boxW = tw + 24;
        int boxH = fontSize + 16;
        int x = (screenW - boxW) / 2;
        int y = screenH - 150;
        DrawRectangle(x, y, boxW, boxH, {0, 0, 0, 180});
        DrawRectangleLines(x, y, boxW, boxH, {220, 220, 220, 220});
        DrawText(msg.c_str(), x + 12, y + 8, fontSize, RAYWHITE);
    }

    // Full dialogue textbox. No-op when the runner is inactive.
    inline void draw(const DialogueRunner &runner, int screenW, int screenH)
    {
        if (!runner.isActive())
            return;

        const int margin = 24;
        const int boxH = 170;
        const int boxX = margin;
        const int boxY = screenH - boxH - margin;
        const int boxW = screenW - margin * 2;
        const int pad = 16;
        const int textWidth = boxW - pad * 2;

        // Panel
        DrawRectangle(boxX, boxY, boxW, boxH, {15, 15, 25, 230});
        DrawRectangleLines(boxX, boxY, boxW, boxH, {180, 180, 210, 230});

        // Speaker name label
        const std::string speaker = runner.speaker();
        int nameW = MeasureText(speaker.c_str(), 20) + 16;
        DrawRectangle(boxX, boxY - 26, nameW, 26, {40, 40, 70, 240});
        DrawRectangleLines(boxX, boxY - 26, nameW, 26, {180, 180, 210, 230});
        DrawText(speaker.c_str(), boxX + 8, boxY - 22, 20, GOLD);

        // Body text (wrapped)
        const int bodyFont = 18;
        int ty = boxY + pad;
        for (const std::string &line : wrapText(runner.text(), bodyFont, textWidth))
        {
            DrawText(line.c_str(), boxX + pad, ty, bodyFont, RAYWHITE);
            ty += bodyFont + 4;
        }

        // Choices, or a continue hint
        const auto &choices = runner.visibleChoices();
        const int choiceFont = 18;
        int cy = boxY + boxH - pad - (static_cast<int>(choices.size()) * (choiceFont + 6));
        if (choices.empty())
        {
            const char *hint = "\xE2\x96\xB6 Continue  (E)"; // ▶
            int hw = MeasureText(hint, 16);
            DrawText(hint, boxX + boxW - pad - hw, boxY + boxH - pad - 16, 16, LIGHTGRAY);
        }
        else
        {
            for (int i = 0; i < static_cast<int>(choices.size()); ++i)
            {
                bool selected = (i == runner.selection());
                Color col = selected ? YELLOW : GRAY;
                std::string prefix = selected ? "> " : "  ";
                DrawText((prefix + choices[i].text).c_str(),
                         boxX + pad, cy, choiceFont, col);
                cy += choiceFont + 6;
            }
        }

        // Controls hint
        DrawText("W/S select   E confirm", boxX + pad, boxY + boxH - 4 - 12, 12,
                 {150, 150, 150, 200});
    }
} // namespace DialogueUI
