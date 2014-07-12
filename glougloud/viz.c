#include <libglouglou.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <dirent.h>
#include <dlfcn.h>

#include "glougloud_internal.h"

#define MSG_WELCOME "Welcome to glougloud\n"

struct ggdmodviz {
	LIST_ENTRY(ggdmodviz) entry;
	void *handle;
	char *name;
	struct ggdmodviz_conf *conf;
	struct ggdmodviz_conf *(*ggdmodviz_init)(struct glougloud *);
	int (*ggdmodviz_text_get)(struct ggdviz_cli *, char *, int, char *, int, char **);
};

struct glougloud_viz {
	int pid;
	struct event_base *evb;
	LIST_HEAD(, ggdmodviz) mods;
	redisAsyncContext *rc;
	/* clients list */
	LIST_HEAD(, ggdviz_cli) clients;
	uint clients_count;
	uint clients_ids_counter;
	/* servers */
	struct {
		struct evconnlistener *listener;
	} servtcp;
};

struct glougloud *_ggd;
struct glougloud_viz *_viz;

static int
_modules_load(void)
{
	struct ggdmodviz *mod;
	DIR *d = NULL;
	struct dirent *ent;
	int n;
	void *handle, *sym;
	char path[MAXPATHLEN];
	char *dir;
	struct ggdmodviz_conf *modconf;
	struct ggdmodviz_conf *(*fn_ggdmodviz_init)(struct glougloud *);
	int (*fn_ggdmodviz_text_get)(struct ggdviz_cli *, char *, int, char *, int, char **);

    dir = GLOUGLOUD_MOD_PATH;
	d = opendir(dir);
	if (!d)
		goto err;
	while ((ent = readdir(d))) {
		n = strnlen(ent->d_name, sizeof(ent->d_name));
		if (!n || n < 2 || (ent->d_name[0] == '.'))
			continue;
		log_debug("viz: module %s to load", ent->d_name);
		snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
		handle = dlopen(path, RTLD_LAZY);
		log_tmp("m 1");
		if (!handle)
			continue;
		dlerror(); // clear errors

		log_tmp("m 2");
        if (!(sym = dlsym(handle, "ggdmodviz_init")))
            continue;
        fn_ggdmodviz_init = sym;
        dlerror(); // clear errors

		log_tmp("m 3");
        if (!(sym = dlsym(handle, "ggdmodviz_text_get")))
            continue;
        fn_ggdmodviz_text_get = sym;
        dlerror(); // clear errors

		log_tmp("m 4");
        modconf = fn_ggdmodviz_init(_ggd);
        if (!modconf)
            continue;
        if (modconf->api_version < GLOUGLOUD_MOD_API_VERSION) {
            log_warn("not loading mod %s: old api version %d, should be %d",
                    ent->d_name,
                    modconf->api_version, GLOUGLOUD_MOD_API_VERSION);
            continue;
        }

		log_tmp("m 5");
		mod = xcalloc(1, sizeof(struct ggdmodviz));
		mod->handle = handle;
		mod->name = strdup(ent->d_name);
        mod->conf = modconf;
		mod->ggdmodviz_init = fn_ggdmodviz_init;
		mod->ggdmodviz_text_get = fn_ggdmodviz_text_get;
		LIST_INSERT_HEAD(&_viz->mods, mod, entry);

		log_debug("viz: module %s done, id %d", mod->name, mod->conf->id);
	}

    closedir(d);
	return 0;

err:
    log_warn("viz: modules load failed");
    if (d)
        closedir(d);
	return -1;
}

/*
 * Redis notification callback
 *
 * algo to notify modules:
 * notif = extract reply->str
 * foreach module
 *   if ! module match dbname
 *     continue
 *   foreach client
 *     module_viz_send(cli, notif)
 */
/* "pmessage","__key*__:*","__keyevent@0__:set","foo" */
static void
_redis_cb_notification(redisAsyncContext *c, void *r, void *privdata)
{
	redisReply *reply;
	struct ggdviz_cli *cli;
	struct ggdmodviz *m;
	char *ntf_type, *ntf_pattern, *ntf_event_type, *ntf_op, *ntf_target;
	int ntf_db, ntf_op_len, ntf_target_len, res, len;
	char *s;

	reply = r;
	if (!reply)
		return;

    res = redis_parse_keyspace_notification(reply,
            &ntf_type, &ntf_pattern, &ntf_event_type, &ntf_db, &ntf_op, &ntf_op_len, &ntf_target, &ntf_target_len);
    if (res < 0) {
        log_info("viz: could not parse redis keyspace notification (%d) !\n"
                "Notification was: %s", res, reply->str);
        return;
    } else if (res < 4) {
        log_info("viz: redis notification not about db changes, ignoring.");
        return;
    }
    log_debug("viz: db change: %d %s %s", ntf_db, ntf_op, ntf_target);

	LIST_FOREACH(m, &_viz->mods, entry) {
		if (m->conf->id != ntf_db)
			continue;
		LIST_FOREACH(cli, &_viz->clients, entry) {
		    switch (cli->type) {
            case GGDVIZ_CLI_TCP:
                len = m->ggdmodviz_text_get(cli, ntf_op, ntf_op_len, ntf_target, ntf_target_len, &s);
                if (len <= 0) {
                    log_warn("module %s failed on ggdmodviz_text_get %s %s",
                            m->name, ntf_op, ntf_target);
                    break;
                }
                bufferevent_write(cli->tcp.bev, s, len);
                break;
            case GGDVIZ_CLI_WS:
                log_tmp("_redis_cb_notification: GGDVIZ_CLI_WS not implemented !");
                break;
            }
		}
	}
}

static void
_redis_cb_connect(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		log_warn("redis error: %s", c->errstr);
		return;
	}
	log_info("viz: redis connected");
}

static void
_redis_cb_disconnect(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		log_warn("redis error: %s", c->errstr);
		return;
	}
	log_info("viz: redis disconnected");
}

int
_redis_connect(void)
{
	_viz->rc = redis_connect(_viz->evb,
		_redis_cb_connect, _redis_cb_disconnect);
	if (_viz->rc->err)
		return -1;
	redisAsyncCommand(_viz->rc, _redis_cb_notification, "event",
		"PSUBSCRIBE __keyevent@*__:*");

	return 0;
}

void
_redis_disconnect(void)
{
    if (_viz->rc)
        redis_disconnect(_viz->rc);
	_viz->rc = NULL;
}


struct ggdviz_cli *
_servtcp_cli_add(struct sockaddr *sa, struct bufferevent *bev)
{
	struct ggdviz_cli *cli;

	cli = xcalloc(1, sizeof(struct ggdviz_cli));
	cli->id = _viz->clients_ids_counter;
	_viz->clients_ids_counter++;
	cli->type = GGDVIZ_CLI_TCP;
	cli->tcp.bev = bev;
	addr_ston(sa, &cli->tcp.addr);
	LIST_INSERT_HEAD(&_viz->clients, cli, entry);

	log_debug("viz: _servtcp_cli_add, cli %d %s",
		cli->id, addr_ntoa(&cli->tcp.addr));
	bufferevent_write(bev, MSG_WELCOME, strlen(MSG_WELCOME));

	return cli;
}

void
_servtcp_cli_del(struct ggdviz_cli *cli)
{
	bufferevent_free(cli->tcp.bev);
	LIST_REMOVE(cli, entry);
	_viz->clients_count--;
	free(cli);
}

static void
_servtcp_cb_read(struct bufferevent *bev, void *user_data)
{
	struct ggdviz_cli *cli;
	struct evbuffer *buf;
	char *line;
	size_t len;

	cli = user_data;
	buf = bufferevent_get_input(bev);
	line = evbuffer_readln(buf, &len, EVBUFFER_EOL_CRLF);
	log_debug("viz: _servtcp_cb_read, cli %d %s, len %d: %s",
		cli->id, addr_ntoa(&cli->tcp.addr), len, line);
}

static void
_servtcp_cb_event(struct bufferevent *bev, short events, void *user_data)
{
	struct ggdviz_cli *cli;

	cli = user_data;
	if (events & BEV_EVENT_EOF) {
		log_debug("viz: cli %d: connection closed", cli->id);
	} else if (events & BEV_EVENT_TIMEOUT) {
		log_debug("viz: cli %d: connection timeout", cli->id);
	} else if (events & BEV_EVENT_ERROR) {
		log_debug("viz: cli %d: connection error: %s", cli->id, strerror(errno));
	}
	_servtcp_cli_del(cli);
}

static void
_servtcp_cb_listener(struct evconnlistener *listener,
	evutil_socket_t fd, struct sockaddr *sa, int socklen,
	void *user_data)
{
	struct ggdviz_cli *cli;
	struct bufferevent *bev;

	bev = bufferevent_socket_new(_viz->evb, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		log_warn("viz: error constructing bufferevent !");
		return;
	}

	cli = _servtcp_cli_add(sa, bev);

	bufferevent_setcb(bev, _servtcp_cb_read, NULL,
		_servtcp_cb_event, cli);
	bufferevent_enable(bev, EV_WRITE|EV_READ);
}

int
_servtcp_start(void)
{
	struct evconnlistener *listener;
	struct sockaddr_in sin;

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
	addr_ntos(&_ggd->viz.serv_ip, (struct sockaddr *)&sin);
        sin.sin_port = htons(_ggd->viz.serv_port);

	listener = evconnlistener_new_bind(_viz->evb,
		_servtcp_cb_listener, NULL,
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin, sizeof(sin));

	if (!listener)
		return -1;
	_viz->servtcp.listener = listener;

	return 0;
}

void
_servtcp_stop(void)
{
	struct ggdviz_cli *cli, *clitmp;

	if (_viz->servtcp.listener)
		evconnlistener_free(_viz->servtcp.listener);
	_viz->servtcp.listener = NULL;

	LIST_FOREACH_SAFE(cli, &_viz->clients, entry, clitmp)
		if (cli->type == GGDVIZ_CLI_TCP)
			_servtcp_cli_del(cli);
}

int
viz_init(struct glougloud *ggd)
{
	_ggd = ggd;
	_viz = xcalloc(1, sizeof(struct glougloud_viz));
	_viz->pid = fork();
	if (_viz->pid > 0)
		return 0;
	setprocname("viz");
	droppriv(GLOUGLOUD_USER_VIZ, 1, NULL);

	_viz->evb = event_base_new();
	if (_modules_load() < 0)
		goto err;
	if (_redis_connect() < 0)
		goto err;
	if (_servtcp_start() < 0)
		goto err;

	event_base_dispatch(_viz->evb);

	exit(0);

err:
	viz_shutdown();
	log_fatal("viz error");
	exit(-1); /* UNREACHED */
}

void
viz_shutdown(void) {
	_redis_disconnect();
	_servtcp_stop();
}

