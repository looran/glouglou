#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>
#include <string.h>

#include <event.h>
#include <libglouglou.h>

#include "glougloud_internal.h"

struct event_base *ev_base;

#if defined(__OPENBSD__)
void __dead
#else
void
#endif
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-Dhv] [-l probes_server_ip] [-L viz_server_ip]\n"
			"\t\t[-p probes_server_port] [-P viz_server_port]\n", __progname);
	exit(1);
}

static void
sig_handler(int sig, short why, void *data)
{
	log_info("got signal %d", sig);
	if (sig == SIGINT || sig == SIGTERM) {
		viz_shutdown();
		probes_shutdown();
		redis_shutdown();
		event_base_loopexit(ev_base, NULL);
	}
}

int
main(int argc, char **argv)
{
	struct event *ev_sigint, *ev_sigterm, *ev_sigchld, *ev_sighup;
	struct glougloud *ggd;
	int op;

	ggd = xcalloc(1, sizeof(struct glougloud));

	addr_aton("127.0.0.1", &ggd->probes.serv_ip);
	addr_aton("127.0.0.1", &ggd->viz.serv_ip);
	ggd->probes.serv_port = GLOUGLOU_PROBE_DEFAULT_PORT;
	ggd->viz.serv_port = GLOUGLOU_VIZ_DEFAULT_PORT;
	ggd->daemonize = 1;
	ggd->logfile = GLOUGLOUD_LOGFILE;
	ggd->loglevel = LOG_WARN;
	ggd->redis.socket_chrooted = "/socket/redis.sock";
	ggd->redis.socket = "/var/lib/glougloud/chroot/socket/redis.sock";

	while ((op = getopt(argc, argv, "Dhl:L:p:P:v")) != -1) {
		switch (op) {
			case 'D':
				ggd->logfile = NULL;
				ggd->daemonize = 0;
				break;
			case 'h':
				usage();
				/* NOTREACHED */
			case 'l':
				if (addr_aton(optarg, &ggd->probes.serv_ip) < 0)
					err(1, "invalid probes server ip");
				break;
			case 'L':
				if (addr_aton(optarg, &ggd->viz.serv_ip) < 0)
					err(1, "invalid vizualisation server ip");
				break;
			case 'p':
				ggd->probes.serv_port = atoi(optarg);
				break;
			case 'P':
				ggd->viz.serv_port = atoi(optarg);
				break;
			case 'v':
				ggd->loglevel++;
				break;
			default:
				usage();
				/* NOTREACHED */
		}
	}

	if (geteuid() != 0)
		errx(1, "must be root");
	log_init(ggd->logfile, ggd->loglevel);
	log_warn("glougloud startup");

	if (redis_init(ggd) < 0)
		log_fatal("init redis failed");
	if (probes_init(ggd) < 0)
		log_fatal("init probes failed");
	if (viz_init(ggd) < 0)
		log_fatal("init viz failed");

	ev_base = event_base_new();
	ev_sigint = evsignal_new(ev_base, SIGINT, sig_handler, NULL);
	ev_sigterm = evsignal_new(ev_base, SIGTERM, sig_handler, NULL);
	ev_sigchld = evsignal_new(ev_base, SIGCHLD, sig_handler, NULL);
	ev_sighup = evsignal_new(ev_base, SIGHUP, sig_handler, NULL);
	evsignal_add(ev_sigint, NULL);
	evsignal_add(ev_sigterm, NULL);
	evsignal_add(ev_sigchld, NULL);
	evsignal_add(ev_sighup, NULL);
	signal(SIGPIPE, SIG_IGN);

	if (ggd->daemonize) {
		ggd->pid = fork();
		log_info("daemonized, pid %d", ggd->pid);
		if (ggd->pid > 0)
			return 0;
	}

	event_base_dispatch(ev_base);

	viz_shutdown();
	probes_shutdown();
	redis_shutdown();

	return 0;
}
