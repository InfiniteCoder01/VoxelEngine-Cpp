#pragma once
#include "networking/settings.h"
#include "../world/Level.h"
#include "../engine.h"
#include <string>

class ClientController {
    ENetAddress address;
    ENetHost* client;
    ENetPeer* peer;
public:
    Level* level;

    ClientController(Engine* engine, const std::string& addr, const std::string& name);
    ~ClientController();

    void update();
};
