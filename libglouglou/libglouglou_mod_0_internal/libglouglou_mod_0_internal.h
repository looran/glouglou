#include "../libglouglou.h"

#define GLOUGLOU_PASS_SIZE_MAX 16

#define GGPKT_MOD_0_INTERNAL_ID 0
#define GGPKT_MOD_0_INTERNAL_VERSION 1

enum ggpkt_mod_0_internal_type {
    GGPKT_MOD_0_INTERNAL_HELLO = 0x00,
    GGPKT_MOD_0_INTERNAL_HELLO_RES = 0x01,
    GGPKT_MOD_0_INTERNAL_AUTH = 0x02,
    GGPKT_MOD_0_INTERNAL_AUTH_RES = 0x03,
    GGPKT_MOD_0_INTERNAL_PING = 0x10,
    GGPKT_MOD_0_INTERNAL_PING_RES = 0x11,
    GGPKT_MOD_0_INTERNAL_NOTICE = 0x20,
};

enum ggpkt_mod_0_internal_auth_status {
    AUTH_STATUS_OK = 0x00,
    AUTH_STATUS_REFUSED = 0x01,
};

struct ggpkt_mod_0_internal {
    struct __attribute__((packed)) ggpkt_header header; /* libglouglou mandatory */
    u_int8_t type;
    union {
        struct __attribute__((packed)) hello {
            u_int32_t random_probe;
        } hello;
        struct __attribute__((packed)) hello_res {
            u_int32_t random_ggd;
        } hello_res;
        struct __attribute__((packed)) auth {
            u_int16_t probe_id;
            u_int8_t  probe_password_len;
            char      probe_password[GLOUGLOU_PASS_SIZE_MAX];
            u_int32_t random_probe;
            u_int32_t random_ggd;
        } auth;
        struct __attribute__((packed)) auth_res {
            u_int8_t status;
        } auth_res;
        struct __attribute__((packed)) notice {
            u_int16_t len;
            char msg[GGPKT_ARG_MAX];
        } notice;
        struct __attribute__((packed)) ping {
            u_int32_t random;
        } ping;
        struct __attribute__((packed)) ping_res {
            u_int32_t random_res;
        } ping_res;
    } data;
};

struct ggpkt *ggpkt_mod_0_internal_hello(void);
struct ggpkt *ggpkt_mod_0_internal_hello_res(void);
struct ggpkt *ggpkt_mod_0_internal_hello_auth(int probe_id, char *pass,
    int random_probe, int random_ggd);
struct ggpkt *ggpkt_mod_0_internal_auth_res(int status);
struct ggpkt *ggpkt_mod_0_internal_notice(char *notice_msg);
struct ggpkt *ggpkt_mod_0_internal_ping(void);
struct ggpkt *ggpkt_mod_0_internal_ping_res(int random_res);
struct ggpkt_mod_0_internal *ggpkt_mod_0_internal_decode(char *buf);
