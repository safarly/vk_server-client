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

	if (client->name_read == false) {
		if (handle_name(client, save_dir, save_name) < 0) {
			return -1;
		}

		if (client->name_read == false) {
			return 1;
		}

		client->file.fd = open(save_name, O_CREAT | O_EXCL | O_WRONLY, DEFAULT_MODE);
		print_verbose(VF_FILEFD, client);
	}


	if (client->file.fd < 0) {
		print_error(strerror(errno));
		return -1;
	}

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
			if (errno == EAGAIN) {
				break ;
			}

			print_error(strerror(errno));
			return -1;
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

	print_verbose(VF_INFO, client);

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
			if (errno == EAGAIN) {
				print_verbose(VF_EAGAIN, client);
				return 1;
			}

			print_error(strerror(errno));
			return -1;
		}

		if (count == 0) {
			print_error(strerror(errno));
			return -1;
		}

		client->name_bytes_read += count;
		print_verbose(VF_HNDL_NAME, client);
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
	printf("Path for a received file: \'%s\'\n", save_name);
	client->name_read = true;

	return 1;
}
