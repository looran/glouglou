#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

#include <libglouglou.h>

#include "glougloud_internal.h"

struct glougloud_redis {
	int pid;
};

struct glougloud *_ggd = NULL;
struct glougloud_redis *_redis;

int
redis_init(struct glougloud *ggd) {
	char redis_conf[4096];
	char *echo_args[] = {"echo", redis_conf, NULL};
	char *redis_args[] = {"glougloud: redis", "-", NULL};
	char newpath[4096];
	char *path;

	_ggd = ggd;
	_redis = xcalloc(1, sizeof(struct glougloud_redis));
	_redis->pid = fork();
	if (_redis->pid > 0)
		return 0;
	droppriv(GLOUGLOUD_USER_PROBES, 0, NULL);
	path = getenv("PATH");
	snprintf(newpath, sizeof(newpath),
		"%s:/bin:/sbin:/usr/local/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin", path);
	setenv("PATH", newpath, 1);

	snprintf(redis_conf, sizeof(redis_conf),
		"daemonize no\n"
		"pidfile /var/run/glougloud/redis.pid\n"
		"port 0\n"
		"unixsocket %s\n"
		"unixsocketperm 770\n"
		"timeout 0\n"
		"loglevel notice\n"
		/* XXX for the moment we log in glougloud log
		 * "logfile /var/log/glougloud/redis.log\n" */
		"databases 16\n"
		"save 900 1\n"
		"save 300 10\n"
		"save 60 10000\n"
		"rdbcompression yes\n"
		"dbfilename glougloud_dump.rdb\n"
		"dir /var/lib/glougloud/\n"
		"slowlog-log-slower-than 10000\n"
		"slowlog-max-len 1024\n"
		"notify-keyspace-events KEA\n",
		_ggd->redis.socket);
	exec_pipe("echo", echo_args, "redis-server", redis_args);

	log_warn("error starting redis server");
	exit(EXIT_FAILURE);
}

void
redis_shutdown(void) {
	log_debug("redis shutdown ...");
	kill_wait(_redis->pid, 10);
	log_info("redis shutdown ok");
}

redisAsyncContext *
redis_connect(struct event_base *evb,
	void (*cb_connect)(const redisAsyncContext *, int),
	void (*cb_disconnect)(const redisAsyncContext *, int))
{
	redisAsyncContext *rc;

	do {
		rc = redisAsyncConnectUnix(_ggd->redis.socket_chrooted);
		if (rc->err) {
			log_warn("redis connect: %s", rc->errstr);
			sleep(1);
		}
	} while (rc->err);
	redisLibeventAttach(rc, evb);
	redisAsyncSetConnectCallback(rc, cb_connect);
	redisAsyncSetDisconnectCallback(rc, cb_disconnect);

	return rc;
}

void
redis_disconnect(redisAsyncContext *rc)
{
	redisAsyncDisconnect(rc);
	redisAsyncFree(rc);
}

/* Parse redis keyspace notification message
 * WARNING: modifies notification string
 *
 * Example notification string:
 * '"pmessage","__key*__:*","__keyevent@0__:set","foo"'
 *
 * 1379432174 viz: _redis_cb_notification: 2 4, (null)
 * 1379432174 element0: 1 0 pmessage
 * 1379432174 element1: 1 0 __key*@*__:*
 * 1379432174 element2: 1 0 __keyevent@0__:sadd
 * 1379432174 element3: 1 0 /n/192.168.1.3
 */
int
redis_parse_keyspace_notification(redisReply *reply,
        char **ntf_type, char **ntf_pattern, char **ntf_event_type,
        int *ntf_db, char **ntf_op, int *ntf_op_len,
        char **ntf_target, int *ntf_target_len)
{
    redisReply *r;
    int i;
    char *tmp;
    static char l_ntf_event_type[64];
    static char l_ntf_op[64];

    log_debug("redis_parse_keyspace_notification %d with %d elements", reply->type, reply->elements);

    if (reply->type != REDIS_REPLY_ARRAY) {
        log_info("parse_redis_keyspace_notification: not REDIS_REPLY_ARRAY !");
        return -1;
    }
    if (reply->elements < 1) {
        log_info("parse_redis_keyspace_notification: Only %d elements instead of 4 !", reply->elements);
        return -2;
    }
    *ntf_type = *ntf_pattern = *ntf_event_type = *ntf_op = *ntf_target = NULL;
    *ntf_db = -1;

    for (i=0; i<reply->elements; i++) {
        r = reply->element[i];
        log_debug("element%d: %d %d %s", i, r->type, r->elements, r->str);
        switch (r->type) {
        case REDIS_REPLY_STRING:
            switch (i) {
            case 0: /* pmessage */
                *ntf_type = r->str;
                break;
            case 1: /* __key*@*__:* */
                *ntf_pattern = r->str;
                break;
            case 2: /* __keyevent@0__:sadd */
                tmp = strtok(r->str + 2, "@");
                memcpy(l_ntf_event_type, tmp, strlen(tmp) + 1);
                *ntf_event_type = l_ntf_event_type;
                tmp = strtok(NULL, "_");
                if (!tmp) return -3;
                *ntf_db = atoi(tmp);
                if (*ntf_db < 0) return -4;
                tmp = strtok(NULL, ":");
                if (!tmp) return -5;
                tmp = strtok(NULL, "\0");
                if (!tmp) return -6;
                memcpy(l_ntf_op, tmp, strlen(tmp) + 1);
                *ntf_op = l_ntf_op;
                *ntf_op_len = strlen(l_ntf_op);
                break;
            case 3: /* /n/192.168.1.3 */
                *ntf_target = r->str;
                *ntf_target_len = r->len;
                break;
            }
        case REDIS_REPLY_INTEGER:
            /* ignored */
            break;
        }
    }

    return reply->elements;
}
