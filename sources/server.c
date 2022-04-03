#include "server.h"

int		set_server_event(int epfd, int server)
{
	struct epoll_event	server_event;

	server_event.data.fd = server;
	server_event.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, server, &server_event) < 0) {
		print_error(strerror(errno));
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
		print_error(strerror(errno));
		return -1;
	}

	*sigfd = signalfd(-1, &mask, SFD_NONBLOCK);
	if (*sigfd == -1) {
		print_error(strerror(errno));
		return -1;
	}

	signal_event.data.fd = *sigfd;
	signal_event.events = EPOLLIN; // check edge trigger or level

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, *sigfd, &signal_event) < 0) {
		print_error(strerror(errno));
		return -1;
	}

	return 1;
}

int		main(int argc, char **argv) /* argv[1] - port, argv[2] - save dir */
{
	int		server;
	int		client;
	int		epfd;
	int		sigfd;
	int		event_count = 0;
	struct epoll_event	events[MAX_EVENTS];

	if (check_server_args(argc, argv) < 0) {
		return EXIT_FAILURE;
	}

	server = create_server_socket(argv[1]);
	if (server < 0) {
		return EXIT_FAILURE;
	}

	epfd = epoll_create1(0);
	if (epfd < 0) {
		print_error(strerror(errno));
		return EXIT_FAILURE;
	}

	if (set_server_event(epfd, server) < 0) {
		return EXIT_FAILURE;
	}

	if (set_signal_event(epfd, &sigfd) < 0) {
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
				client = accept(server, NULL, NULL); //accept needs to be in while loop or check if EPOLLET or EPOLL LEVEL TRIGGEREd needed
				if (client < 0) {
					print_error(strerror(errno));
					continue ;
				}

				// log(0, "blaak %f", 34);
				// update printe_error with variadic args

				else {
					struct client_data *client_data = malloc(sizeof(struct client_data));
					client_data->socket = client;
					client_data->epev.data.ptr = client_data;
					client_data->epev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
					client_data->bytes_read = 0;
					client_data->count = 0;

				// memset(client_data->path_name, 0, sizeof(client_data->path_name));

					set_socket_nonblock(client);
					epoll_ctl(epfd, EPOLL_CTL_ADD, client, &client_data->epev);
					printf("Client connected\n");
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
				struct client_data *client_data = events[i].data.ptr;

				if (events[i].events & EPOLLIN) {	// read from client sock
					printf("Read data from client\n");
					// ssize_t	count = 0, bytes_read = 0, bytes_written = 0;
					// int file = open("~/Desktop/sendto/test.txt", O_CREAT | O_TRUNC | O_RDWR);
				// int count could be mad on stack
					while (client_data->bytes_read < sizeof(struct file_info))
					{
						client_data->count = read(client_data->socket,
							((char *)&client_data->file) + client_data->bytes_read,
							sizeof(struct file_info) - client_data->bytes_read);
						if (client_data->count < 0) {
							if (errno != EAGAIN) {
								print_error(strerror(errno));
								// destroy client
								// close connection]]
								// this is an error, return -1
							}

							else {
								break ;
							}
						}
						if (client_data->count == 0) {
							// destroy client
							// close connection
							// this is an error, return -1
							return -1;
						}
						client_data->bytes_read += client_data->count;
						if (client_data->bytes_read == sizeof(struct file_info)) {
							// client_data->bytes_read = 0;
							// client_data->count = 0;

							client_data->filefd = open("/home/emurky/Desktop/sendto/test.txt", O_CREAT | O_TRUNC | O_RDWR);
							if (client_data->filefd < 0) {
								perror("open");
							}
							break ;
						}
					}

					printf("file_info was read, struct size %zu and size is %zu\n", sizeof(client_data->file), client_data->file.file_stat.st_size);

					copy_data(client_data->socket, client_data->filefd);
				}

				if (events[i].events & (EPOLLHUP | EPOLLRDHUP)) { // close client socket and free struct cl_ev
					printf("Client closed\n");
					// struct client_data *client_to_close = events[i].data.ptr;
					close(client_data->socket);
					close(client_data->filefd);
					free(client_data);
					epoll_ctl(epfd, EPOLL_CTL_DEL, client_data->socket, &client_data->epev);
				}


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

// int		receive_file(struct client_data *client, char *save_dir)
// {
// 	int		filefd;
	// char	path_name[PATH_MAX];
	// struct stat	file_stat;

	// memset(client->path_name, 0, sizeof(client->path_name));

	// ssize_t	bytes_written = 0;
	// ssize_t	bytes_read = 0;
	// ssize_t	count = 0;

	// while (true)
	// {
	// 	client->count = read(client->socket, &client->file_stat + client->bytes_read, sizeof(struct stat) - client->bytes_read);
	// 	if (client->count == 0) {
	// 		break ;
	// 	}
	// 	client->bytes_read += client->count;
	// }
	// printf("%ld file size\n", client->file_stat.st_size);

	// if (read(client->socket, &client->file_stat, sizeof(struct stat)) < 0) {
	// 	return -1;
	// }
// (void)save_dir;
	// if (handle_name(client, save_dir) < 0) {
	// 	return -1;
	// }
											//  O_EXCL
	// filefd = open("~/Desktop/sendto", O_CREAT | O_TRUNC | O_WRONLY, client->file_stat.st_mode);
	// if (filefd < 0) {
	// 	return -1;
	// }

	// if (copy_data(client->socket, filefd) < 0) {
	// 	close(filefd);
	// 	return -1;
	// }

	// printf("File was %ssuccessfully%s saved\n", GREEN, RESET);
	// close(filefd);
// 	return 1;
// }

int		handle_name(int client, const char *save_dir, char *path_name)
{
	char	buf_name[NAME_MAX];
	size_t	namelen = 0;

	memset(buf_name, 0, sizeof(buf_name));
	if (read(client, &namelen, sizeof(namelen)) < 0) {
		return -1;
	}
	if (read(client, buf_name, namelen) < 0) {
		return -1;
	}
	if (strchr(buf_name, '/')) {
		return print_error(ERR_FILENAME);
	}
	strcpy(path_name, save_dir);
	if (path_name[strlen(path_name) - 1] != '/') {
		strcat(path_name, "/");
	}
	strcat(path_name, buf_name);
	printf("file - \'%s\'\n", path_name);
	return 1;
}


// int		handle_name(struct client_data *client, const char *save_dir)
// {
// 	char	buf_name[NAME_MAX];
// 	size_t	namelen = 0;

// 	ssize_t	bytes_read = 0;
// 	ssize_t	count = 0;

// 	memset(buf_name, 0, sizeof(buf_name));

// 	bytes_read = 0;
// 	count = 0;

// 	while (true)
// 	{
// 		count = read(client->socket, &namelen + bytes_read, sizeof(size_t) - bytes_read);
// 		if (count == 0) {
// 			break ;
// 		}
// 		bytes_read += count;
// 	}
// 	printf("%ld namelen\n", namelen);

// 	if (read(client->socket, &namelen, sizeof(namelen)) < 0) {
// 		return -1;
// 	}

// 	bytes_read = 0;
// 	count = 0;

// 	while (true)
// 	{
// 		count = read(client->socket, &buf_name + bytes_read, namelen - bytes_read);
// 		if (count == 0) {
// 			break ;
// 		}
// 		bytes_read += count;
// 	}
// 	printf("%s buf_name\n", buf_name);

// 	// if (read(client->socket, buf_name, namelen) < 0) {
// 	// 	return -1;
// 	// }

// 	if (strchr(buf_name, '/')) {
// 		return print_error(ERR_FILENAME);
// 	}

// 	strcpy(client->path_name, save_dir);
// 	if (client->path_name[strlen(client->path_name) - 1] != '/') {
// 		strcat(client->path_name, "/");
// 	}

// 	strcat(client->path_name, buf_name);
// 	printf("file - \'%s\'\n", client->path_name);
// 	return 1;
// }
