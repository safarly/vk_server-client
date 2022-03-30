#include "server.h"

int		main(int argc, char **argv) /* argv[1] - server addr, argv[2] - file to send */
{
	int		fd;
	int		client;
	char	*file_name;
	size_t	namelen;
	struct stat	file_stat;
	// size_t	filesize;

	check_client_args(argc, argv, &file_stat);
	fd = open(argv[2], O_RDONLY);
	if (fd < 0) {
		perror(argv[0]);
		return EXIT_FAILURE;
	}
	client = create_client_socket(argv[1]);
	file_name = get_filename(argv[2]);
	namelen = strlen(file_name);
	printf("filename - %s, filesize - %zu\n", file_name, file_stat.st_size);
	write(client, &file_stat, sizeof(struct stat));
	write(client, &namelen, sizeof(namelen));
	write(client, file_name, namelen);

	// write(client, "\n", 1);
	close(fd);

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

void	check_client_args(int argc, char **argv, struct stat *file_stat)
{
	memset(file_stat, 0, sizeof(struct stat));
	if (argc != 3) {
		print_error(ERR_ARGC);
	}
	stat(argv[2], file_stat);
	if (!S_ISREG(file_stat->st_mode)) {
		print_error(ERR_FILE);
	}
	// return file_stat.st_size;
}

char	*get_filename(char *path)
{
	char	*filename = strrchr(path, '/');

	if (filename == NULL) {
		return path;
	}
	filename++;
	return filename;
}
