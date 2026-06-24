#ifndef GAME_COMMAND_DRAFT_H
#define GAME_COMMAND_DRAFT_H

// =============================================================================
// DRAFT. Defines the command object that travels Dashboard -> Core 0 -> Core 1.
// This header is inert until it is #included somewhere (Arduino only compiles
// .cpp/.ino/.c files; an unincluded .h has no effect on the build).
// =============================================================================

#include <stdint.h>
#include "../physics/PhysicsEngine.h" // provides PhysicsParams

/** What the dashboard is asking the device to do. Parsed ONCE on Core 0 from
 *  the "command" string, then passed as an enum so Core 1 never re-parses text. */
enum class CommandType : uint8_t
{
    None = 0,
    Start, // start the selected game
    Stop   // stop / end the current game
    // room to grow: Pause, Restart, SelectGame, ...
};

/** Fixed maximum length for the player name (including the terminating '\0'). */
static const uint8_t kMaxPlayerNameLen = 24;

/**
 * @brief One command from a dashboard, ready to travel through a FreeRTOS queue.
 *
 * IMPORTANT: this is a plain, trivially-copyable struct (POD). xQueueSend copies
 * it byte-for-byte (memcpy), so it must contain NO String, std::string, pointer
 * to heap data, or virtual method. Strings are fixed-size char arrays. The Core 0
 * MQTT callback fills the fields; the Core 1 game loop reads a copy.
 *
 * The "= default" member initialisers keep a half-filled payload sane and do not
 * affect trivial copyability (they only touch default construction, not copying).
 */
struct GameCommand
{
    CommandType type = CommandType::None;

    uint8_t gameId = 1; // game.game_id (1 = Labyrinth, 2 = Flatland)

    uint16_t cookiesCount = 10;  // parameters.cookies_count
    uint8_t wallThicknessPx = 6; // parameters.wall_thickness_px

    char playerName[kMaxPlayerNameLen] = {0}; // player.name, always '\0'-terminated

    PhysicsParams physics; // physics.* -> applied directly via engine.setParams()
};

#endif // GAME_COMMAND_DRAFT_H
