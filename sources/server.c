#include "server.h"

int		main(int argc, char **argv)
{
	check_arguments(argc, argv);

	return 0;
}

void	check_arguments(int argc, char **argv)
{
	struct stat	file_stat = (struct stat){ 0 };

	if (argc != 3) {
		print_error(ERR_ARGC);
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
