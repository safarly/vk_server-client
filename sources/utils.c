#include "server.h"

int		copy_data(int source, int dest)
{
	ssize_t	bytes_written;
	ssize_t	bytes_read;
	ssize_t count;
	char	buffer[BUFF_SIZE];

	memset(buffer, 0, sizeof(buffer));
	while (true)
	{
		bytes_read = read(source, buffer, sizeof(buffer));
		if (bytes_read < 0) {
			if (errno == EAGAIN) {
				break ;
			}

			return -1;
		}

		if (bytes_read == 0) {
			break ;
		}

		bytes_written = 0;
		while (bytes_written < bytes_read)
		{
			count = write(dest, buffer + bytes_written, bytes_read - bytes_written);
			if (count < 0) {
				return -1;
			}

			bytes_written += count;
		}
	}

	return 1;
}

int		_print_error(const char *err, int line, const char *file)
{
	fprintf(stderr, "%s%serror:%s %s: %s:%d\n", BOLD, RED, RESET, err, file, line);
	return -1;
}

void	print_verbose(int flag, client_data *client)
{
	if (!verbose) {
		return ;
	}

	switch (flag) {
		case VF_EPOLLIN:
			printf("EPOLLIN registered from socket %d\n", client->socket);
			break ;

		case VF_INFO:
			printf("file_info was read, size of file is %zu\n", client->file.size);
			break ;

		case VF_NAME:
			printf("name\n");
			break ;

		case VF_EAGAIN:
			printf("%zu bytes was read from %u of file name\n",
					client->name_bytes_read, client->file.namelen);
			break ;

		case VF_HNDL_NAME:
			printf("in handle_name() %zu bytes was read from namelen %d\n",
					client->name_bytes_read, client->file.namelen);
			break ;

		case VF_FILEFD:
			printf("file fd: %d\n", client->file.fd);
			break ;

		case VF_SOCK:
			printf("client socket: %d\n", client->socket);
	}
}

int		isvalid_char(char c)
{
	return (isalpha(c) || isdigit(c) || c == '.' || c == '-' || c == '_');
}

int		check_file_name(client_data *client)
{
	char	*name = client->name;

	while (*name)
	{
		if (!isvalid_char(*name)) {
			return -1;
		}

		name++;
	}

	return 1;
}

void	get_path(const char *save_dir, char *name, char *save_name)
{
	strcpy(save_name, save_dir);

	if (save_name[strlen(save_name) - 1] != '/') {
		strcat(save_name, "/");
	}

	strcat(save_name, name);
}

int		arg_is_numerical(const char *arg)
{
	while (*arg)
	{
		if (!isdigit(*arg)) {
			return 0;
		}

		arg++;
	}

	return 1;
}
