#ifndef SERVER_H
# define SERVER_H

# define _XOPEN_SOURCE	600

# include <stdbool.h>
# include <ctype.h>
# include <limits.h>
# include <errno.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>
# include <netdb.h>
# include <signal.h>
# include <sys/signalfd.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/epoll.h>

# include "colors.h"
# include "errors.h"

# define BACKLOG_LIMIT	128
# define BUFF_SIZE		4096
# define MAX_EVENTS		128
# define DEFAULT_MODE	0644
# define VERBOSE		0

# define print_error(err)	_print_error(err, __LINE__, __FILE__)

enum hints_flag		{ HF_SERVER, HF_CLIENT };
enum verbose_flag	{ VF_EPOLLIN, VF_INFO, VF_NAME, VF_EAGAIN,
					  VF_HNDL_NAME, VF_FILEFD, VF_SOCK };

typedef			struct file_info
{
	int			fd;
	unsigned	namelen;
	size_t		size;
}				file_info;

typedef			struct file_flags
{
	bool		info_read;
	bool		name_read;
	bool		opened;
	bool		saved;
}				file_flags;

typedef					struct client_data
{
	int					socket;
	int					epfd;
	struct epoll_event	epev;

	struct file_info	file;
	struct file_flags	fflags;
	size_t				bytes_read;
	size_t				name_bytes_read;
	char				name[NAME_MAX + 1];
}						client_data;

//		server.c
int		check_server_args(int argc, char **argv);
void	run_server(int epfd, int sigfd, int server, const char *save_dir);
int		process_client_event(client_data *client, const char *save_dir);

//		epoll.c
int		set_epoll_and_events(int server, int *sigfd);
int		set_server_event(int epfd, int server);
int		set_signal_event(int epfd, int *sigfd);
int		accept_new_client(int epfd, int server);
client_data	*client_init(int socket, int epfd);
int		client_destroy(client_data *client);

//		receive_file.c
int		receive_data_from_client(client_data *client, const char *save_dir);
int		get_file_info(client_data *client);
int		handle_name(client_data *client, const char *save_dir, char *save_name);

//		client.c
int		check_client_args(int argc, char **argv, file_info *file);
int		send_file(int server, file_info *file, char *file_path);
char	*get_file_name(char *path);

//		sockets.c
int		set_socket_nonblock(int socket);
int		create_client_socket(char *host);
int		create_server_socket(char *port);
char	*get_port(char *host);
void	hints_init(struct addrinfo *hints, int flag);

//		utils.c
int		copy_data(int source, int dest);
int		_print_error(const char *err, int line, const char *file);
void	print_verbose(int flag, client_data *client);
int		isvalid_char(char c);
int		check_file_name(client_data *client);
void	get_path(const char *save_dir, char *name, char *save_name);
int		arg_is_numerical(const char *arg);

#endif
