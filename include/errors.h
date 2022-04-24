#ifndef ERRORS_H
# define ERRORS_H

# define ERR_USAGE_SRV	"\033[1mUsage:\033[0m ./server port save_directory\n"
# define ERR_USAGE_CLT	"\033[1mUsage:\033[0m ./client address:port file_path\n"
# define ERR_ISNUM		"Socket argument must be numerical"
# define ERR_SOCK_LIM	"Socket limit has been exceeded (limit is 65535)"
# define ERR_DIR		"Invalid directory"
# define ERR_FILE		"Invalid file (is not a regular file)"
# define ERR_PORT		"Invalid port"
# define ERR_NAME		"Invalid characters in filename. \
Characters must be alphanumerical or '.', '-' and '_'"
# define ERR			"Something went wrong"

#endif
