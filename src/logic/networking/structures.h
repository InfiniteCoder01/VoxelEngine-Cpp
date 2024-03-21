#pragma once
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
    /// @brief Chunk data
    Chunk,
};

ByteBuilder handshake(MessageType type);
void serializeDateTime(ByteBuilder& builder, Level* level);

void checkHandshake(ENetPacket* packet, std::function<void(MessageType, ByteReader&)> f);
void deserializeDateTime(ByteReader& reader, Level* level);

ENetPacket* pack(const ByteBuilder& builder, enet_uint32 flags);
