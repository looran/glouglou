/* glougloud internal */

#include <dnet.h>
#include <event.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>

#include "glougloud.h"

#define GLOUGLOUD_VERSION 3
#define GLOUGLOUD_MOD_API_VERSION 1

#define GLOUGLOUD_USER_PROBES "_ggdprobe"
#define GLOUGLOUD_USER_VIZ "_ggdviz"
#define GLOUGLOUD_LOGFILE "/var/log/glougloud.log"
#define GLOUGLOUD_MOD_PATH "/modules/"

/* redis.c */

int  redis_init(struct glougloud *);
void redis_shutdown(void);

redisAsyncContext *redis_connect(struct event_base *,
	void (*cb_connect)(const redisAsyncContext *, int),
	void (*cb_disconnect)(const redisAsyncContext *, int));
void redis_disconnect(redisAsyncContext *);

int
redis_parse_keyspace_notification(redisReply *reply,
        char **ntf_type, char **ntf_pattern, char **ntf_event_type,
        int *ntf_db, char **ntf_op, int *ntf_op_len,
        char **ntf_target, int *ntf_target_len);

/* probes.c */

int  probes_init(struct glougloud *);
void probes_shutdown(void);

/* viz.c */

int  viz_init(struct glougloud *);
void viz_shutdown(void);
