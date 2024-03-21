#include "ClientController.h"
#include "networking/structures.h"
#include "../frontend/locale/langs.h"
#include "../files/rle.h"
#include "../world/World.h"
#include "../world/WorldGenerators.h"
#include "../lighting/Lighting.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../voxels/Chunk.h"
#include "../voxels/ChunksStorage.h"
#include "../voxels/ChunksMatrix.h"
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
        player = level->getObject<Player>(0).get();

        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
           checkHandshake(event.packet, [&] (auto type, auto& reader) {
                if (type == MessageType::WorldInfo) {
                    deserializeDateTime(reader, level);
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
    enet_peer_send(peer, 0, pack(handshake(MessageType::Leave), ENET_PACKET_FLAG_RELIABLE));
    enet_host_flush(client);
    enet_peer_reset(peer);
    enet_host_destroy(client);
}

std::unique_ptr<ubyte[]> ClientController::decompress(ByteReader& reader, size_t size) {
    std::unique_ptr<ubyte[]> decompressed = std::make_unique<ubyte[]>(size);
    size_t srcsize = reader.getInt64();
    extrle::decode(reader.pointer(), srcsize, decompressed.get());
    reader.skip(srcsize);
    return decompressed;
}

void ClientController::update() {
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            checkHandshake(event.packet, [&] (auto type, auto& reader) {
                if (type == MessageType::Sync) {
                    deserializeDateTime(reader, level);
                } else if (type == MessageType::Chunk) {
                    auto x = reader.getInt32();
                    auto z = reader.getInt32();
                    auto chunk = std::make_shared<Chunk>(x, z);
                    level->chunksStorage->store(chunk);

                    {
                        auto data = decompress(reader, CHUNK_DATA_LEN);
                        chunk->decode(data.get());
                        // auto invs = wfile->fetchInventories(chunk->x, chunk->z);
                        // chunk->setBlockInventories(std::move(invs));
                        chunk->setLoaded(true);
                        // for(auto& entry : chunk->inventories) {
                        //     level->inventories->store(entry.second);
                        // }
                    }

                    if (reader.get()) {
                        auto data = decompress(reader, LIGHTMAP_DATA_LEN);
                        std::unique_ptr<light_t[]> lights (Lightmap::decode(data.get()));
                        chunk->lightmap.set(lights.get());
                        chunk->setLoadedLights(true);
                    }

                    player->chunksMatrix->putChunk(chunk);
                    chunk->updateHeights();

                    if (!chunk->isLoadedLights()) {
                        Lighting::prebuildSkyLight(
                            chunk.get(), level->content->getIndices()
                        );
                    }
                    chunk->setReady(true);
                }
            });
            enet_packet_destroy(event.packet);
        }
    }
}
