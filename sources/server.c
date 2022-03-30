#include "server.h"

void	create_file(int client)
{
	int		bytes_read;
	char	buf_name[PATH_MAX];
	size_t	namelen = 0;

	bytes_read = read(client, &namelen, sizeof(namelen));
	memset(buf_name, 0, sizeof(buf_name));
	if (bytes_read < 0) {
		print_error(strerror(errno));
	}
	bytes_read = read(client, buf_name, namelen);
	if (bytes_read < 0) {
		print_error(strerror(errno));
	}
	printf("namelen - %zu, filename - %s\n", namelen, buf_name);
}

int		main(int argc, char **argv) /* argv[1] - port, argv[2] - save dir */
{
	int		server;
	int		client;
	// int		bytes_read;
	// int		count;

	check_server_args(argc, argv);
	server = create_server_socket(argv[1]);
	while (true) {
		client = accept(server, NULL, NULL);
		// bytes_read = 0;
		// count = 0;
		if (client > 0) {
			create_file(client);
			printf("Client connected successfully\n");
		}
	}
	return 0;
}

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
	if (ap == NULL) {
		print_error(ERR_BIND);
	}
	return -1;
}

void	check_server_args(int argc, char **argv)
{
	struct stat	file_stat;

	memset(&file_stat, 0, sizeof(file_stat));
	if (argc != 3) {
		print_error(ERR_ARGC);
	}
	if (!arg_is_numerical(argv[1])) {
		print_error(ERR_ISNUM);
	}
	if ((unsigned int)atoi(argv[1]) > SHRT_MAX) {
		print_error(ERR_SOCK_LIM);
	}
	stat(argv[2], &file_stat);
	if (!S_ISDIR(file_stat.st_mode)) {
		print_error(ERR_DIR);
	}
}
