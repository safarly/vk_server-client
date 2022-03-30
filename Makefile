# ############################################################################ #
#                                                                              #
#                                 Makefile                                     #
#                                                                              #
# ############################################################################ #

SRCSDIR				= ./sources
INCSDIR				= ./include

SERVER				= server
S_SRCS				= $(addprefix $(SRCSDIR)/, \
						server.c \
						utils.c )
S_OBJS				= $(S_SRCS:.c=.o)
S_DEPS				= $(S_SRCS:.c=.d)

CLIENT				= client
C_SRCS				= $(addprefix $(SRCSDIR)/, \
						client.c \
						utils.c )
C_OBJS				= $(C_SRCS:.c=.o)
C_DEPS				= $(C_SRCS:.c=.d)

# NAME				= vk

CC 					= gcc
RM 					= rm -f
CFLAGS				= -Wall -Wextra -Werror --pedantic -g -fsanitize=address
CPPFLAGS			= -MMD -I$(INCSDIR)

all:				server client

$(S_OBJS):			Makefile
$(C_OBJS):			Makefile

clang:				fclean
					make CC=clang all

$(SERVER):			$(S_OBJS) Makefile
					$(CC) $(CFLAGS) $(S_OBJS) -o $(SERVER)

$(CLIENT):			$(C_OBJS) Makefile
					$(CC) $(CFLAGS) $(C_OBJS) -o $(CLIENT)

$(NAME):			$(SERVER) $(CLIENT)

-include			$(S_DEPS) $(C_DEPS)

clean:
					$(RM) $(S_OBJS) $(S_DEPS) $(C_OBJS) $(C_DEPS)

fclean:				clean
					$(RM) $(SERVER) $(CLIENT)

leaks:				all
					valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
					./client localhost:4242 ~/.vimrc
					@printf "\n"
					valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
					./server 4242 ~/Desktop

re:					fclean all

sc:
	rm -rf *.o *.a */*.o */*.a */*/*.o */*/*.a */*.d *.d */*.d */*/*.d
	rm -rf 'server '* 'client '* server client

.PHONY:				all clean fclean re sc

#.SILENT:
