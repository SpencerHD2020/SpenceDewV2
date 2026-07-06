#pragma once
#include "raylib.h"
#include "raymath.h"

// ============================================================
//  Input  —  centralised key/button mappings
//  Change bindings here; the rest of the code never touches
//  raw key codes.
// ============================================================
namespace Input
{

    // --- Movement ---
    inline bool moveLeft() { return IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT); }
    inline bool moveRight() { return IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT); }
    inline bool moveUp() { return IsKeyDown(KEY_W) || IsKeyDown(KEY_UP); }
    inline bool moveDown() { return IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN); }
    inline bool sprintHeld() { return IsKeyDown(KEY_LEFT_SHIFT); }

    // --- Actions (just-pressed, one frame only) ---
    inline bool attackJustPressed()
    {
        return IsKeyPressed(KEY_Z) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }
    inline bool dodgeJustPressed()
    {
        return IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_X);
    }
    inline bool castJustPressed()
    {
        return IsKeyPressed(KEY_C) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    }

    // --- Interaction / dialogue ---
    inline bool interactJustPressed() { return IsKeyPressed(KEY_E); }
    inline bool dialogueUpJustPressed() { return IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP); }
    inline bool dialogueDownJustPressed() { return IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN); }
    inline bool dialogueConfirmJustPressed()
    {
        return IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    }

    // --- Farming / world ---
    // Single "use tool on the ground" button (temporary: till → plant → water
    // based on the target tile's state).
    inline bool farmActionJustPressed() { return IsKeyPressed(KEY_F); }
    // Debug: warp to the next level.
    inline bool warpJustPressed() { return IsKeyPressed(KEY_BACKSPACE); }

    // Returns a normalised direction vector from WASD / arrow keys.
    // Returns {0,0} when no directional input is held.
    inline Vector2 getMoveVector()
    {
        Vector2 dir = {0.0f, 0.0f};
        if (moveLeft())
            dir.x -= 1.0f;
        if (moveRight())
            dir.x += 1.0f;
        if (moveUp())
            dir.y -= 1.0f;
        if (moveDown())
            dir.y += 1.0f;
        return Vector2Normalize(dir);
    }

} // namespace Input
