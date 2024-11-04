GREEN           := \033[0;92m
YELLOW          := \033[0;93m
BLUE            := \033[0;94m
PURPLE          := \033[0;35m
IPURPLE         := \033[3;35m
END_COLOR       := \033[0;39m


NAME = ircserv
CC = c++
CFLAGS = -g3 -Wall -Wextra -Werror -std=c++98

FILES = srcs/main srcs/Channel srcs/Client srcs/Server srcs/cmd srcs/invite srcs/join srcs/kick srcs/mode srcs/privmsg srcs/topic srcs/handle srcs/signal
OBJDIR = obj

SRC = $(FILES:=.cpp)
OBJS = $(addprefix $(OBJDIR)/, $(FILES:=.o))
HEADER = inc/Channel.hpp inc/Client.hpp inc/Server.hpp


all: $(NAME)

$(NAME):	$(OBJS) $(HEADER)
		@$(CC) $(OBJS) $(CFLAGS) -o $(NAME)
		@echo "$(BLUE)Compiled !$(END_COLOR)"

$(OBJDIR)/%.o: %.cpp $(HEADER)
		@mkdir -p $(dir $@)
		@$(CC) $(CFLAGS) -c $< -o $@

clean:
		@echo "$(IPURPLE)Cleaning...$(END_COLOR)\n"
		@rm -rf $(OBJDIR) $(OBJS)
		@echo "$(GREEN)Cleaned !$(END_COLOR)\n"

fclean: clean
		@echo "$(IPURPLE)Cleaning all...$(END_COLOR)\n"
		@rm -f $(NAME)
		@echo "$(GREEN)All cleaned !$(END_COLOR)\n"

re: fclean all

.PHONY: all clean fclean re