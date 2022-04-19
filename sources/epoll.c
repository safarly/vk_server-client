#include "server.h"

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

int		accept_new_client(int epfd, int server)
{
	int		client_sock;
	client_data	*client;
	struct sockaddr_in	address;
	socklen_t			addr_size = sizeof(address);

	memset(&address, 0, addr_size);
	client_sock = accept(server, (struct sockaddr *) &address, &addr_size); //accept needs to be in while loop or check if EPOLLET or EPOLL LEVEL TRIGGEREd needed
	if (client_sock < 0) {
		return -1;
	}

	client = client_init(client_sock, epfd);
	if (client == NULL) {
		return -1;
	}

	if (set_socket_nonblock(client->socket) < 0) {
		close(client->socket);
		free(client);
		return -1;
	}

	inet_ntop(AF_INET, &address.sin_addr, client->addr_str, INET_ADDRSTRLEN);

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, client->socket, &client->epev) < 0) {
		close(client->socket);
		free(client);
		return -1;
	}

	printf("\nClient connected from %s\n", client->addr_str);
	print_verbose(VF_SOCK, client);

	return 1;
}

client_data	*client_init(int socket, int epfd)
{
	client_data	*client = malloc(sizeof(client_data));
	if (client == NULL) {
		return NULL;
	}

	memset(client, 0, sizeof(client_data));
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
	if (client->fflags.opened == true) {
		close(client->file.fd);
	}

	free(client);
	client = NULL;

	return 1;
}
