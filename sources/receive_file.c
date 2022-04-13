#include "server.h"

int		receive_data_from_client(client_data *client, const char *save_dir)
{
	char	save_name[PATH_MAX];

	memset(save_name, 0, sizeof(save_name));

	if (client->file_info_read == false) {
		if (get_file_info(client) < 0) {
			return -1;
		}
		if (client->file_info_read == false) {
			return 1;
		}
	}
	printf("file_info was read, socket %d file fd %d\n", client->socket, client->file.fd);

	if (client->name_read == false) {
		if (handle_name(client, save_dir, save_name) < 0) {
			return -1;
		}

		if (client->name_read == false) {
			return 1;
		}

		printf("file_name was read, %s\n", save_name);

		client->file.fd = open(save_name, O_CREAT | O_EXCL | O_WRONLY, DEFAULT_MODE);
		printf("file was opened, %d is fd\nsave_name is %s\n", client->file.fd, save_name);
	}


	if (client->file.fd < 0) {
		print_error(strerror(errno));
		return -1;
	}

	// printf("file_info was read, socket %d file fd %d\n", client->socket, client->file.fd);

	if (copy_data(client->socket, client->file.fd) < 0) {
		print_error(strerror(errno));
		return -1;
	}

	return 0;
}

int		get_file_info(client_data *client)
{
	int		count;

	while (client->bytes_read < sizeof(file_info))
	{
		count = read(client->socket,
			((char *)&client->file) + client->bytes_read,
			sizeof(file_info) - client->bytes_read);
		if (count < 0) {
			if (errno != EAGAIN) {
				print_error(strerror(errno));
				return -1;
			}

			else {
				break ;
			}
		}

		if (count == 0) {
			print_error(strerror(errno));
			return -1;
		}

		client->bytes_read += count;
		if (client->bytes_read == sizeof(file_info)) {
			client->file_info_read = true;
			client->file.fd = 0;
			break ;
		}
	}

	return 1;
}

int		handle_name(client_data *client, const char *save_dir, char *save_name)
{
	ssize_t	count;

	while (client->name_bytes_read < client->file.namelen)
	{
		count = read(client->socket, client->name + client->name_bytes_read,
					 client->file.namelen - client->name_bytes_read);
		if (count < 0) {
			if (errno != EAGAIN) {
				print_error(strerror(errno));
				return -1;
			}

			else {
				printf("%zu bytes was read from %u\n", client->name_bytes_read, client->file.namelen);
				return 1;
			}
		}

		if (count == 0) {
			print_error(strerror(errno));
			return -1;
		}

		client->name_bytes_read += count;
		printf("in handle_name num_bytes_read %zu namelen %d\n", client->name_bytes_read, client->file.namelen);
	}

	if (check_file_name(client) < 0) {
		print_error(ERR_NAME);
		return -1;
	}

	strcpy(save_name, save_dir);

	if (save_name[strlen(save_name) - 1] != '/') {
		strcat(save_name, "/");
	}

	strcat(save_name, client->name);

	printf("file - \'%s\'\n", save_name);
	client->name_read = true;

	return 1;
}
