# Compiler and Compiler Flags
CC=gcc
CFLAGS=-Wall -g -Iinclude -Ilib

# Linker flags
LDFLAGS=-lreadline -lm

# The build target executable:
TARGET1=powerOn
TARGET2=minios
# FORK_PROGRAM=kernel/20192490/print_os_name

# Source, Object files
SRCS=kernel/kernel.c \
	kernel/system.c \
	kernel/mem_s/mem_allocate.c \
	kernel/mem_s/dummy_memory_management.c \
	kernel/program_handle/execute.c \
	lib/print_util.c \
	lib/frame_struct.c \
	  
OBJS=$(SRCS:.c=.o)

#Include directory
INCLUDE_DIR=include

#$(FORK_PROGRAM)... 추후에 추가 $(OBJS)->$^
all: $(TARGET1) $(TARGET2) 

$(TARGET1): boot/boot.o
	$(CC) $(CFLAGS) -o $(TARGET1) boot/boot.o $(LDFLAGS)

$(TARGET2): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJS) $(LDFLAGS)

# Make fork program... 추후에 추가
#$(FORK_CHILD_PROGRAM): kernel/20192490/print_os_name.o
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# To obtain object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up:
clean:
	rm -f $(OBJS) $(TARGET)
