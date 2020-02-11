#include "router_hal.h"

// configure this to match the output of `ip a`
const char *interfaces[N_IFACE_ON_BOARD] = {
#ifndef USE_NETNS
  "eth1", "eth2",
#elif ROUTER_NUMBER == 1
  "a2", "b2",
#elif ROUTER_NUMBER == 3
  "c4", "d4",
#else
  "b3", "c3",
#endif
};
