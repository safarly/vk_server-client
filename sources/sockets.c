#include "server.h"

int		set_socket_nonblock(int socket)
{
	if (fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK) < 0) {
		print_error(strerror(errno));
		return -1;
	}
	return 1;
}

int		create_server_socket(char *port)
{
	int		sock;
	int		addr_status;
	struct addrinfo	hints;
	struct addrinfo	*addresses, *ap;

	hints_init(&hints, HF_SERVER);
	addr_status = getaddrinfo(NULL, port, &hints, &addresses);
	if (addr_status) {
		print_error(gai_strerror(addr_status));
		return -1;
	}

	for (ap = addresses; ap != NULL; ap = ap->ai_next) {
		sock = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
		if (sock == -1) {
			continue ;
		}

		if (bind(sock, ap->ai_addr, ap->ai_addrlen) == 0
			&& listen(sock, BACKLOG_LIMIT) == 0
			&& set_socket_nonblock(sock) != -1) {
			freeaddrinfo(addresses);
			return sock;
		}

		close(sock);
	}

	freeaddrinfo(addresses);
	print_error(strerror(errno));

	return -1;
}

int		create_client_socket(char *host)
{
	char	*hostaddr = host;
	char	*port = get_port(host);
	int		sock;
	int		addr_status;
	struct addrinfo	hints;
	struct addrinfo	*addresses, *ap;

	if (port == NULL) {
		print_error(ERR_PORT);
		return -1;
	}

	hints_init(&hints, HF_CLIENT);
	addr_status = getaddrinfo(hostaddr, port, &hints, &addresses);
	if (addr_status) {
		print_error(gai_strerror(addr_status));
		return -1;
	}

	for (ap = addresses; ap != NULL; ap = ap->ai_next) {
		sock = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
		if (sock == -1) {
			continue ;
		}

		if (connect(sock, ap->ai_addr, ap->ai_addrlen) != -1) {
			printf("Client has connected to server\n");
			freeaddrinfo(addresses);
			return sock;
		}
		close(sock);
	}

	freeaddrinfo(addresses);
	print_error(strerror(errno));

	return -1;
}

void	hints_init(struct addrinfo *hints, int flag)
{
	memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_UNSPEC;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = 0;

	if (flag == HF_SERVER) {
		hints->ai_flags = AI_PASSIVE;
	}

	else if (flag == HF_CLIENT) {
		hints->ai_flags = 0;
	}
}

char	*get_port(char *host)
{
	char	*port = strrchr(host, ':');

	if (port == NULL) {
		return NULL;
	}

	*port = '\0';
	port++;

	return port;
}
