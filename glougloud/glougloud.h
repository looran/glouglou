/*
 * Public header for glougloud and glougloud modules
 * (both probes and viz modules)
 */

#if defined(__OpenBSD__)
#include <sys/queue.h>
#else
#include <bsd/sys/queue.h>
#endif

#include <dnet.h>

struct glougloud {
	int daemonize;
	char *logfile;
	int loglevel;
	int pid;
	struct {
		char *socket;
		char *socket_chrooted;
	} redis;
	struct {
		struct addr serv_ip;
		int serv_port;
	} probes;
	struct {
		struct addr serv_ip;
		int serv_port;
	} viz;
};

enum ggdviz_cli_type {
	GGDVIZ_CLI_TCP = 0,
	GGDVIZ_CLI_WS = 1
};

struct ggdviz_cli {
	LIST_ENTRY(ggdviz_cli) entry;
	int id;
	enum ggdviz_cli_type type;
	union {
		struct {
			struct bufferevent *bev;
			struct addr addr;
		} tcp;
	};
};

struct ggdprobe_cli {
    LIST_ENTRY(ggprobe_cli) entry;
    int id;
};

struct ggdmodviz_conf {
    int id;
    int api_version;
};

