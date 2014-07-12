#include <unistd.h>

#include <libglouglou.h>

#include "glougloud_internal.h"

struct modprobe {
	LIST_ENTRY(mod) entry;
	void *handle;
	char *name;
	int id;
	char (*ggdmodprobe_redis_get)(struct ggdprobe_cli *prb, struct gg_packet *pkt);
};

struct glougloud_probes {
	int pid;
	struct event_base *evb;
	LIST_HEAD(, modprobe) mods;
	redisAsyncContext *rc;
	struct gg_server *server;
};

struct glougloud *_ggd;
struct glougloud_probes *_probes;

static void
cb_connect(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		log_warn("redis error: %s", c->errstr);
		return;
	}
	log_info("probes: redis connected");
}

static void
cb_disconnect(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		log_warn("redis error: %s", c->errstr);
		return;
	}
	log_info("probes: redis disconnected");
}

static int
prb_handle_conn(struct gg_server *srv, struct gg_user *usr)
{
	// XXX
	return 0;
}

static int
prb_handle_packet(struct gg_server *srv, struct gg_user *usr, struct gg_packet *pkt)
{
	// XXX
	return 0;
}

static int 
_modules_load(void)
{
    return 0;
}

int
probes_init(struct glougloud *ggd) {
	_ggd = ggd;
	_probes = xcalloc(1, sizeof(struct glougloud_probes));
	_probes->pid = fork();
	if (_probes->pid > 0)
		return 0;
	setprocname("probes");
	droppriv(GLOUGLOUD_USER_PROBES, 1, NULL);

	_probes->evb = event_base_new();

	if (_modules_load() < 0)
	    goto err;

	_probes->rc = redis_connect(_probes->evb, cb_connect, cb_disconnect);
	if (_probes->rc->err)
		return -1;

	_probes->server = gg_server_start(_probes->evb,
		&ggd->probes.serv_ip, ggd->probes.serv_port,
		prb_handle_conn, prb_handle_packet, NULL);
	if (!_probes->server) {
		log_warn("probes: gg_server_start failed");
		return -1;
	}

	event_base_dispatch(_probes->evb);

	gg_server_stop(_probes->server);

	return 0;

err:
    probes_shutdown();
    return -1;
}

void
probes_shutdown(void) {
}

