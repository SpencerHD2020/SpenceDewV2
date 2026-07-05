#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

// ============================================================
//  AnimatedSprite  —  plays named frame animations from a
//  grid-layout sprite sheet PNG.
//
//  Companion JSON (e.g. "player.png.json") defines rows:
//  {
//    "frameWidth": 32, "frameHeight": 32,
//    "animations": {
//      "idle_down": { "row": 0, "frameCount": 4, "fps": 8, "loop": true }
//    }
//  }
//
//  Right-facing anims reuse left-facing rows with flipX = true.
// ============================================================

struct AnimationDef
{
    int row = 0;
    int frameCount = 1;
    float fps = 8.0f;
    bool loop = true;
};

class AnimatedSprite
{
public:
    Texture2D texture = {};
    // Frame cell size in source-texture pixels. Float because sprite sheets are
    // not always an integer multiple of the cell size (e.g. a 525x562 sheet on a
    // 6x6 grid → 87.5 x 93.667). Using ints here truncates and misaligns every
    // frame, which makes the animation sample wrong slivers of the sheet.
    float frameWidth = 32.0f;
    float frameHeight = 32.0f;

    bool flipX = false;
    bool finished = false; // true for exactly one frame when a non-looping anim ends

    std::unordered_map<std::string, AnimationDef> animations;

    // Load texture from path; JSON is assumed at path + ".json" unless jsonPath is given.
    bool load(const std::string &texturePath, const std::string &jsonPath = "");
    void unload();

    // Start a named animation (no-op if already playing that anim).
    void play(const std::string &animName);

    // Call once per frame before draw().
    void update(float delta);

    // Draw centred on position (world-space).
    void draw(Vector2 position, float scale = 1.0f, Color tint = WHITE) const;

    bool hasTexture() const { return texture.id != 0; }
    bool isAnimFinished() const { return finished; }
    const std::string &currentAnim() const { return m_currentAnim; }

private:
    std::string m_currentAnim;
    int m_currentFrame = 0;
    float m_frameTimer = 0.0f;
    bool m_isPlaying = false;

    Rectangle getSourceRect() const;
};
