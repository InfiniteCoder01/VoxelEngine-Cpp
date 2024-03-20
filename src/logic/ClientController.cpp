#include "ClientController.h"
#include "networking/structures.h"
#include "../frontend/locale/langs.h"
#include "../world/World.h"
#include "../world/WorldGenerators.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include <iostream>

ClientController::ClientController(Engine* engine, const std::string& addr, const std::string& name) {
    { // Connect to the server
        client = enet_host_create(nullptr, 1, MAX_CHANNELS, CLIENT_BANDWIDTH_IN, CLIENT_BANDWIDTH_OUT);
        if (!client) {
            throw langs::get(L"error.could-not-create-host");
        }
        auto colon = addr.find(":");
        enet_address_set_host(&address, addr.substr(0, colon).c_str());
        address.port = std::stoi(addr.substr(colon + 1));
        peer = enet_host_connect(client, &address, MAX_CHANNELS, 0);
        if (!peer) {
            throw langs::get(L"error.could-not-connect");
        }
    }

    ENetEvent event;
    // Wait for the connect event
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connected to peer!" << std::endl;
    } else {
        throw langs::get(L"error.could-not-connect");
    }

    { // Auth
        ByteBuilder packet = handshake(MessageType::Join);
        packet.put(name);
        packet.putInt32(engine->getSettings().chunks.loadDistance);
        enet_peer_send(peer, 0, pack(packet, ENET_PACKET_FLAG_RELIABLE));
    }
    { // Load the world
        engine->loadAllPacks();
        engine->loadContent();
        level = World::create(
            "Server", WorldGenerators::getDefaultGeneratorID(), "", 0, 
            engine->getSettings(), 
            engine->getContent(),
            engine->getContentPacks()
        );

        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
           checkHandshake(event.packet, [&] (auto type, auto& reader) {
                if (type == MessageType::WorldInfo) {
                    deserializeDateTime(reader, level);
                    auto player = level->getObject<Player>(0);
                    player->hitbox->position = glm::vec3(reader.getFloat32(), reader.getFloat32(), reader.getFloat32());
                } else {
                    throw langs::get(L"error.could-not-load-from-server");
                }
            });
            enet_packet_destroy(event.packet);
        } else {
            throw langs::get(L"error.could-not-load-from-server");
        }
    }
}

ClientController::~ClientController() {
    enet_peer_reset(peer);
    enet_host_destroy(client);
}

void ClientController::update() {
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            checkHandshake(event.packet, [&] (auto type, auto& reader) {
                if (type == MessageType::Sync) {
                    deserializeDateTime(reader, level);
                }
            });
            enet_packet_destroy(event.packet);
        }
    }
}
