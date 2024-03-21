#pragma once
#include "networking/settings.h"
#include "../world/Level.h"
#include "../util/timeutil.h"

class ServerController {
    ENetAddress address;
    ENetHost* server;

    timeutil::Timer syncTimer;

    Level* level;
    const EngineSettings& settings;

    void disconnect(ENetPeer* peer);
public:
    ServerController(Level* level);
    ~ServerController();

    void update();
};
