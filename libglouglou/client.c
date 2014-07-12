#include "libglouglou.h"
    
int
gg_client_send(struct gg_client *cli, int module_id, struct ggpkt *pkt)
{
    if (!pkt) {
        log_warn("gg_client_send called with NULL pkt !");
        return -1;
    }

    // XXX
    return -1;
}
