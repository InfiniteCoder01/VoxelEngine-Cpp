#pragma once
#include "networking/settings.h"
#include "../world/Level.h"
#include "../util/timeutil.h"
#include "../coders/byte_utils.h"

class ServerController {
    ENetAddress address;
    ENetHost* server;

    timeutil::Timer syncTimer;

    Level* level;
    const EngineSettings& settings;

    std::unique_ptr<ubyte[]> compressionBuffer;

    void disconnect(ENetPeer* peer);
    void compress(ByteBuilder& dst, const ubyte* const src, size_t srclen);
public:
    ServerController(Level* level);
    ~ServerController();

    void update();
};
