#include "server.h"

void	set_socket_nonblock(int socket)
{
	fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
}

int		main(int argc, char **argv) /* argv[1] - port, argv[2] - save dir */
{
	int		server;
	int		client;
	int		epfd;

	if (check_server_args(argc, argv) < 0) {
		return EXIT_FAILURE;
	}

	epfd = epoll_create1(0);
	if (epfd < 0) {
		print_error(strerror(errno));
		return EXIT_FAILURE;
	}

	server = create_server_socket(argv[1]);
	if (server < 0) {
		return EXIT_FAILURE;
	}

	set_socket_nonblock(server);

	struct epoll_event server_event;
	struct epoll_event events[MAX_EVENTS];

	server_event.data.fd = server;
	server_event.events = EPOLLIN;

	epoll_ctl(epfd, EPOLL_CTL_ADD, server, &server_event); //check what is returned

	int event_count = 0;
	printf("Server has started and is listening on port %s\n", argv[1]);
	while (true)
	{
		event_count = epoll_wait(epfd, events, MAX_EVENTS, -1);

		for (int i = 0; i < event_count; i++) {
			if (events[i].data.fd == server) {
				client = accept(server, NULL, NULL);
				if (client < 0) {
					print_error(strerror(errno));
					continue ;
				}
				struct client_data *client_data = malloc(sizeof(struct client_data));
				client_data->socket = client;
				client_data->epev.data.ptr = client_data;
				client_data->epev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
				set_socket_nonblock(client);
				epoll_ctl(epfd, EPOLL_CTL_ADD, client, &client_data->epev);
			}
			else {
				struct client_data *client_data = events[i].data.ptr;
				if (events[i].events & (EPOLLHUP | EPOLLRDHUP)) { // close client socket and free struct cl_ev
					printf("Client closed\n");
					// struct client_data *client_to_close = events[i].data.ptr;
					close(client_data->socket);
					free(client_data);
					continue;
				}

				else if (events[i].events & EPOLLIN) {	// read from client sock
					printf("Read data from client\n");
					if (receive_file(client_data, argv[2]) < 0) {
						print_error(strerror(errno));
						// close(client);
						// printf("\nStill listening on port %s\n", argv[1]);
						continue ;
					}
					continue;
				}

				printf("Client connected\n");
				// if (receive_file(client, argv[2]) < 0) {
				// 	print_error(strerror(errno));
				// 	close(client);
				// 	printf("\nStill listening on port %s\n", argv[1]);
				// 	continue ;
				// }

				// close(client);
			}
		// epoll_ctl(epfd, )
	}

		// if (client < 0) {
		// 	print_error(strerror(errno));
		// 	continue ;
		// }
// dont forget to free
		// if (client > 0) {
			// printf("Client connected\n");
			// if (receive_file(client, argv[2]) < 0) {
			// 	print_error(strerror(errno));
			// 	// status = 1;
			// 	// write(client, &status, 1);
			// 	close(client);
			// 	printf("\nStill listening on port %s\n", argv[1]);
			// 	continue ;
			// }

			// close(client);
			// printf("\nStill listening on port %s\n", argv[1]);
		// }
	}

	close(server);
	return 0;
}

int		check_server_args(int argc, char **argv)
{
	struct stat	file_stat;

	memset(&file_stat, 0, sizeof(file_stat));
	if (argc != 3) {
		fprintf(stderr, ERR_USAGE_SRV);
		return -1;
	}

	if (!arg_is_numerical(argv[1])) {
		print_error(ERR_ISNUM);
		return -1;
	}

	if ((unsigned int)atoi(argv[1]) > SHRT_MAX) {
		print_error(ERR_SOCK_LIM);
		return -1;
	}

	stat(argv[2], &file_stat);
	if (!S_ISDIR(file_stat.st_mode)) {
		print_error(ERR_DIR);
		return -1;
	}

	return 1;
}

int		receive_file(struct client_data *client, char *save_dir)
{
	int		filefd;
	// char	path_name[PATH_MAX];
	// struct stat	file_stat;

	memset(client->path_name, 0, sizeof(client->path_name));

	// ssize_t	bytes_written = 0;
	ssize_t	bytes_read = 0;
	ssize_t	count = 0;

	while (true)
	{
		count = read(client->socket, &client->file_stat + bytes_read, sizeof(struct stat) - bytes_read);
		if (count == 0) {
			break ;
		}
		bytes_read += count;
	}
	printf("%ld file size\n", client->file_stat.st_size);

	// if (read(client->socket, &client->file_stat, sizeof(struct stat)) < 0) {
	// 	return -1;
	// }

	if (handle_name(client, save_dir) < 0) {
		return -1;
	}

	filefd = open(client->path_name, O_CREAT | O_EXCL | O_WRONLY, client->file_stat.st_mode);
	if (filefd < 0) {
		return -1;
	}

	if (copy_data(client->socket, filefd) < 0) {
		close(filefd);
		return -1;
	}

	printf("File was %ssuccessfully%s saved\n", GREEN, RESET);
	close(filefd);
	return 1;
}

int		handle_name(struct client_data *client, const char *save_dir)
{
	char	buf_name[NAME_MAX];
	size_t	namelen = 0;

	memset(buf_name, 0, sizeof(buf_name));
	if (read(client->socket, &namelen, sizeof(namelen)) < 0) {
		return -1;
	}

	if (read(client->socket, buf_name, namelen) < 0) {
		return -1;
	}

	if (strchr(buf_name, '/')) {
		return print_error(ERR_FILENAME);
	}

	strcpy(client->path_name, save_dir);
	if (client->path_name[strlen(client->path_name) - 1] != '/') {
		strcat(client->path_name, "/");
	}

	strcat(client->path_name, buf_name);
	printf("file - \'%s\'\n", client->path_name);
	return 1;
}
