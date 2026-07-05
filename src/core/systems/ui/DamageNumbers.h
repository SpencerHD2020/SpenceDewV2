#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>

// ============================================================
//  DamageNumbers  —  floating world-space text that rises
//  and fades out after 1 second.
//
//  Call spawn() inside a hit callback, update() each frame,
//  and draw() inside BeginMode2D so numbers appear in world space.
// ============================================================

struct DamageNumber
{
    Vector2 position;
    std::string text;
    Color color;
    float lifetime;
    float maxLifetime;
    float riseSpeed = 35.0f;

    bool isAlive() const { return lifetime > 0.0f; }

    void update(float delta)
    {
        if (!isAlive())
            return;
        lifetime -= delta;
        position.y -= riseSpeed * delta;
        // Slight drift
        position.x += 8.0f * delta * (lifetime / maxLifetime - 0.5f);
    }

    void draw() const
    {
        if (!isAlive())
            return;
        float alpha = std::max(0.0f, lifetime / maxLifetime);
        Color c = {color.r, color.g, color.b,
                   static_cast<unsigned char>(255.0f * alpha)};
        DrawText(text.c_str(),
                 static_cast<int>(position.x),
                 static_cast<int>(position.y),
                 16, c);
    }
};

struct DamageNumbers
{
    std::vector<DamageNumber> numbers;

    void spawn(Vector2 worldPos, float damage, Color color = RED)
    {
        DamageNumber dn;
        dn.position = {worldPos.x - 8.0f, worldPos.y - 24.0f};
        dn.lifetime = 1.0f;
        dn.maxLifetime = 1.0f;
        dn.color = color;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.0f", damage);
        dn.text = buf;
        numbers.push_back(dn);
    }

    void update(float delta)
    {
        for (auto &n : numbers)
            n.update(delta);
        numbers.erase(
            std::remove_if(numbers.begin(), numbers.end(),
                           [](const DamageNumber &n)
                           { return !n.isAlive(); }),
            numbers.end());
    }

    // Call between BeginMode2D / EndMode2D
    void draw() const
    {
        for (const auto &n : numbers)
            n.draw();
    }
};
