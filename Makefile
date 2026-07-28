# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/22 14:54:39 by tcali             #+#    #+#              #
#    Updated: 2026/07/28 19:07:26 by sdossa           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=webserv

CC=c++
CFLAGS=-Wall -Wextra -Werror -std=c++98

OBJ_DIR=obj
SRC_DIR=srcs
HTTP_DIR=http
INCLUDES=-I./includes -I./http

SRC=\
	$(SRC_DIR)/main.cpp\
	$(SRC_DIR)/Server.cpp\
	$(SRC_DIR)/Client.cpp\
	$(SRC_DIR)/ServerConfig.cpp\
	$(HTTP_DIR)/HttpRequest.cpp\
	$(HTTP_DIR)/HttpResponse.cpp\
	$(HTTP_DIR)/MethodHandler.cpp\

OBJ=$(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(NAME): $(OBJ) Makefile
	$(CC) $(LDFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rdf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re