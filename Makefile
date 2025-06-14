NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes

SRC_DIR = srcs
OBJ_DIR = obj

SRCS = $(SRC_DIR)/Server.cpp \
	   $(SRC_DIR)/Client.cpp \
	   $(SRC_DIR)/HttpRequest.cpp \
	   $(SRC_DIR)/ConfigFile.cpp \
	   $(SRC_DIR)/Config.cpp \
	   $(SRC_DIR)/LocationConfig.cpp \
	   $(SRC_DIR)/ServerConfig.cpp 

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Color codes
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[1;34m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(YELLOW)Building webserver...$(RESET)"
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(SRC_DIR)/main.cpp $(OBJS)
	@echo "$(GREEN)Webserver ready!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

test: srcs/test_main.cpp $(OBJS)
	@echo "$(YELLOW)Building test_webserv...$(RESET)"
	@$(CXX) $(CXXFLAGS) -o test_webserv $(SRC_DIR)/test_main.cpp $(OBJS)
	@echo "$(GREEN)Test binary ready!$(RESET)"

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(YELLOW)Removing binaries...$(RESET)"
	@rm -rf $(NAME) test_webserv

re: fclean all

.PHONY: all clean fclean re test