# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tcali <tcali@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/22 14:54:39 by tcali             #+#    #+#              #
#    Updated: 2026/08/18 19:28:59 by tcali            ###   ########.fr        #
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
	$(SRC_DIR)/LocationConfig.cpp\
	$(SRC_DIR)/ConfigParser.cpp\
	$(SRC_DIR)/CgiHandler.cpp\
	$(SRC_DIR)/CgiProcess.cpp\
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