libsendbuf - buffer helper for sending fixed size data

You give sendbuf_new a send_func callback, a buffer size and a
maximum milisecond to wait before sending, and libsendbuf will
call the send_func appropriatly.

2 ways to add data to the buffer:
* sendbuf_append, where you give yourself the token to add to the
buffer
* sendbuf_gettoken, that avoids memcpy by returning you a pointer
where you will write directly to the buffer

ALTERNATIVES
You can use libevent bufferevent / evbuffer instead.
www.wangafu.net/~nickm/libevent-book/Ref6_bufferevent.html 
http://www.wangafu.net/~nickm/libevent-book/Ref7_evbuffer.html
