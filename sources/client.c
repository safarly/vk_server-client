#include "server.h"

int		main(int argc, char **argv)
{
	check_arguments(argc, argv);
	printf("tut\n");

	return 0;
}

void	check_arguments(int argc, char **argv)
{
	struct stat	file_stat = (struct stat){ 0 };

	if (argc != 3) {
		print_error(ERR_ARGC);
	}
	stat(argv[2], &file_stat);
	if (!S_ISREG(file_stat.st_mode)) {
		print_error(ERR_FILE);
	}
}
