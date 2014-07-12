#ifndef _LIBGLOUGLOU_H_
#define _LIBGLOUGLOU_H_

#include <sys/types.h>
#include <dnet.h>
#include <event.h>

#if defined(__OpenBSD__)
#include <sys/queue.h>
#else
#include <bsd/sys/queue.h>
#endif

#define GLOUGLOU_PROBE_DEFAULT_PORT 4430
#define GLOUGLOU_VIZ_DEFAULT_PORT 4431

/* packet.c */

#define GGPKT_ARG_MAX 60
#define GGPKT_BUF_SIZE 16384

/* included in packets from libglouglou modules */
struct ggpkt_header {
	u_int8_t	module_id;
	u_int8_t	module_version;
	u_int16_t   len;
};

struct ggpkt {
    void *body; /* packet created in libglouglou modules */
    int   (*body_len)(struct ggpkt *); /* implemented in libglouglou modules */
};

struct ggpkt *ggpkt_new(int module_id, int module_version,
                void *body, int (*fn_body_len)(struct ggpkt *));
int           ggpkt_header_encode(struct ggpkt_header *header,
                int module_id, int module_version);
int           ggpkt_header_decode(struct ggpkt_header *header,
                int *module_id, int *module_version);

/* client.c */

enum gg_client_status {
	GG_CLIENT_STATUS_CONNECTED = 0,
	GG_CLIENT_STATUS_AUTHENTICATED = 1,
	GG_CLIENT_STATUS_OK = 2
};

struct gg_client {
	struct event_base *evb;
	struct addr *ip;
	int	port;
	struct sockaddr_in addr;
	struct event *ev;
	struct event *ev_timer;
	int	sock;
	enum gg_client_status status;
	int (*handle_conn)(struct gg_client *);
	int (*handle_packet)(struct gg_client *, struct ggpkt *);
	void *usrdata;
	struct sendbuf *sbuf;
};


struct gg_client *gg_client_connect(struct event_base *,
                    struct addr *ip, int port,
                    int *register_module_ids,
                    int (*handle_conn)(struct gg_client *prb),
                    int (*handle_pkt)(struct gg_client *prb,
                                        struct ggpkt *pkt));
void		      gg_client_disconnect(struct gg_client *);
int 		      gg_client_send(struct gg_client *,
                    int module_id, struct ggpkt *);

/* server.c */

enum gg_user_status {
	GG_USER_STATUS_CONNECTED = 0,
	GG_USER_STATUS_AUTHENTICATED = 1,
	GG_USER_STATUS_OK = 2
};

struct gg_user {
	LIST_ENTRY(gg_user) entry;
	int id;
	int sock;
	enum gg_user_status status;
	struct sockaddr_in addr;
	struct sendbuf *sbuf;
};

struct gg_server {
	struct event_base *evb;
	struct addr *ip;
	int	 port;
	struct sockaddr_in addr;
	struct event *ev;
	int	 sock;
	int *module_ids;
	int (*handle_conn)(struct gg_server *, struct gg_user *);
	int (*handle_packet)(struct gg_server *,
			struct gg_user *, struct ggpkt *);
	void	*usrdata;
	LIST_HEAD(, gg_user) user_list;
	int	 user_count;
	int	 user_id_count;
};

struct gg_server *gg_server_start(struct event_base *,
			struct addr *ip, int port,
			int *register_module_ids,
			int (*handle_conn)(struct gg_server *srv, struct gg_user *usr),
			int (*handle_pkt)(struct gg_server *srv, struct gg_user *usr, struct ggpkt *pkt), void *);
void		  gg_server_stop(struct gg_server *srv);

/* log.c */

#define LOG_FORCED -2
#define LOG_FATAL -1
#define LOG_WARN 0
#define LOG_INFO 1
#define LOG_DEBUG 2

int	 log_init(char *, int);
void	 log_shutdown(void);
void	 log_tmp(const char *, ...);
void	 log_debug(const char *, ...);
void	 log_info(const char *, ...);
void	 log_warn(const char *, ...);
#if defined(__OpenBSD__)
void __dead log_fatal(const char *, ...);
#else
void	 log_fatal(const char *, ...);
#endif

/* utils.c */

void	*xmalloc(size_t);
void	*xcalloc(size_t, size_t);
void	 fd_nonblock(int);
void	 addrcpy(struct sockaddr_in *, struct sockaddr_in *);
int	 addrcmp(struct sockaddr_in *, struct sockaddr_in *);
void	 droppriv(char *, int, char *);
int	exec_pipe(char *, char **, char *, char **);
void	kill_wait(pid_t, int);
struct event *udp_server_create(struct event_base *, struct addr *, int, event_callback_fn, void *);
void	setprocname(const char *);

#endif /* _LIBGLOUGLOU_H_ */
