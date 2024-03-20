#pragma once
#include <stddef.h>
#include <enet/enet.h>

const size_t MAX_CLIENTS = 32;
const size_t MAX_CHANNELS = 2;

const enet_uint32 SERVER_BANDWIDTH_IN = 0;
const enet_uint32 SERVER_BANDWIDTH_OUT = 0;

const enet_uint32 CLIENT_BANDWIDTH_IN = 0;
const enet_uint32 CLIENT_BANDWIDTH_OUT = 0;

const char* const HANDSHAKE_MAGIC = "VE-CppHSH";
const uint32_t HANDSHAKE_VERSION = 1;

const uint64_t SYNC_INTERVAL = 1000000ul;
