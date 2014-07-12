#include <string.h>
#include <libglouglou.h>
#include "../glougloud_internal.h"

int
main(void)
{
    int res;
    char *ntf_type, *ntf_pattern, *ntf_event_type, *ntf_op, *ntf_target;
    int ntf_db;
    redisReply *reply;

    reply = xcalloc(1, sizeof(redisReply));
    reply->type = REDIS_REPLY_ARRAY;
    reply->elements = 4;
    reply->element = xcalloc(4, sizeof(redisReply *));
    reply->element[0] = xcalloc(1, sizeof(redisReply));
    reply->element[0]->type = REDIS_REPLY_STRING;
    reply->element[1] = xcalloc(1, sizeof(redisReply));
    reply->element[1]->type = REDIS_REPLY_STRING;
    reply->element[2] = xcalloc(1, sizeof(redisReply));
    reply->element[2]->type = REDIS_REPLY_STRING;
    reply->element[3] = xcalloc(1, sizeof(redisReply));
    reply->element[3]->type = REDIS_REPLY_STRING;

#define test_num 1
    reply->element[0]->str = strdup("pmessage");
    reply->element[1]->str = strdup("__key*@*__:*");
    reply->element[2]->str = strdup("__keyevent@0__:sadd");
    reply->element[3]->str = strdup("/n/192.168.1.3");
    res = redis_parse_keyspace_notification(reply,
            &ntf_type, &ntf_pattern, &ntf_event_type,
            &ntf_db, &ntf_op, &ntf_target);
    if ( res < 0
            || strcmp(ntf_type, "pmessage")
            || strcmp(ntf_pattern, "__key*@*__:*")
            || strcmp(ntf_event_type, "keyevent")
            || (ntf_db != 0)
            || strcmp(ntf_op, "sadd")
            || strcmp(ntf_target, "/n/192.168.1.3") ) {
        printf("Error on %d (%d): %s %s %s %d %s %s\n", test_num, res,
            ntf_type, ntf_pattern, ntf_event_type, ntf_db, ntf_op, ntf_target);
    }

    return 0;
}
