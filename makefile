# ===== Compiler & flags =====
CC      := gcc
CFLAGS  := -Wall -Wextra -pthread -Iinclude
LDFLAGS := -pthread

# ===== Target =====
TARGET := main

# ===== Source files =====
SRC_DIR := src
SRCS := $(SRC_DIR)/main.c \
		$(SRC_DIR)/ringBuffer.c \
        $(SRC_DIR)/serverThread.c \
        $(SRC_DIR)/networkThread.c

# ===== Object files =====
OBJS := $(SRCS:.c=.o)

# ===== Default target =====
all: $(TARGET)

# ===== Link =====
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# ===== Compile =====
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ===== Clean =====
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

# ===== Phony targets =====
.PHONY: all clean
