#include "network.h"
#include <enet/enet.h>

namespace network {
int initialize() {
    return enet_initialize();
}

void terminate() {
    enet_deinitialize();
}
}
