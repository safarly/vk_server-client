#include "server.h"


void	handle_name(int client, const char *save_dir, char *path_name)
{
	char	buf_name[PATH_MAX];
	size_t	namelen = 0;

	memset(buf_name, 0, PATH_MAX);
	memset(path_name, 0, PATH_MAX);
	if (read(client, &namelen, sizeof(namelen)) < 0) {
		print_error(strerror(errno));
	}
	if (read(client, buf_name, namelen) < 0) {
		print_error(strerror(errno));
	}
	strcpy(path_name, save_dir);
	strcat(path_name, "/");
	strcat(path_name, buf_name);

	printf("namelen - %zu, filename - %s\n", namelen, path_name);
}

void	receive_file(int client, char *save_dir)
{
	int		filefd;
	char	path_name[PATH_MAX];
	struct stat	file_stat;

	if (read(client, &file_stat, sizeof(struct stat)) < 0) {
		print_error(strerror(errno));
	}
	handle_name(client, save_dir, path_name);
	filefd = open(path_name, O_CREAT | O_EXCL, file_stat.st_mode);
	if (filefd < 0) {
		// print_error(strerror(errno));
		fprintf(stderr, "%s%sError:%s %s\n", BOLD, RED, RESET, strerror(errno));
	}
	// printf("namelen - %zu, filename - %s\n", namelen, path_name);
	close(filefd);
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
			printf("Client connected successfully\n");
			receive_file(client, argv[2]);
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
