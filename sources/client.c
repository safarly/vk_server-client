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
