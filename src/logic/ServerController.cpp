#include "ServerController.h"
#include "networking/settings.h"
#include "networking/structures.h"
#include "../world/World.h"
#include "../files/WorldFiles.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../items/Inventories.h"
#include "../frontend/locale/langs.h"
#include <iostream>

ServerController::ServerController(Level* level)
    : level(level),
      settings(level->settings) {
    address.host = ENET_HOST_ANY;
    address.port = 1234;
    server = enet_host_create(&address, MAX_CLIENTS, MAX_CHANNELS, SERVER_BANDWIDTH_IN, SERVER_BANDWIDTH_OUT);
    if (!server) {
        throw langs::get(L"error.could-not-create-host");
    }
    std::cout << "Server open on port " << address.port << std::endl;
}

ServerController::~ServerController() {
    enet_host_destroy(server);
}

void ServerController::disconnect(ENetPeer* peer) {
    auto player = (Player*)peer->data;
    level->world->wfile->writePlayer(player);
    std::cout << player->getName() << " left the game!" << std::endl;
    level->removeObject(player->getId());
    peer->data = nullptr;
}

void ServerController::update() {
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            try {
               checkHandshake(event.packet, [&] (auto type, auto& reader) {
                    if (type == MessageType::Join) {
                        std::string name = reader.getString();
                        std::shared_ptr<Player> player = nullptr;
                        for (const auto& object : level->objects) {
                            if (std::shared_ptr<Player> playerObject = std::dynamic_pointer_cast<Player>(object)) {
                                if (playerObject->getName() == name) {
                                    player = playerObject;
                                    break;
                                }
                            }
                        }
                        if (!player) {
                            player = level->spawnObject<Player>(level, name, glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED,
                                                                level->inventories->create(DEF_PLAYER_INVENTORY_SIZE), settings);
                            level->world->wfile->readPlayer(player.get(), name);
                        }

                        std::cout << name << " joined the game!" << std::endl;
                        event.peer->data = player.get();

                        player->radius = reader.getInt32();
                        // player->chunksMatrix->events;

                        ByteBuilder builder = handshake(MessageType::WorldInfo);
                        serializeDateTime(builder, level);
                        builder.putFloat32(player->hitbox->position.x);
                        builder.putFloat32(player->hitbox->position.y);
                        builder.putFloat32(player->hitbox->position.z);
                        enet_peer_send(event.peer, 0, pack(builder, ENET_PACKET_FLAG_RELIABLE));
                    } else if (type == MessageType::Leave) {
                        disconnect(event.peer);
                        enet_peer_reset(event.peer);
                    }
                });
            } catch (...) {}
            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            disconnect(event.peer);
        }
    }

    if (syncTimer.stop() >= SYNC_INTERVAL) {
        ByteBuilder builder = handshake(MessageType::Sync);
        serializeDateTime(builder, level);
        enet_host_broadcast(server, 0, pack(builder, 0));
        syncTimer.reset();
    }
}
