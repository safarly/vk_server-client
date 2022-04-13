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
				return 1;
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
