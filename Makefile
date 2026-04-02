NAME		= math-engine

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17 `pkg-config --cflags gtk+-3.0 webkit2gtk-4.1`
LIBS		= -lzstd `pkg-config --libs gtk+-3.0 webkit2gtk-4.1`

INCLUDES	= -I includes

SRCS_DIR	= srcs
SRCS		= $(SRCS_DIR)/main.cpp \
			  $(SRCS_DIR)/Distribution.cpp \
			  $(SRCS_DIR)/ModeManager.cpp \
			  $(SRCS_DIR)/AppBridge.cpp

OBJS_DIR	= objs
OBJS		= $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) -std=c++17 $(OBJS) -o $(NAME) $(LIBS)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)
	rm -rf output

re: fclean all

run: all
	./$(NAME)

.PHONY: all clean fclean re run
