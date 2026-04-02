NAME		= math-engine
NAME_GUI	= math-engine-gui

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17
CXXFLAGS_GUI = -Wall -Wextra -Werror -std=c++17 `pkg-config --cflags gtk+-3.0 webkit2gtk-4.1`
LIBS		= -lzstd
LIBS_GUI	= -lzstd `pkg-config --libs gtk+-3.0 webkit2gtk-4.1`

INCLUDES	= -I includes

SRCS_DIR	= srcs
SRCS		= main.cpp \
			  $(SRCS_DIR)/Distribution.cpp

SRCS_GUI	= gui_main.cpp \
			  $(SRCS_DIR)/Distribution.cpp \
			  $(SRCS_DIR)/ModeManager.cpp \
			  $(SRCS_DIR)/AppBridge.cpp

OBJS_DIR	= objs
OBJS		= $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)
OBJS_GUI	= $(OBJS_DIR)/gui_main.o \
			  $(OBJS_DIR)/Distribution.o \
			  $(OBJS_DIR)/ModeManager.o \
			  $(OBJS_DIR)/AppBridge.o

all: $(NAME)

gui: $(NAME_GUI)

$(NAME): $(OBJS_DIR)/main.o $(OBJS_DIR)/Distribution.o
	$(CXX) $(CXXFLAGS) $(OBJS_DIR)/main.o $(OBJS_DIR)/Distribution.o -o $(NAME) $(LIBS)

$(NAME_GUI): $(OBJS_GUI)
	$(CXX) $(CXXFLAGS) $(OBJS_GUI) -o $(NAME_GUI) $(LIBS_GUI)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/main.o: main.cpp
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/gui_main.o: gui_main.cpp
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS_GUI) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/AppBridge.o: $(SRCS_DIR)/AppBridge.cpp
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS_GUI) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME) $(NAME_GUI)
	rm -rf output

re: fclean all

run: $(NAME)
	./$(NAME)

run-gui: gui
	./$(NAME_GUI)

.PHONY: all gui clean fclean re run run-gui
