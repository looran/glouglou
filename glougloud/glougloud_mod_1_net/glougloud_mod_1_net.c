#include <glougloud.h>

#include <libglouglou.h>

#define GLOUGLOUD_MOD_NET_ID 1

struct ggdmodviz_conf *
ggdmodviz_init(struct glougloud *ggd)
{
    struct ggdmodviz_conf *mod;

    log_debug("glougloud_mod_net: viz init");

    mod = xcalloc(1, sizeof(struct ggdmodviz_conf));
    mod->id = GLOUGLOUD_MOD_NET_ID;
    mod->api_version = 1;

    return mod;
}

int
ggdmodviz_text_get(struct ggdviz_cli *cli, char *op, int op_len,
        char *target, int target_len, char **resp)
{
    static char text[4096];
    int len;

    log_debug("glougloud_mod_net:\nop: %s\ntarget: %s", op, target);
    text[0] = '\0';
    *resp = text;
    if (target_len < 4)
        return -1;

    switch(target[1]) {
        case 'n': 
            len = snprintf(text, sizeof(text), "New node %s\n", target+3);
            break;
        case 'N': 
            len = snprintf(text, sizeof(text), "Delete node %s\n", target+3);
            break;
        default:
            log_debug("ggdmodviz_text_get: unsupported target %s", target);
            len = -1;
    }

    return len;
}

void *
ggdmodviz_netstream_get(struct ggdviz_cli *cli, char *notification)
{
    return NULL;
}

int
ggdmodviz_conf_set(struct ggdviz_cli *cli, void *conf)
{
    return -1;
}

int
ggmodprobe_init(struct glougloud *ggd)
{
    log_debug("glougloud_mod_net: probe init");
    return 0;
}

char *
ggdmodprobe_redis_get(struct ggdprobe_cli *prb,
	struct gg_packet *pkt)
{
    return NULL;
}
