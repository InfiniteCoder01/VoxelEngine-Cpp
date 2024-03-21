#pragma once
#include "networking/settings.h"
#include "../coders/byte_utils.h"
#include "../world/Level.h"
#include "../engine.h"
#include <string>

class ClientController {
    ENetAddress address;
    ENetHost* client;
    ENetPeer* peer;

    std::unique_ptr<ubyte[]> decompress(ByteReader& reader, size_t size);
public:
    Level* level;
    Player* player;

    ClientController(Engine* engine, const std::string& addr, const std::string& name);
    ~ClientController();

    void update();
};
