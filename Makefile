CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude/

SRC = src/main.cpp \
      src/Server.cpp \
      src/Config.cpp \
      src/http/Response.cpp

OBJ = $(SRC:.cpp=.o)
NAME = webserv

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

