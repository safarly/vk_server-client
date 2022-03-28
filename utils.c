#include "server.h"

bool	arg_is_numerical(const char *arg)
{
	while (*arg) {
		if (!isdigit(*arg)) {
			return false;
		}
		arg++;
	}
	return true;
}

void	print_error(const char *err)
{
	dprintf(STDERR_FILENO, "%sError:%s %s", RED, RESET, err);
	exit(EXIT_FAILURE);
}
