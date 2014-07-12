#include "../libglouglou.h"

int
main(int argc, char **argv)
{
	char *echo_args[] = {"echo", "toto", NULL};
	char *grep_args[] = {"grep", "-q", "toto", NULL};

	exec_pipe("echo", echo_args, "grep", grep_args);

	/* returns only on error */
	return 1;
}
