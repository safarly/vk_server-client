#include "server.h"

int		main(int argc, char **argv) /* argv[1] - port, argv[2] - save dir */
{
	int		server;
	int		client;
	// char	status;

	if (check_server_args(argc, argv) < 0) {
		return EXIT_FAILURE;
	}
	server = create_server_socket(argv[1]);
	if (server < 0) {
		return EXIT_FAILURE;
	}
	printf("Server has started and is listening on port %s\n", argv[1]);
	while (true)
	{
		client = accept(server, NULL, NULL);
		if (client < 0) {
			print_error(strerror(errno));
			continue ;
		}
		if (client > 0) {
			printf("Client connected\n");
			if (receive_file(client, argv[2]) < 0) {
				print_error(strerror(errno));
				// status = 1;
				// write(client, &status, 1);
				close(client);
				printf("\nStill listening on port %s\n", argv[1]);
				continue ;
			}
			close(client);
			printf("\nStill listening on port %s\n", argv[1]);
		}
	}
	close(server);
	return 0;
}

int		check_server_args(int argc, char **argv)
{
	struct stat	file_stat;

	memset(&file_stat, 0, sizeof(file_stat));
	if (argc != 3) {
		fprintf(stderr, ERR_USAGE_SRV);
		return -1;
	}
	if (!arg_is_numerical(argv[1])) {
		print_error(ERR_ISNUM);
		return -1;
	}
	if ((unsigned int)atoi(argv[1]) > SHRT_MAX) {
		print_error(ERR_SOCK_LIM);
		return -1;
	}
	stat(argv[2], &file_stat);
	if (!S_ISDIR(file_stat.st_mode)) {
		print_error(ERR_DIR);
		return -1;
	}
	return 1;
}

int		receive_file(int client, char *save_dir)
{
	int		filefd;
	char	path_name[PATH_MAX];
	struct stat	file_stat;

	memset(path_name, 0, sizeof(path_name));
	if (read(client, &file_stat, sizeof(struct stat)) < 0) {
		return -1;
	}
	if (handle_name(client, save_dir, path_name) < 0) {
		return -1;
	}
	filefd = open(path_name, O_CREAT | O_EXCL | O_WRONLY, file_stat.st_mode);
	if (filefd < 0) {
		return -1;
	}
	if (copy_data(client, filefd) < 0) {
		close(filefd);
		return -1;
	}
	printf("File was %ssuccessfully%s saved\n", GREEN, RESET);
	close(filefd);
	return 1;
}

int		handle_name(int client, const char *save_dir, char *path_name)
{
	char	buf_name[NAME_MAX];
	size_t	namelen = 0;

	memset(buf_name, 0, sizeof(buf_name));
	if (read(client, &namelen, sizeof(namelen)) < 0) {
		return -1;
	}
	if (read(client, buf_name, namelen) < 0) {
		return -1;
	}
	if (strchr(buf_name, '/')) {
		return print_error(ERR_FILENAME);
	}
	strcpy(path_name, save_dir);
	if (path_name[strlen(path_name) - 1] != '/') {
		strcat(path_name, "/");
	}
	strcat(path_name, buf_name);
	printf("file - \'%s\'\n", path_name);
	return 1;
}
