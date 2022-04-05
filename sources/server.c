#include "server.h"

int		set_server_event(int epfd, int server)
{
	struct epoll_event	server_event;

	server_event.data.fd = server;
	server_event.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server, &server_event) < 0) {
		return -1;
	}

	return 1;
}

int		set_signal_event(int epfd, int *sigfd)
{
	sigset_t	mask;
	struct epoll_event	signal_event;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
		return -1;
	}

	*sigfd = signalfd(-1, &mask, SFD_NONBLOCK);
	if (*sigfd == -1) {
		return -1;
	}

	signal_event.data.fd = *sigfd;
	signal_event.events = EPOLLIN; // check edge trigger or level

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, *sigfd, &signal_event) < 0) {
		return -1;
	}

	return 1;
}

int		set_epoll_and_events(int server, int *sigfd)
{
	int		epfd;

	epfd = epoll_create1(0);
	if (epfd < 0) {
		return -1;
	}

	if (set_server_event(epfd, server) < 0) {
		return -1;
	}

	if (set_signal_event(epfd, sigfd) < 0) {
		return -1;
	}

	return epfd;
}

client_data	*client_init(int socket, int epfd)
{
	client_data	*client = malloc(sizeof(client_data));
	if (client == NULL) {
		return NULL;
	}

	client->socket = socket;
	client->epfd = epfd;
	client->epev.data.ptr = client;
	client->epev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
	client->bytes_read = 0;
	client->name_bytes_read = 0;

	return client;
}

int		client_destroy(client_data *client)
{
	epoll_ctl(client->epfd, EPOLL_CTL_DEL, client->socket, &client->epev);
	close(client->socket);
	close(client->file.fd);
	free(client);
	client = NULL;

	return 1;
}

int		accept_new_client(int epfd, int server)
{
	int		client_sock;
	client_data	*client;

	client_sock = accept(server, NULL, NULL); //accept needs to be in while loop or check if EPOLLET or EPOLL LEVEL TRIGGEREd needed
	if (client_sock < 0) {
		return -1;
	}
	// log(0, "blaak %f", 34); update printe_error with variadic args

	else {
		client = client_init(client_sock, epfd);
		if (client == NULL || set_socket_nonblock(client->socket) < 0) {
			return -1;
		}

		epoll_ctl(epfd, EPOLL_CTL_ADD, client->socket, &client->epev);
		printf("Client connected\n");
	}

	return 1;
}

int		main(int argc, char **argv) /* argv[1] - port, argv[2] - save dir */
{
	int		server;
	int		epfd;
	int		sigfd;
	int		count;
	int		event_count;
	client_data 		*client;
	struct epoll_event	events[MAX_EVENTS];

	char	save_name[PATH_MAX];
	memset(save_name, 0, sizeof(save_name));

	if (check_server_args(argc, argv) < 0) {
		return EXIT_FAILURE;
	}

	server = create_server_socket(argv[1]);
	if (server < 0) {
		return EXIT_FAILURE;
	}

	epfd = set_epoll_and_events(server, &sigfd);
	if (epfd < 0) {
		print_error(strerror(errno));
		return EXIT_FAILURE;
	}

	printf("Server has started and is listening on port %s\n", argv[1]);
	int terminate = 0;
	while (!terminate)
	{
		event_count = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (event_count < 0) {
			print_error(strerror(errno));
			continue ;
		}

		for (int i = 0; i < event_count; i++) {
			if (events[i].data.fd == server) {
				if (accept_new_client(epfd, server) < 0) {
					print_error(strerror(errno));
					continue ;
				}
			}

			else if (events[i].data.fd == sigfd) {
				printf("\nInterrupted!\n");
				// free memory
				// close fds
				terminate = 1;
				break ;
			}

			else {
				client = events[i].data.ptr;

				if (events[i].events & EPOLLIN) {
					printf("Read data from client\n");
				// int count could be mad on stack
					while (client->bytes_read < sizeof(file_info))
					{
						count = read(client->socket,
							((char *)&client->file) + client->bytes_read,
							sizeof(file_info) - client->bytes_read);
						if (count < 0) {
							if (errno != EAGAIN) {
								print_error(strerror(errno));
								client_destroy(client);
								// close connection]]
								// this is an error, return -1
							}

							else {
								break ;
							}
						}

						if (count == 0) {
							print_error(strerror(errno));
							client_destroy(client);
							// close connection
							// this is an error, return -1
							return -1;
						}

						client->bytes_read += count;
						if (client->bytes_read == sizeof(file_info)) {
							handle_name(client, argv[2], save_name);
							client->file.fd = open(save_name, O_CREAT | O_EXCL | O_WRONLY, DEFAULT_MODE);
							if (client->file.fd < 0) {
								perror("FATAL");
							}

							break ;
						}
					}

					if (client->file.fd < 0) {
						print_error(strerror(errno));
						client_destroy(client);
						continue ;
					}

					printf("file_info was read, struct size %zu and size is %zu\n", sizeof(client->file), client->file.size);

					copy_data(client->socket, client->file.fd);
				}

				if (events[i].events & (EPOLLHUP | EPOLLRDHUP)) { // close client socket and free struct cl_ev
					client_destroy(events[i].data.ptr);
					printf("Client closed\n");
				}
			}
		}
	}

	close(epfd);
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

int		handle_name(client_data *client, const char *save_dir, char *save_name)
{
	ssize_t	count;
	size_t	bytes_read = client->name_bytes_read;
	ushort	namelen = client->file.namelen;
	char	buf_name[NAME_MAX];

	memset(buf_name, 0, sizeof(buf_name));

	while (bytes_read < namelen)
	{
		count = read(client->socket, buf_name + bytes_read, namelen - bytes_read);
		if (count < 0) {
			if (errno != EAGAIN) {
				print_error(strerror(errno));
				client_destroy(client);
				return -1;
			}

			else {
				break ;
			}
		}

		if (count == 0) {
			print_error(strerror(errno));
			client_destroy(client);
			return -1;
		}

		bytes_read += count;
		if (bytes_read == client->file.namelen) {
			strcpy(save_name, save_dir);
			if (save_name[strlen(save_name) - 1] != '/') {
				strcat(save_name, "/");
			}
			strcat(save_name, buf_name);
			printf("file - \'%s\'\n", save_name);
		}

		return 1;
	}

	strcpy(save_name, save_dir);

	if (save_name[strlen(save_name) - 1] != '/') {
		strcat(save_name, "/");
	}

	strcat(save_name, buf_name);
	printf("file - \'%s\'\n", save_name);
	return 1;
}
