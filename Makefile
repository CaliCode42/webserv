# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tcali <tcali@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/22 14:54:39 by tcali             #+#    #+#              #
#    Updated: 2026/06/22 14:58:00 by tcali            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=webserv

CC=c++
CFLAGS=-Wall -Wextra -Werror -std=c++98

OBJ_DIR=obj
SRC_DIR=srcs
INCLUDES=-I./includes

SRC=\
	$(SRC_DIR)/main.cpp\
	$(SRC_DIR)/Server.cpp\
	$(SRC_DIR)/Client.cpp\
	$(SRC_DIR)/HttpRequest.cpp\
	$(SRC_DIR)/HttpResponse.cpp\


OBJ=$(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(NAME): $(OBJ) Makefile
	$(CC) $(LDFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -rdf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re