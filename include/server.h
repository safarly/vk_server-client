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

    //    #include <sys/signalfd.h>
    //    #include <signal.h>
    //    #include <unistd.h>
    //    #include <stdlib.h>
    //    #include <stdio.h>

# include "colors.h"
# include "errors.h"

# define BACKLOG_LIMIT	128
# define BUFF_SIZE		4096
# define MAX_EVENTS		128

struct file_info
{
	// char				name[NAME_MAX];
	size_t				namelen;
	// size_t				size;
	struct stat			file_stat;
};

struct client_data
{
	int					socket;
	struct epoll_event	epev;
	// struct stat			file_stat;

	struct file_info	file;
	size_t	bytes_read;
	ssize_t	count;

	int		filefd;
};


int		set_socket_nonblock(int socket);

//		server.c
int		check_server_args(int argc, char **argv);
int		receive_file(struct client_data *client, char *save_dir);
int		handle_name(int client, const char *save_dir, char *path_name);

//		client.c
int		check_client_args(int argc, char **argv, struct stat *file_stat);
int		send_file(int server, int file, char **argv);
char	*get_filename(char *path);

//		sockets.c
int		create_client_socket(char *host);
int		create_server_socket(char *port);
char	*get_port(char *host);

//		utils.c
int		copy_data(int source, int dest);
int		print_error(const char *err);
int		arg_is_numerical(const char *arg);

#endif
