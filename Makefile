NAME		= math-engine
NAME_GUI	= math-engine-gui

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++17

INCLUDES	= -I includes -I libs/imgui -I includes/Windows

SRCS_DIR	= srcs
SRCS		= main.cpp \
			  $(SRCS_DIR)/Distribution.cpp

SRCS_GUI	= gui_main.cpp \
			  $(SRCS_DIR)/Distribution.cpp \
			  $(SRCS_DIR)/ModeManager.cpp \
			  $(SRCS_DIR)/ModeEditor.cpp \
			  $(SRCS_DIR)/Windows/GuiWindow.cpp \
			  $(SRCS_DIR)/Windows/StatisticsWindow.cpp

IMGUI_SRCS	= libs/imgui/imgui.cpp \
			  libs/imgui/imgui_draw.cpp \
			  libs/imgui/imgui_tables.cpp \
			  libs/imgui/imgui_widgets.cpp \
			  libs/imgui/imgui_impl_glfw.cpp \
			  libs/imgui/imgui_impl_opengl3.cpp

OBJS_DIR	= objs
OBJS		= $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)
OBJS_GUI	= $(OBJS_DIR)/gui_main.o \
			  $(OBJS_DIR)/Distribution.o \
			  $(OBJS_DIR)/ModeManager.o \
			  $(OBJS_DIR)/ModeEditor.o \
			  $(OBJS_DIR)/Windows/GuiWindow.o \
			  $(OBJS_DIR)/Windows/StatisticsWindow.o
OBJS_IMGUI	= $(IMGUI_SRCS:libs/imgui/%.cpp=$(OBJS_DIR)/imgui_%.o)

LIBS		= -lzstd
LIBS_GUI	= -lzstd -lglfw -lGL

all: $(NAME)

gui: $(NAME_GUI)

$(OBJS_DIR)/main.o: main.cpp
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(NAME): $(OBJS_DIR)/main.o $(OBJS_DIR)/Distribution.o
	$(CXX) $(CXXFLAGS) $(OBJS_DIR)/main.o $(OBJS_DIR)/Distribution.o -o $(NAME) $(LIBS)

$(NAME_GUI): $(OBJS_GUI) $(OBJS_IMGUI)
	$(CXX) $(CXXFLAGS) $(OBJS_GUI) $(OBJS_IMGUI) -o $(NAME_GUI) $(LIBS_GUI)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(OBJS_DIR)/Windows
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/gui_main.o: gui_main.cpp
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(OBJS_DIR)/Windows
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/imgui_%.o: libs/imgui/%.cpp
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(OBJS_DIR)/Windows
	$(CXX) -std=c++17 $(INCLUDES) -c $< -o $@

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
