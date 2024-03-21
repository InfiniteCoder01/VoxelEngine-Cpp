#include "structures.h"
#include "settings.h"
#include "../../world/World.h"
#include "../../frontend/locale/langs.h"
#include <stdexcept>
#include <string.h>

ByteBuilder handshake(MessageType type) {
    ByteBuilder builder;
    builder.put((const ubyte*)HANDSHAKE_MAGIC, strlen(HANDSHAKE_MAGIC));
    builder.putInt32(HANDSHAKE_VERSION);
    builder.putInt16((uint16_t)type);
    return builder;
}

void serializeDateTime(ByteBuilder& builder, Level* level) {
    builder.putFloat32(level->world->daytime);
    builder.putFloat32(level->world->daytimeSpeed);
    builder.putFloat64(level->world->totalTime);
}

// * Deserialize Part

void checkHandshake(ENetPacket* packet, std::function<void(MessageType, ByteReader&)> f) {
    ByteReader reader(packet->data, packet->dataLength);
    try {
        reader.checkMagic(HANDSHAKE_MAGIC, strlen(HANDSHAKE_MAGIC));
        uint32_t version = reader.getInt32();
        if (version < 1 || version > HANDSHAKE_VERSION) throw std::runtime_error("Invalid handshake version");
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        throw langs::get(L"error.unsupported-server");
    }
    try {
        MessageType type = (MessageType)reader.getInt16();
        f(type, reader);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        throw langs::get(L"error.could-not-load-from-server");
    }
}

void deserializeDateTime(ByteReader& reader, Level* level) {
    level->world->daytime = reader.getFloat32();
    level->world->daytimeSpeed = reader.getFloat32();
    level->world->totalTime = reader.getFloat64();
}

ENetPacket* pack(const ByteBuilder& builder, enet_uint32 flags) {
    return enet_packet_create(builder.data(), builder.size(), flags);
}

