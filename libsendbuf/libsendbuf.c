/*
 * Copyright (c) 2012, 2013 Laurent Ghigonis <laurent@gouloum.fr>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include "libsendbuf.h"

static void cb_timer(evutil_socket_t, short, void *);

/*
 * Public
 */

/*
 * Create a sendbuf
 */
struct sendbuf *
sendbuf_new(struct event_base *ev_base, int buffer_size, int msec_max,
	    int (*send_func)(void *, int, void *), void *usrdata)
{
	struct sendbuf *sbuf = NULL;
	struct event *ev_timer;

	sbuf = calloc(1, sizeof(struct sendbuf));
	if (!sbuf)
		return NULL;
	sbuf->ev_base = ev_base;
	sbuf->msec_max = msec_max;
	sbuf->buffer_size = buffer_size;
	sbuf->send_func = send_func;
	sbuf->usrdata = usrdata;
	sbuf->buffer = malloc(sbuf->buffer_size);
	if (!sbuf->buffer)
		goto err;

	ev_timer = evtimer_new(ev_base, cb_timer, sbuf);
	sbuf->ev_timer = ev_timer;
	sbuf->ev_timer_tv.tv_usec = msec_max * 1000;
	evtimer_add(ev_timer, &sbuf->ev_timer_tv);

	return sbuf;

err:
	sendbuf_free(sbuf);
	return NULL;
}

void
sendbuf_free(struct sendbuf *sbuf)
{
	if (sbuf->ev_timer)
		event_del(sbuf->ev_timer);
	if (sbuf->buffer && sbuf->send_func)
		sendbuf_flush(sbuf);
	if (sbuf->buffer)
		free(sbuf->buffer);
	free(sbuf);
}

/*
 * Append to the token buffer data to be sent
 * uses a memcpy, in contrary to sendbuf_gettoken().
 * return size on success, -1 on error
 */
int
sendbuf_append(struct sendbuf *sbuf, void *token, int size)
{
	if (sbuf->buffer_pos + size >= sbuf->buffer_size)
		if (sendbuf_flush(sbuf) == -1)
			return -1;

	memcpy(sbuf->buffer + sbuf->buffer_pos, token, size);
	sbuf->buffer_pos = sbuf->buffer_pos + size;

	return size;
}

/*
 * Returns a token buffer to write data to be sent
 * avoids memcpy, in contrary to sendbuf_append().
 * might return NULL if the sendbuf is temporary full
 */
void *
sendbuf_gettoken(struct sendbuf *sbuf, int size)
{
	void *token;

	if (sbuf->buffer_pos + size >= sbuf->buffer_size)
		if (sendbuf_flush(sbuf) == -1)
			return NULL;

	token = sbuf->buffer + sbuf->buffer_pos;
	sbuf->buffer_pos = sbuf->buffer_pos + size;

	return token;
}

/*
 * Send buffer immediatly
 * Note that you can still add data to the buffer even if flushing is in
 * progress
 * returns 0 on success or -1 on error
 */
int
sendbuf_flush(struct sendbuf *sbuf)
{
	int tosend, sent;

	if (sbuf->buffer_pos == 0)
		return 0;

	sbuf->flushing = 1;

	tosend = sbuf->buffer_pos - sbuf->flushing_pos;
	sent = sbuf->send_func(sbuf->buffer + sbuf->flushing_pos,
			tosend, sbuf->usrdata);
	if (sent == -1) {
		// XXX handle error
		return -1;
	} else if (sent < tosend) {
		sbuf->flushing_pos = sbuf->flushing_pos + sent;
		return -1;
	}
	sbuf->buffer_pos = 0;

	sbuf->flushing = 0;
	sbuf->flushing_pos = 0;
	return 0;
}


/*
 * Private
 */

static void
cb_timer(evutil_socket_t fd, short what, void *arg)
{
	struct sendbuf *sbuf;

	sbuf = arg;
	sendbuf_flush(sbuf);
	evtimer_add(sbuf->ev_timer, &sbuf->ev_timer_tv);
}
