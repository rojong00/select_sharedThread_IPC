CC		:=gcc
#CFLAGS	:=-Wall -Werror -Wextra -g -O2 
CFLAGS	:=-Wall -g -O2 

TARGET	:=server
SRCS	:=select.c sharedMutex.c
OBJS	:=$(SRCS:.c=.o)
TARGET2	:=client
SRCS2	:=client.c


$(TARGET)	:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lrt -lpthread

select.o	: select.c
	$(CC) $(CFLAGS) -c -o $@ $< -I.

sharedMutex.o	: sharedMutex.c
	$(CC) $(CFLAGS) -c -o $@ $< -I.

TARGET2:
	gcc -o $(TARGET2) $(SRCS2)


clean:
	rm -rf *.o
	rm $(TARGET)
