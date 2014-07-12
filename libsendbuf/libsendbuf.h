#include <event.h>
#include <sys/queue.h>

struct sendbuf {
	struct event_base *ev_base;
	struct event      *ev_timer;
	struct timeval     ev_timer_tv;
	int   msec_max;
	int   buffer_size;
	void *buffer;
	int   buffer_pos; /* next to use in buffer */
	int   flushing;
	int   flushing_pos; /* next to send in buffer */
	int (*send_func)(void *, int, void *);
	void *usrdata;
};

struct sendbuf	*sendbuf_new(struct event_base *, int, int,
			int (*send_func)(void *, int, void *),
			void *);
void		 sendbuf_free(struct sendbuf *);
int		 sendbuf_append(struct sendbuf *, void *, int);
void		*sendbuf_gettoken(struct sendbuf *, int);
int		 sendbuf_flush(struct sendbuf *);
