#include "server.h"

int		main(int argc, char **argv) /* argv[1] - server addr, argv[2] - file to send */
{
	int		filefd;
	int		server;
	struct stat	file_stat;

	if (check_client_args(argc, argv, &file_stat) < 0) {
		return EXIT_FAILURE;
	}

	server = create_client_socket(argv[1]);
	if (server < 0) {
		return EXIT_FAILURE;
	}

	// if (write(server, &file_stat, sizeof(struct stat)) < 0) {
	// 	close(server);
	// 	print_error(strerror(errno));
	// 	return EXIT_FAILURE;
	// }

	filefd = open(argv[2], O_RDONLY);
	if (filefd < 0) {
		print_error(strerror(errno));
		close(server);
		return EXIT_FAILURE;
	}

	if (send_file(server, filefd, argv) < 0) {
		close(filefd);
		close(server);
		print_error(strerror(errno));
		return EXIT_FAILURE;
	}

	shutdown(server, SHUT_RDWR);
char check_buf[10];
	int socket_check = read(server, check_buf, 1);
	printf("%d - check read from socket\n", socket_check);

	printf("File was %ssuccessfully%s sent\n", GREEN, RESET);
	close(filefd);
	close(server);

	return 0;
}

int		check_client_args(int argc, char **argv, struct stat *file_stat)
{
	memset(file_stat, 0, sizeof(struct stat));
	if (argc != 3) {
		fprintf(stderr, ERR_USAGE_CLT);
		return -1;
	}

	stat(argv[2], file_stat);
	if (!S_ISREG(file_stat->st_mode)) {
		print_error(ERR_FILE);
		return -1;
	}

	return 1;
}

int		send_file(int server, int file, char **argv)
{
	char	*file_name;
	size_t	namelen;

	// file_name = get_filename(argv[2]);
	// namelen = strlen(file_name);
	// if (write(server, &namelen, sizeof(namelen)) < 0) {
	// 	return -1;
	// }

	// if (write(server, file_name, namelen) < 0) {
	// 	return -1;
	// }

	if (copy_data(file, server) < 0) {
		return -1;
	}
	// char	status = 0;
	// if (read(server, &status, sizeof(char)) < 0 || status) {
	// 	return print_error(ERR);
	// }
	return 1;
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
