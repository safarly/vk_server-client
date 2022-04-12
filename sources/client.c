#include "server.h"

int		main(int argc, char **argv) /* argv[1] - server addr, argv[2] - file to send */
{
	int		server;
	file_info	file;

	memset(&file, 0, sizeof(file_info));

	if (check_client_args(argc, argv, &file) < 0) {
		return EXIT_FAILURE;
	}

	server = create_client_socket(argv[1]);
	if (server < 0) {
		return EXIT_FAILURE;
	}

	file.fd = open(argv[2], O_RDONLY);
	if (file.fd < 0) {
		close(server);
		print_error(strerror(errno));
		return EXIT_FAILURE;
	}

	if (send_file(server, &file, argv[2]) < 0) {
		close(file.fd);
		close(server);
		print_error(strerror(errno));
		return EXIT_FAILURE;
	}

	shutdown(server, SHUT_RDWR);

	char check_buf[1];
	int socket_check = read(server, check_buf, 1);
	printf("%d - check read from socket\n", socket_check);

	printf("File was %ssuccessfully%s sent\n", GREEN, RESET);
	close(file.fd);
	close(server);

	return 0;
}

int		check_client_args(int argc, char **argv, file_info *file)
{
	struct stat	file_stat;

	memset(&file_stat, 0, sizeof(struct stat));
	if (argc != 3) {
		fprintf(stderr, ERR_USAGE_CLT);
		return -1;
	}

	stat(argv[2], &file_stat);
	if (!S_ISREG(file_stat.st_mode)) {
		print_error(ERR_FILE);
		return -1;
	}

	file->size = file_stat.st_size;

	return 1;
}

int		send_file(int server, file_info *file, char *file_path)
{
	char	*file_name;

	file_name = get_file_name(file_path);
	file->namelen = strlen(file_name);

	if (write(server, file, sizeof(file_info) - 7) < 0) {
		return -1;
	}
	sleep(1);
	if (write(server, (char *)file + 7, 7) < 0) {
		return -1;
	}

	if (write(server, file_name, 3) < 0) {
		return -1;
	}
	sleep(1);
	if (write(server, file_name + 3, file->namelen - 3) < 0) {
		return -1;
	}

	if (copy_data(file->fd, server) < 0) {
		return -1;
	}

	return 1;
}

char	*get_file_name(char *path)
{
	char	*filename = strrchr(path, '/');

	if (filename == NULL) {
		return path;
	}

	filename++;
	return filename;
}
