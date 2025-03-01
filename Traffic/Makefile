CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g # -fsanitize=address
LIBS = -pthread

TARGET = ascars
BUILD_DIR = build

SRCS = 	main.c 		\
		init.c 		\
		terminal.c 	\
		utils.c 	\
		inputs.c 	\
		ui.c		\
		cars.c		\
		chained.c	\
		path_find.c	\
		globals.c	\
		title_screen.c	\
		lights.c	\
		game_logic.c	\
		music.c
		# debug.c

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)
	@echo "✅ Build completed successfully!"
	@echo "\033[4;36mTo enable the built-in music, install 'mpg123'\033[0m"
	@echo "🚀 You can now launch \033[4;32m./ascars\033[0m"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "🔧 Compiling $< ..."
	@$(CC) $(CFLAGS) -c $< -o $@ $(LIBS) 

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	@echo "🔨 Linking $(TARGET) ..."
	@$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

re: clean all

clean:
	@echo "🧹 Cleaning objects..."
	@rm -rf $(TARGET) $(BUILD_DIR)

fclean: clean	
	@echo "🧹 Cleaning exe..."
	@rm -f $(TARGET)