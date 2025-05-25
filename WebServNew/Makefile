NAME = webserv

SRC_DIR = ./srcs
OBJ_DIR = ./objs

CC = c++
FLAGS = -Wall -Wextra -Werror -I./include -O2 -std=c++98

SRCS =	$(SRC_DIR)/Server.cpp \
		$(SRC_DIR)/Client.cpp \
		$(SRC_DIR)/HttpRequest.cpp \
		$(SRC_DIR)/HttpResponse.cpp \
		$(SRC_DIR)/Config.cpp \
		$(SRC_DIR)/main.cpp

RM = rm -rf
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: obj_dir $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(OBJ) $(FLAGS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(FLAGS) -c $< -o $@

obj_dir:
	@mkdir -p $(OBJ_DIR)

clean:
	@$(RM) $(OBJ_DIR)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fcle	an re norm obj_dir
