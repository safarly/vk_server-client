#include "server.h"

int		main(int argc, char **argv) /* argv[1] - port, argv[2] - save dir */
{
	int		server;
	int		client;

	check_server_args(argc, argv);
	server = create_server_socket(argv[1]);
	printf("Server has started and is listening on port %s\n", argv[1]);
	while (true)
	{
		client = accept(server, NULL, NULL);
		if (client > 0) {
			printf("Client connected\n");
			receive_file(client, argv[2]);
			close(client);
		}
	}
	return 0;
}

void	check_server_args(int argc, char **argv)
{
	struct stat	file_stat;

	memset(&file_stat, 0, sizeof(file_stat));
	if (argc != 3) {
		print_error(ERR_USAGE_SRV);
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

void	receive_file(int client, char *save_dir)
{
	int		filefd;
	char	path_name[PATH_MAX];
	struct stat	file_stat;

	memset(path_name, 0, sizeof(path_name));
	if (read(client, &file_stat, sizeof(struct stat)) < 0) {
		print_error(strerror(errno));
	}
	handle_name(client, save_dir, path_name);
	filefd = open(path_name, O_CREAT | O_EXCL | O_WRONLY, file_stat.st_mode);
	if (filefd < 0) {
		// print_error(strerror(errno));
		fprintf(stderr, "%s%sError:%s %s\n", BOLD, RED, RESET, strerror(errno));
		return ;
	}
	copy_data(client, filefd);
	printf("File was %ssuccessfully%s saved\n", GREEN, RESET);
	close(filefd);
}

void	handle_name(int client, const char *save_dir, char *path_name)
{
	char	buf_name[NAME_MAX];
	size_t	namelen = 0;

	memset(buf_name, 0, sizeof(buf_name));
	if (read(client, &namelen, sizeof(namelen)) < 0) {
		print_error(strerror(errno));
	}
	if (read(client, buf_name, namelen) < 0) {
		print_error(strerror(errno));
	}
	strcpy(path_name, save_dir);
	strcat(path_name, "/");
	strcat(path_name, buf_name);

	printf("filename - %s\n", path_name);
}
