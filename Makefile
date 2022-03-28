# ############################################################################ #
#                                                                              #
#                                 Makefile                                     #
#                                                                              #
# ############################################################################ #

SERVER				= server
S_SRCS				= server.c utils.c
S_OBJS				= $(S_SRCS:.c=.o)
S_DEPS				= $(S_SRCS:.c=.d)

CLIENT				= client
C_SRCS				= client.c utils.c
C_OBJS				= $(C_SRCS:.c=.o)
C_DEPS				= $(C_SRCS:.c=.d)

NAME				= vk

CC 					= gcc
RM 					= rm -f
CFLAGS				= -Wall -Wextra -Werror --pedantic #-g -fsanitize=address
CPPFLAGS			= -MMD -I.

all:				server client

$(SERVER):			$(S_OBJS)
					$(CC) $(CFLAGS) $(S_OBJS) -o $(SERVER)

$(CLIENT):			$(C_OBJS)
					$(CC) $(CFLAGS) $(C_OBJS) -o $(CLIENT)

$(NAME):			$(SERVER) $(CLIENT)

-include			$(S_DEPS) $(C_DEPS)

clean:
					$(RM) $(S_OBJS) $(S_DEPS) $(C_OBJS) $(C_DEPS)

fclean:				clean
					$(RM) $(SERVER) $(CLIENT)

re:					fclean all

sc:
	rm -rf *.o *.a */*.o */*.a */*/*.o */*/*.a */*.d *.d */*.d */*/*.d
	rm -rf 'server '* 'client '* server client

.PHONY:				all clean fclean re sc

#.SILENT:
