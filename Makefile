NAME = ircserv
CC = c++
CFLAGS = #-Wall -Wextra -Werror -std=c++98

SRCS = srcs/main.cpp srcs/Channel.cpp srcs/Client.cpp srcs/Server.cpp

OBJS = $(SRCS:.cpp=.o)
OBJS_BONUS = $(SRCS_BONUS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp inc/Server.hpp inc/Client.hpp inc/Channel.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(OBJS_BONUS)

fclean: clean
	@rm -f $(NAME) $(NAME_BOUNUS) a.out

re: fclean all

.PHONY: all clean fclean re