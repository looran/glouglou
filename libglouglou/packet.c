#include <stdarg.h>
#include <stdlib.h>

#include "libglouglou.h"

struct ggpkt *ggpkt_new(int module_id, int module_version,
    void *body, int (*fn_body_len)(struct ggpkt *))
{
    struct ggpkt *pkt;
    pkt = xcalloc(1, sizeof(struct ggpkt));
    pkt->body = body;
    pkt->body_len = fn_body_len;
    ggpkt_header_encode((struct ggpkt_header *)body,
        module_id, module_version);

    return pkt;
}

int
ggpkt_header_encode(struct ggpkt_header *header,
    int module_id, int module_version)
{
    header->module_id = module_id;
    header->module_version = module_version;
    return 0;
}

int
ggpkt_header_decode(struct ggpkt_header *header,
    int *module_id, int *module_version)
{
    *module_id = header->module_id;
    *module_version = header->module_version;
    return 0;
}
    
