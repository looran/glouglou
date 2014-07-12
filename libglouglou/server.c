#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <libsendbuf.h>
#include "libglouglou.h"

/* we include libglouglou_mod_0_internal header and link against it, wich
 * is not the case for other libglouglou module which are normaly independent */
#include "libglouglou_mod_0_internal/libglouglou_mod_0_internal.h"

void
cb_srv_receive(evutil_socket_t fd, short what, void *arg)
{
    struct gg_server *srv;
    struct ggpkt *pkt;
	struct sockaddr_in remote;
	socklen_t remote_len;
	char buf[PACKET_BUFFER_SIZE];

    // XXX IN PROGRESS
    srv = (struct gg_server *)arg;
	remote_len = sizeof(struct sockaddr_in);
	len = recvfrom(fd, buf, sizeof(buf), 0,
			(struct sockaddr *)&remote, &remote_len);
	if (len < 0) {
		error("recvfrom failed");
		return;
	}

	pkt = ggpkt_receive_header(fd, &len, &remote, &remote_len);

	usr = user_find(srv, &remote);
	if (!usr) {
        
		usr = user_add(srv, &remote);
		// XXX delay until AUTH if (srv->handle_conn)
			// srv->handle_conn(srv, usr);
	    
		user_send(usr, "", 0);
	} else {
		debug("Incoming data from existing user !");
		if (len == 0) {
			user_del(srv, usr);
			return;
		}
		if (srv->handle_packet) {
			buf_p = buf;
			buf_len = len;
			while (buf_len > 0 && (pkt = pkt_decode(&buf_p, &buf_len)))
				srv->handle_packet(srv, usr, pkt);
			if (buf_len > 0) {
				/* XXX store incomplete packet for next recv */
				error("incomplete packet, dropped %d bytes !", buf_len);
			}
		}
	}
    
    pkt = ggpkt_mod_0_internal_hello_res();
    gg_server_send(srv, pkt);
}

int
user_send(struct gg_user *usr, void *data, int size)
{
	int sent;

	sent = sendto(usr->sock, data, size, 0, (struct sockaddr *)&usr->addr,
			sizeof(struct sockaddr_in));
	if (sent == -1)
		log_warn("failed: %s", strerror(errno));
	return sent;
}

struct gg_user *
user_add(struct gg_server *srv, struct sockaddr_in *remote)
{
	struct gg_user *usr;
	struct sendbuf *sbuf;

	usr = xcalloc(1, sizeof(struct gg_user));
	usr->id = srv->user_id_count;
	srv->user_id_count++;
	srv->user_count++;
	usr->sock = srv->sock;
	addrcpy(&usr->addr, remote);

	sbuf = sendbuf_new(srv->ev_base, PACKET_SNDBUF_MAX, 200, cb_usr_send, usr);
	if (!sbuf)
		goto err;
	usr->sbuf = sbuf;

	LIST_INSERT_HEAD(&srv->user_list, usr, entry);
	verbose("Add user %d !", usr->id);

	return usr;

err:
	user_del(srv, usr);
	return NULL;
}

void
user_del(struct gg_server *srv, struct gg_user *usr)
{
	log_debug("Del user %d !", usr->id);
	if (usr->sbuf)
		sendbuf_free(usr->sbuf);
	user_send(usr, "", 0);
	LIST_REMOVE(usr, entry);
	srv->user_count--;
	free(usr);
}


struct gg_server *gg_server_start(struct event_base *evb,
			struct addr *ip, int port,
			int *register_module_ids,
			int (*handle_conn)(struct gg_server *srv, struct gg_user *usr),
			int (*handle_packet)(struct gg_server *srv, struct gg_user *usr, struct ggpkt *pkt), void *usrdata)
{
	struct gg_server *srv;

	srv = xcalloc(1, sizeof(struct gg_server));
	srv->evb = evb;
	srv->ip = ip;
	srv->port = port;
	srv->module_ids = register_module_ids;
	srv->handle_conn = handle_conn;
	srv->handle_packet = handle_packet;
	srv->usrdata = usrdata;
	srv->ev = udp_server_create(evb, ip, port, cb_srv_receive, srv);
	if (!srv->ev)
		goto err;

	return srv;

err:
	log_warn("gg_server_start: %s", strerror(errno));
	gg_server_stop(srv);
	return NULL;
}


void
gg_server_stop(struct gg_server *srv)
{
	struct gg_user *usr;

	while ((usr = LIST_FIRST(&srv->user_list))) {
		user_del(srv, usr);
	}
	if (srv->ev) {
		close(event_get_fd(srv->ev));
		event_del(srv->ev);
	}
	free(srv);
}

