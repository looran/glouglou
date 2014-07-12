#include <sys/types.h>

#if !defined(__OpenBSD__)
#define __USE_GNU
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>

#if !defined(__OpenBSD__)
#include <sys/prctl.h>
#endif

#include "libglouglou.h"

/*
 * Various utils
 */

void *
xmalloc(size_t size)
{
	void *data;

	data = malloc(size);
	if (!data)
		err(1, "could not malloc %d", (int)size);
	return data;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *data;

	data = calloc(nmemb, size);
	if (!data)
		err(1, "could not calloc %d", (int)size);
	return data;
}

void
fd_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	int rc = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (rc == -1)
		err(1, "failed to set fd %i non-blocking", fd);
}

void
addrcpy(struct sockaddr_in *dst, struct sockaddr_in *src)
{
	dst->sin_addr.s_addr = src->sin_addr.s_addr;
	dst->sin_port = src->sin_port;
	dst->sin_family = src->sin_family;
}

int
addrcmp(struct sockaddr_in *a, struct sockaddr_in *b)
{
	if (a->sin_addr.s_addr != b->sin_addr.s_addr)
		return -1;
	if (a->sin_port != b->sin_port)
		return -2;
	if (a->sin_family != b->sin_family)
		return -3;
	return 0;
}

void
droppriv(char *user, int do_chroot, char *chroot_path)
{
	struct passwd	*pw;

	pw = getpwnam(user);
	if (!pw)
		err(1, "unknown user %s", user);
	if (do_chroot) {
		if (!chroot_path)
			chroot_path = pw->pw_dir;
		if (chroot(chroot_path) != 0)
			err(1, "unable to chroot");
	}
	if (chdir("/") != 0)
		err(1, "unable to chdir");
	if (setgroups(1, &pw->pw_gid) == -1)
		err(1, "setgroups() failed");
	if (setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) == -1)
		err(1, "setresgid failed");
	if (setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) == -1)
		err(1, "setresuid() failed");
	endpwent();
}

/* pipe cmd1 stdout in cmd2 stdin
 * return only on error (execve cmd2) */
int
exec_pipe(char *cmd1, char **cmd1_args, char *cmd2, char **cmd2_args)
{
	int pfd[2];

	pipe(pfd);
	if(fork() == 0)
	{
		dup2(pfd[1], 1);
		close(pfd[0]);
		close(pfd[1]);
		execvp(cmd1, cmd1_args);
		perror("execvp");
		exit(0);
	}
	dup2(pfd[0], 0);
	close(pfd[0]);
	close(pfd[1]);
	execvp(cmd2, cmd2_args);
	perror("execvp");
	return -1;
}

void
kill_wait(pid_t pid, int seconds) {
	int i;

	kill(pid, SIGTERM);
	for(i=0; i<10; i++) {
		if (waitpid(pid, NULL, WNOHANG) == -1)
			return;
		sleep(1);
	}
	log_warn("timeout SIGTERM %d, sending SIGKILL", pid);
	kill(pid, SIGKILL);
	sleep(1);
}

struct event *
udp_server_create(struct event_base *evb, struct addr *ip, int port, event_callback_fn cb_srv, void *cb_srv_data)
{
	int s = -1;
	struct sockaddr_in sock_addr;
	struct event *ev = NULL;
	int sock_on = 1;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		goto err;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
			&sock_on, sizeof(sock_on));
	fd_nonblock(s);

	bzero(&sock_addr, sizeof(sock_addr));
	addr_ntos(ip, (struct sockaddr *)&sock_addr);
	sock_addr.sin_port = htons(port);

	if (bind(s, (struct sockaddr *)&sock_addr,
				sizeof(sock_addr)) != 0)
		goto err;

	ev = event_new(evb, s, EV_READ|EV_PERSIST, cb_srv, cb_srv_data);
	event_add(ev, NULL);
	return ev;

err:
	if (s != -1)
		close(s);
	if (ev)
		event_del(ev);
	return NULL;
}

void
setprocname(const char *name)
{
#if defined(__OpenBSD__)
	setproctitle(name);
#else
	char basename[16];
	char newname[64];

	prctl(PR_GET_NAME, (unsigned long) basename, 0, 0, 0);
	snprintf(newname, sizeof(newname), "%s: %s", basename, name);
	prctl(PR_SET_NAME, newname, 0, 0, 0);
#endif
}

