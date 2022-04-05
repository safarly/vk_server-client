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
# include <sys/epoll.h>
# include <sys/signalfd.h>
# include <signal.h>

# include "colors.h"
# include "errors.h"

# define BACKLOG_LIMIT	128
# define BUFF_SIZE		4096
# define MAX_EVENTS		128
# define DEFAULT_MODE	0644

# define print_error(err)	_print_error(err, __LINE__, __FILE__)

typedef			struct file_info
{
	int			fd;
	ushort		namelen;
	size_t		size;
}				file_info;

typedef					struct client_data
{
	int					socket;
	int					epfd;
	struct epoll_event	epev;

	struct file_info	file;
	bool				file_info_read;
	bool				name_read;
	size_t				bytes_read;
	size_t				name_bytes_read;
	char				name[NAME_MAX];
}						client_data;


int		set_socket_nonblock(int socket);

//		server.c
int		check_server_args(int argc, char **argv);
int		receive_file(struct client_data *client, char *save_dir);
int		handle_name(client_data *client, const char *save_dir, char *save_name);

//		client.c
int		check_client_args(int argc, char **argv, file_info *file);
int		send_file(int server, file_info *file, char *file_path);
char	*get_file_name(char *path);

//		sockets.c
int		create_client_socket(char *host);
int		create_server_socket(char *port);
char	*get_port(char *host);

//		utils.c
int		copy_data(int source, int dest);
int		_print_error(const char *err, int line, const char *file);
int		arg_is_numerical(const char *arg);

#endif
