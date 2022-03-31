#ifndef SERVER_H
# define SERVER_H

# include <stdbool.h>
# include <ctype.h>
# include <limits.h>
# include <errno.h>
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
# define BUFF_SIZE		4096
// # define EXIT			1
// # define NOEXIT			0

//		server.c
void	check_server_args(int argc, char **argv);
void	receive_file(int client, char *save_dir);
void	handle_name(int client, const char *save_dir, char *path_name);

//		client.c
void	check_client_args(int argc, char **argv, struct stat *file_stat);
void	send_file(int server, int file, char **argv);
char	*get_filename(char *path);

//		sockets.c
int		create_client_socket(char *host);
int		create_server_socket(char *port);
char	*get_port(char *host);

//		utils.c
void	copy_data(int source, int dest);
void	print_error(const char *err);
bool	arg_is_numerical(const char *arg);

#endif
