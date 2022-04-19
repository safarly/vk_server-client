#include "server.h"

int		receive_data_from_client(client_data *client, const char *save_dir)
{
	char		save_name[PATH_MAX];
	struct stat	file_stat;

	memset(save_name, 0, sizeof(save_name));

	if (client->fflags.info_read == false) {
		if (get_file_info(client) < 0) {
			return -1;
		}
		if (client->fflags.info_read == false) {
			return 1;
		}
	}

	if (client->fflags.name_read == false) {
		if (handle_name(client, save_dir, save_name) < 0) {
			return -1;
		}

		if (client->fflags.name_read == false) {
			return 1;
		}

		client->file.fd = open(save_name, O_CREAT | O_EXCL | O_WRONLY, DEFAULT_MODE);
		print_verbose(VF_FILEFD, client);
	}


	if (client->file.fd < 0) {
		print_error(strerror(errno));
		return -1;
	}

	else if (client->fflags.opened == false) {
		client->fflags.opened = true;
	}

	if (copy_data(client->socket, client->file.fd) < 0) {
		print_error(strerror(errno));
		return -1;
	}

	if (client->fflags.opened == true && client->fflags.saved == false) {
		get_path(save_dir, client->name, save_name);
		// printf("save_name %s\n", save_name);
		stat(save_name, &file_stat);
		if ((size_t)file_stat.st_size == client->file.size) {
			client->fflags.saved = true;
			printf("File \"%s\" was received and saved in \"%s\"\n", client->name, save_dir);
		}
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
			client->fflags.info_read = true;
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

	get_path(save_dir, client->name, save_name);
	printf("Path for a file to save: \"%s\"\n", save_name);
	client->fflags.name_read = true;

	return 1;
}
