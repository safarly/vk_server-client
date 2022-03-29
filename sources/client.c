#include "server.h"

int		main(int argc, char **argv) /* argv[1] - server addr, argv[2] - file to send */
{
	int	fd;
	int	client;

	check_arguments(argc, argv);
	fd = open(argv[2], O_RDONLY);
	if (fd < 0) {
		perror(argv[0]);
		return EXIT_FAILURE;
	}
	client = create_client_socket(argv[1]);
	close(fd);
(void)client;

	return 0;
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
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	addr_status = getaddrinfo(hostaddr, port, &hints, &addresses);
	if (addr_status) {
		print_error(gai_strerror(addr_status));
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
		close(sock);
	}
	freeaddrinfo(addresses);
	if (ap == NULL) {
		print_error(ERR_CONNECT);
	}
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

void	check_arguments(int argc, char **argv)
{
	struct stat	file_stat = (struct stat){ 0 };

	if (argc != 3) {
		print_error(ERR_ARGC);
	}
	stat(argv[2], &file_stat);
	if (!S_ISREG(file_stat.st_mode)) {
		print_error(ERR_FILE);
	}
}
