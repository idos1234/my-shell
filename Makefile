CC = gcc
AS = nasm
CFLAGS = -Wall -Wextra -g -O0
ASFLAGS = -f elf64

# final exe file
TARGET = my_shell

# obj files
OBJS = main.o parser.o syscalls.o executor.o command_struct.o

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

# compile executor.c
executor.o: executor.c executor.h
	$(CC) $(CFLAGS) -c executor.c

# compile command_struct.c
command_struct.o: command_struct.c command_struct.h
	$(CC) $(CFLAGS) -c command_struct.c

# compile syscalls.s
syscalls.o: syscalls.s
	$(AS) $(ASFLAGS) syscalls.s -o syscalls.o

# run valgrind to check for memory leaks
valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# make clean
clean:
	rm -f $(OBJS) $(TARGET)

# prevent name collisions with real files in the folder
.PHONY: all clean valgrind