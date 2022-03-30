#include "server.h"

void	send_file(int server, int file, char **argv)
{
	char	*file_name;
	size_t	namelen;

	file_name = get_filename(argv[2]);
	namelen = strlen(file_name);
	if (write(server, &namelen, sizeof(namelen)) < 0) {
		print_error(strerror(errno));
	}
	if (write(server, file_name, namelen) < 0) {
		print_error(strerror(errno));
	}
	copy_data(file, server);
}

int		main(int argc, char **argv) /* argv[1] - server addr, argv[2] - file to send */
{
	int		filefd;
	int		client;
	struct stat	file_stat;

	check_client_args(argc, argv, &file_stat);
	filefd = open(argv[2], O_RDONLY);
	if (filefd < 0) {
		perror(argv[0]);
		return EXIT_FAILURE;
	}
	client = create_client_socket(argv[1]);
	if (write(client, &file_stat, sizeof(struct stat)) < 0) {
		print_error(strerror(errno));
	}
	send_file(client, filefd, argv);
	printf("File was %ssuccessfully%s sent\n", GREEN, RESET);
	close(filefd);
	close(client);

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
