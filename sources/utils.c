#include "server.h"

void	copy_data(int source, int dest)
{
	ssize_t	bytes_written;
	ssize_t	bytes_read;
	ssize_t count;
	char	buffer[BUFF_SIZE];

	memset(buffer, 0, sizeof(buffer));
	while (true) {
		bytes_read = read(source, buffer, sizeof(buffer));
		if (bytes_read < 0) {
			print_error(strerror(errno));
		}
		if (bytes_read == 0) {
			break ;
		}
		bytes_written = 0;
		while (bytes_written < bytes_read) {
			count = write(dest, buffer + bytes_written, bytes_read - bytes_written);
			if (count < 0) {
				print_error(strerror(errno));
			}
			bytes_written += count;
		}
	}
}

bool	arg_is_numerical(const char *arg)
{
	while (*arg) {
		if (!isdigit(*arg)) {
			return false;
		}
		arg++;
	}
	return true;
}

void	print_error(const char *err)
{
	fprintf(stderr, "%s%sError:%s %s\n", BOLD, RED, RESET, err);
	exit(EXIT_FAILURE);
}
