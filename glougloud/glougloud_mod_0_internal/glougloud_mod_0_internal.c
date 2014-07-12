#include <glougloud.h>
#include <libglouglou.h>
#include <libglouglou/modules/libglouglou_mod_0_internal.h>

struct ggdmodviz_conf *
ggdmodviz_init(struct glougloud *ggd)
{
    struct ggdmodviz_conf *mod;

    log_debug("glougloud_mod_internal: viz init");

    mod = xcalloc(1, sizeof(struct ggdmodviz_conf));
    mod->id = GLOUGLOUD_MOD_0_INTERNAL_ID;
    mod->api_version = 1;

    return mod;
}

int
ggdmodviz_text_get(struct ggdviz_cli *cli, char *op, int op_len,
        char *target, int target_len, char **resp)
{
    log_debug("glougloud_mod_internal:\nop: %s\ntarget: %s", op, target);
    return -1;
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
    log_debug("glougloud_mod_internal: probe init");
    return 0;
}

char *
ggdmodprobe_redis_get(struct ggdprobe_cli *prb,
	struct gg_packet *pkt)
{
    return NULL;
}
