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
		game_logic.c
		# debug.c

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)
	@echo "✅ Build completed successfully!"
	@echo "🚀 You can now launch ./ascars"

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