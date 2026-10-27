CC = gcc
AS = nasm
CFLAGS = -Wall -Wextra -g
ASFLAGS = -f elf64

# final exe file
TARGET = my_shell

# obj files
OBJS = main.o parser.o syscalls.o

# make
all: $(TARGET)

# all obj files to one obj file
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# compile main.c
main.o: main.c parser.h
	$(CC) $(CFLAGS) -c main.c

# compile parser.o
parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

# compile syscalls.s
syscalls.o: syscalls.s
	$(AS) $(ASFLAGS) syscalls.s -o syscalls.o

# make clean
clean:
	rm -f $(OBJS) $(TARGET)

# prevent name collisions with real files in the folder
.PHONY: all clean