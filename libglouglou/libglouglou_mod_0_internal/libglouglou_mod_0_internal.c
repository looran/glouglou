#include <stdlib.h>
#include <string.h>

#include "libglouglou_mod_0_internal.h"

#define NEWPKT(newpkt_pkt, newpkt_body, newpkt_type)                \
    struct ggpkt *newpkt_pkt;                                       \
    struct ggpkt_mod_0_internal *newpkt_body;                       \
                                                                    \
    newpkt_body = xcalloc(1, sizeof(struct ggpkt_mod_0_internal));  \
    newpkt_body->type = newpkt_type;                                \
    newpkt_pkt = ggpkt_new(GGPKT_MOD_0_INTERNAL_ID,                 \
            GGPKT_MOD_0_INTERNAL_VERSION,                           \
            newpkt_body, _get_len);

struct ggpkt *
ggpkt_mod_0_internal_hello(void)
{
    NEWPKT(pkt, body, GGPKT_MOD_0_INTERNAL_HELLO)
    body->data.hello.random_probe = htonl(rand());
    return pkt;
}

struct ggpkt *
ggpkt_mod_0_internal_hello_res(void)
{
    NEWPKT(pkt, body, GGPKT_MOD_0_INTERNAL_HELLO_RES)
    body->data.hello.random_ggd = htonl(rand());
    return pkt;
}

struct ggpkt *
ggpkt_mod_0_internal_hello_auth(int probe_id, char *pass,
    int random_probe, int random_ggd)
{
    int len;

    len = strlen(pass);
    if (len > GGPKT_ARG_MAX)
        return -1;

    NEWPKT(pkt, body, GGPKT_MOD_0_INTERNAL_AUTH)
    body->data.auth.probe_id = htons(probe_id);
    body->data.auth.probe_password_len = len;
    memcpy(body->data.auth.probe_password, pass, len+1);
    body->data.hello.random_probe = htonl(random_probe);
    body->data.hello.random_ggd = htonl(random_ggd);
    return body;
}

struct ggpkt *
ggpkt_mod_0_internal_auth_res(int status)
{
    NEWPKT(pkt, body, GGPKT_MOD_0_INTERNAL_AUTH_RES)
    body->data.auth_res.status = status;
    return pkt;
}

struct ggpkt *
ggpkt_mod_0_internal_notice(char *notice_msg)
{
    int len;

    len = strlen(notice_msg);
    if (len > GGPKT_ARG_MAX)
        return -1;

    NEWPKT(pkt, body, GGPKT_MOD_0_INTERNAL_NOTICE)
    body->data.notice.len = len;
    memcpy(body->data.notice.msg, notice_msg, len+1);
    return pkt;
}

struct ggpkt *
ggpkt_mod_0_internal_ping(void)
{
    NEWPKT(pkt, body, GGPKT_MOD_0_INTERNAL_PING)
    body->data.ping.random = rand();
    return pkt;
}

struct ggpkt *
ggpkt_mod_0_internal_ping_res(int random_res)
{
    NEWPKT(pkt, body, GGPKT_MOD_0_INTERNAL_PING_RES)
    body->data.ping.random_res = random_res;
    return pkt;
}

/* return pointer to static decode packet from received buffer */
struct ggpkt_mod_0_internal *
ggpkt_mod_0_internal_decode(char *buf)
{
    struct ggpkt_mod_0_internal *bufbody;
    static struct ggpkt_mod_0_internal body;

    bufbody = (struct ggpkt_mod_0_internal *)body;
    body.type = bufbody->type;
    switch (body.type) {
    case GGPKT_MOD_0_INTERNAL_HELLO:
        body.data.hello.random_probe = ntohl(bufbody->data.hello.random_probe);
        break;
    case GGPKT_MOD_0_INTERNAL_HELLO_RES:
        body.data.hello.random_ggd = ntohl(bufbody->data.hello.random_ggd);
        break;
    case GGPKT_MOD_0_INTERNAL_AUTH:
        body.data.auth.probe_id = ntohs(bufbody->data.auth.probe_id);
        body.data.auth.probe_password_len = bufbody->data.auth.len;
        memcpy(body.data.auth.probe_password, bufbody->data.auth.probe_password,
            body.data.auth.probe_password_len);
        body.data.hello.random_probe = ntohl(bufbody->data.auth.random_probe);
        body.data.hello.random_ggd = ntohl(bufbody->data.auth.random_ggd);
        break;
    case GGPKT_MOD_0_INTERNAL_AUTH_RES:
        body.data.auth_res.random_ggd = ntohl(bufbody->data.auth_res.random_ggd);
        break;
    case GGPKT_MOD_0_INTERNAL_NOTICE:
        body.data.notice.len = ntohs(bufbody->data.notice.len);
        memcpy(body.data.notice.msg, bufbody->data.notice.msg,
            body.data.notice.len);
        break;
    case GGPKT_MOD_0_INTERNAL_PING:
        body.data.ping.random = ntohl(bufbody->data.ping.random);
        break;
    case GGPKT_MOD_0_INTERNAL_PING_RES:
        body.data.ping_res.random_res = ntohl(bufbody->data.ping_res.random_res);
        break;
    default:
        goto err;
        /* UNREACHED */
    }

    return &body;

err:
    return NULL;
}

int
_get_len(struct ggpkt *pkt)
{
    struct ggpkt_mod_0_internal *body;
    int size;

    body = (struct ggpkt_mod_0_internal *)pkt->body;
    switch(body->type) {
    case GGPKT_MOD_0_INTERNAL_HELLO:
        size = sizeof((struct ggpkt_mod_0_internal *)0)->data.hello;
        break;
    case GGPKT_MOD_0_INTERNAL_HELLO_RES:
        size = sizeof((struct ggpkt_mod_0_internal *)0)->data.hello_res;
        break;
    case GGPKT_MOD_0_INTERNAL_AUTH:
        size = sizeof((struct ggpkt_mod_0_internal *)0)->data.auth \
            + body->data.auth.probe_password_len;
        break;
    case GGPKT_MOD_0_INTERNAL_AUTH_RES:
        size = sizeof((struct ggpkt_mod_0_internal *)0)->data.auth_res;
        break;
    case GGPKT_MOD_0_INTERNAL_NOTICE:
        size = sizeof((struct ggpkt_mod_0_internal *)0)->data.notice \
            + body->data.notice.len;
        break;
    case GGPKT_MOD_0_INTERNAL_PING:
        size = sizeof((struct ggpkt_mod_0_internal *)0)->data.ping;
        break;
    case GGPKT_MOD_0_INTERNAL_PING_RES:
        size = sizeof((struct ggpkt_mod_0_internal *)0)->data.ping_res;
        break;
    default:
        size = -1;
        break;
    }

    return size;
}

