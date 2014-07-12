#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>
#include <string.h>

#include <event.h>
#include <libglouglou.h>

#include <libglouglou/ext/ggpkt_mod_0_internal.h>

int _test_connected;
int _test_receive_response;

#if defined(__OPENBSD__)
void __dead
#else
void
#endif
usage(void)
{
	// XXX extern char *__progname;

    fprintf(stderr, "usage: [-hv] glougloud_ip glougloud_port [notice_msg]\n");
	exit(1);
}

int
handle_conn(struct gg_client *prb)
{
    _test_connected = 1;
    return 0;
}

int
handle_pkt(struct gg_client *prb, struct ggpkt *pkt)
{
     struct ggpkt_mod_0_internal *body;

     body = ggpkt_mod_0_internal_decode(pkt);
     if (body->type == GGPKT_MOD_0_INTERNAL_PING_RES)
        _test_receive_response = 1;
     else
        printf("Warning: received msg != GGPKT_MOD_0_INTERNAL_PING_RES !\n");

    _test_receive_response = 1;
    return 0;
}

int
main(int argc, char **argv)
{
    int *register_module_ids = [ GGPKT_MOD_0_INTERNAL_ID, -1 ];
    struct event_base *evb;
    struct gg_client *cli;
    struct ggpkt *pkt;
    char *notice_msg;
    int notice_len;
    int loglevel;
    struct addr ip;
    int port;
    int res;
    int op;

    _test_connected = 0;
    _test_receive_response = 0;

    notice_msg = NULL;
    notice_len = 0;
    loglevel = 0;
	while ((op = getopt(argc, argv, "hN:t:v")) != -1) {
		switch (op) {
			case 'h':
				usage();
				/* NOTREACHED */
			case 'v':
				loglevel++;
				break;
			default:
				usage();
				/* NOTREACHED */
		}
	}
    argc -= optind;
    argv += optind;
    if (argc < 2 || argc > 3)
        usage();

    addr_aton(argv[0], &ip);
    port = atoi(argv[1]);
    if (argc == 3) {
        notice_msg = strdup(argv[2]);
        notice_len = strlen(notice_msg);
    }

    evb = event_base_new();

    cli = gg_client_connect(evb, &ip, port, register_modules,
        handle_conn, handle_pkt);
    if (!cli) {
        printf("Error: could not connect ! (gg_client_connect failed)\n");
        return 2;
    }

    pkt = ggpkt_mod_0_internal_ping()
    if (!pkt) {
        printf("Error: could not create PING packet !\n");
        return 10;
    }
    res = gg_client_send(cli, pkt);
    if (res < 0) {
        printf("Error: could not send PING !\n");
        return 11;
    }

    if (notice_msg) {
        pkt = ggpkt_mod_0_internal_notice(notice_len, notice_msg);
        if (!pkt) {
            printf("Error: could not create NOTICE packet !\n");
            return 10;
        }
        res = gg_client_send(cli, pkt);
        if (res < 0) {
            printf("Error: could not send NOTICE !\n");
            return 11;
        }
    }

    event_base_dispatch(evb);

    gg_client_disconnect(cli);

    if (_test_connected == 0) {
        printf("Error: could not connect ! (test_connected == 0)\n");
        return 1;
    }
    if (_test_receive_response == 0) {
        printf("Error: did not receive response !");
        return 15;
    }

    return 0;
}
