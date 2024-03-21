#pragma once
#include <glm/glm.hpp>
#include "../../world/Level.h"
#include "../../coders/byte_utils.h"
#include <enet/enet.h>
#include <functional>

enum class MessageType : uint16_t {
    /// @brief Client requests to join the server
    Join,
    /// @brief Client leaves the server. Useful, because server detects disconnection after like 20 seconds
    Leave,
    /// @brief Sent in response to Join. Contains the initial world information (time, player position, etc.)
    WorldInfo,
    /// @brief Sync with the server from time to time (sent every second)
    Sync,
    /// @brief Update settings
    Settings,
    /// @brief Chunk data
    Chunk,
    /// @brief Object movement
    ObjectMove,
};

ByteBuilder handshake(MessageType type);
void serializeDateTime(ByteBuilder& builder, Level* level);
void serializeVec3(ByteBuilder& builder, glm::vec3 vec);

void checkHandshake(ENetPacket* packet, std::function<void(MessageType, ByteReader&, uint32_t)> f);
void deserializeDateTime(ByteReader& reader, Level* level);
glm::vec3 deserializeVec3(ByteReader& reader);

ENetPacket* pack(const ByteBuilder& builder, enet_uint32 flags);
