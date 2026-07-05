#include "core/animation/AnimatedSprite.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <cstdio>
#include <cmath>

using json = nlohmann::json;

bool AnimatedSprite::load(const std::string &texturePath, const std::string &jsonPath)
{
    texture = LoadTexture(texturePath.c_str());
    if (texture.id == 0)
    {
        printf("[AnimatedSprite] Failed to load texture: %s\n", texturePath.c_str());
        return false;
    }

    std::string jp = jsonPath.empty() ? texturePath + ".json" : jsonPath;
    std::ifstream f(jp);
    if (!f.is_open())
    {
        printf("[AnimatedSprite] JSON not found: %s — animations must be added manually.\n", jp.c_str());
        return true; // Texture still usable; caller can add anim defs in code
    }

    json data = json::parse(f, nullptr, /*allow_exceptions=*/false);
    if (data.is_discarded())
    {
        printf("[AnimatedSprite] JSON parse error: %s\n", jp.c_str());
        return true;
    }

    frameWidth = data.value("frameWidth", 32.0f);
    frameHeight = data.value("frameHeight", 32.0f);

    if (data.contains("animations"))
    {
        for (auto &[name, anim] : data["animations"].items())
        {
            AnimationDef def;
            def.row = anim.value("row", 0);
            def.frameCount = anim.value("frameCount", 1);
            def.fps = anim.value("fps", 8.0f);
            def.loop = anim.value("loop", true);
            animations[name] = def;
        }
    }

    return true;
}

void AnimatedSprite::unload()
{
    if (texture.id != 0)
    {
        UnloadTexture(texture);
        texture = {};
    }
}

void AnimatedSprite::play(const std::string &animName)
{
    if (m_currentAnim == animName)
        return;
    m_currentAnim = animName;
    m_currentFrame = 0;
    m_frameTimer = 0.0f;
    m_isPlaying = true;
    finished = false;
}

void AnimatedSprite::update(float delta)
{
    finished = false;
    if (!m_isPlaying || m_currentAnim.empty())
        return;

    auto it = animations.find(m_currentAnim);
    if (it == animations.end())
        return;

    const AnimationDef &def = it->second;
    const float frameDuration = (def.fps > 0.0f) ? (1.0f / def.fps) : 0.1f;

    m_frameTimer += delta;
    while (m_frameTimer >= frameDuration)
    {
        m_frameTimer -= frameDuration;
        ++m_currentFrame;
        if (m_currentFrame >= def.frameCount)
        {
            if (def.loop)
            {
                m_currentFrame = 0;
            }
            else
            {
                m_currentFrame = def.frameCount - 1;
                m_isPlaying = false;
                finished = true;
            }
        }
    }
}

Rectangle AnimatedSprite::getSourceRect() const
{
    auto it = animations.find(m_currentAnim);
    int row = (it != animations.end()) ? it->second.row : 0;

    float y = row * frameHeight;

    // When flipping, start X at the RIGHT edge of the frame so that the UV
    // range is [right/texW → left/texW] (valid, reversed) rather than
    // [left/texW → negative] which samples outside the texture.
    float x, w;
    if (flipX)
    {
        x = (m_currentFrame + 1) * frameWidth;
        w = -frameWidth;
    }
    else
    {
        x = m_currentFrame * frameWidth;
        w = frameWidth;
    }
    return {x, y, w, frameHeight};
}

void AnimatedSprite::draw(Vector2 position, float scale, Color tint) const
{
    if (texture.id == 0 || m_currentAnim.empty())
        return;

    Rectangle src = getSourceRect();
    float absW = std::abs(src.width) * scale;
    float absH = std::abs(src.height) * scale;
    Rectangle dest = {position.x, position.y, absW, absH};
    Vector2 origin = {absW * 0.5f, absH * 0.5f};

    DrawTexturePro(texture, src, dest, origin, 0.0f, tint);
}
