
NAME = morpion
CC = gcc
CFLAGS = -Wall -Wextra
SRCS = morpion.c 
	
OBJS = $(SRCS:%.c=%.o)

all: morpion

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

morpion: $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

re: fclean all

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

.PHONY: all clean
