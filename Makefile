# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: dgarcez- <dgarcez-@student.42lisboa.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/14 16:42:33 by dgarcez-          #+#    #+#              #
#    Updated: 2026/08/19 17:52:08 by dgarcez-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -g -std=c++98

SOURCES =	Server.cpp Client.cpp Utils.cpp Channel.cpp

SRCS_DIR = srcs
SRCS =	$(addprefix $(SRCS_DIR)/, $(SOURCES))

OBJS_DIR = objs
OBJ_FILES = $(addprefix $(OBJS_DIR)/, $(SOURCES:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJS_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

clean:
	@rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all