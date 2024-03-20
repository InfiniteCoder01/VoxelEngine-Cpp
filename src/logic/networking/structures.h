#pragma once
#include "../../world/Level.h"
#include "../../coders/byte_utils.h"
#include <enet/enet.h>
#include <functional>

enum class MessageType : uint16_t {
    // Client requests to join the server
    Join,
    // Sent in response to Join. Contains the initial world information (time, player position, etc.)
    WorldInfo,
    // Sync with the server from time to time (sent every second)
    Sync,
};

ByteBuilder handshake(MessageType type);
void serializeDateTime(ByteBuilder& builder, Level* level);

void checkHandshake(ENetPacket* packet, std::function<void(MessageType, ByteReader&)> f);
void deserializeDateTime(ByteReader& reader, Level* level);

ENetPacket* pack(const ByteBuilder& builder, enet_uint32 flags);
