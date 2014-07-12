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

#include "libglouglou.h"

FILE *_logfile;
static int _loglevel;

static void logit(int, const char *, const char *, va_list);

/*
 * Log
 * handles only one log file per process
 */

int
log_init(char *filename, int level)
{
	if (filename) {
		_logfile = fopen(filename, "a+");
		if (!_logfile) {
			printf("cannot open log file %s!\n", filename);
			return -1;
		}
	} else {
		_logfile = stderr;
	}
	_loglevel = level;
	return 0;
}

void
log_shutdown(void)
{
	fclose(_logfile);
}

void
log_tmp(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	logit(LOG_FORCED, "XXX ", msg, ap);
	va_end(ap);
}

void
log_debug(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	logit(LOG_DEBUG, "", msg, ap);
	va_end(ap);
}

void
log_info(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	logit(LOG_INFO, "", msg, ap);
	va_end(ap);
}

void
log_warn(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	logit(LOG_WARN, "WARN: ", msg, ap);
	va_end(ap);
}

#if defined(__OpenBSD__)
void __dead
#else
void
#endif
log_fatal(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	logit(LOG_FATAL, "FATAL: ", msg, ap);
	va_end(ap);

	exit(1);
}

/* XXX mpsafe */
static void
logit(int level, const char *prefix, const char *msg, va_list ap)
{
	time_t clock;

	if (level <= _loglevel) {
		time(&clock);
		fprintf(_logfile, "%d ", (int)clock);
		vfprintf(_logfile, prefix, ap);
		vfprintf(_logfile, msg, ap);
		fprintf(_logfile, "\n");
		fflush(_logfile);
	}
}

