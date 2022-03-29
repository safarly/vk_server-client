#ifndef SERVER_H
# define SERVER_H

# include <stdbool.h>
# include <ctype.h>
# include <limits.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/stat.h>

# include "colors.h"

# define ERR_ARGC		"Wrong number of arguments\n"
# define ERR_ISNUM		"Socket argument must be numerical\n"
# define ERR_SOCK_LIM	"Socket limit has been exceeded (limit is 65535)\n"
# define ERR_DIR		"Invalid directory\n"
# define ERR_FILE		"Invalid file (is not a regular file)\n"

void	check_arguments(int argc, char **argv);
void	print_error(const char *err);
bool	arg_is_numerical(const char *arg);

#endif
