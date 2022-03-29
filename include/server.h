#ifndef SERVER_H
# define SERVER_H

# define _XOPEN_SOURCE	600

# include <stdbool.h>
# include <ctype.h>
# include <limits.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <netdb.h>

# include "colors.h"
# include "errors.h"

# define BACKLOG_LIMIT	128

void	check_arguments(int argc, char **argv);
void	print_error(const char *err);
bool	arg_is_numerical(const char *arg);
int		create_client_socket(char *host);
int		create_server_socket(char *port);
char	*get_port(char *host);

#endif
