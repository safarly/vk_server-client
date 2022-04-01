#include "server.h"

int		create_server_socket(char *port)
{
	int		sock;
	int		addr_status;
	struct addrinfo	hints;
	struct addrinfo	*addresses, *ap;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
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
			&& listen(sock, BACKLOG_LIMIT) == 0) {
			freeaddrinfo(addresses);
			return sock;
		}
		close(sock);
	}
	freeaddrinfo(addresses);
	print_error(ERR_BIND);
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
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
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
			freeaddrinfo(addresses);
			return sock;
		}
		// print_error(strerror(errno));
		close(sock);
	}
	freeaddrinfo(addresses);
	print_error(ERR_CONNECT);
	return -1;
	// printf("address - %s, port - %s %d\n", hostaddr, port, hints.ai_family);
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
