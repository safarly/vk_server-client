#include "server.h"

int		main(int argc, char **argv) /* argv[1] - port, argv[2] - save dir */
{
	int		server;
	int		epfd;
	int		sigfd;

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
	run_server(epfd, sigfd, server, argv[2]);

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

void	run_server(int epfd, int sigfd, int server, const char *save_dir)
{
	int		event_count;
	bool	terminate = false;
	client_data			*client;
	struct epoll_event	events[MAX_EVENTS];

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
				puts("\nServer is shutting down...");
				close(epfd);
				close(server);
				terminate = true;
				break ;
			}

			else {
				client = events[i].data.ptr;
				client->epev.events = events[i].events;
				if (process_client_event(client, save_dir) < 0) {
					continue ;
				}
			}
		}
	}
}

int		process_client_event(client_data *client, const char *save_dir)
{
	if (client->epev.events & EPOLLIN) {
		// print_verbose(VF_EPOLLIN, client);
		if (receive_data_from_client(client, save_dir) < 0) {
			client_destroy(client);
			return -1;
		}
	}

	if (client->epev.events & (EPOLLHUP | EPOLLRDHUP)) {
		printf("Client from %s has closed connection\n", client->addr_str);
		client_destroy(client);
	}

	return 1;
}
