
NAME = toucher_couler
CC = gcc
CFLAGS = -Wall -Wextra
SRCS = toucher_couler.c \
	   toucher_anims.c	\
	   toucher_comms.c 	\
	   toucher_utils.c 	\
	
OBJS = $(SRCS:.c=.o)

all: tc

.o: .c
	$(CC) $(CFLAGS) -c $< -o $@

tc: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

re: fclean all

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

.PHONY: all clean
