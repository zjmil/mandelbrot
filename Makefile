CC := clang
CFLAGS := -g -Wall
LIBS := -lallegro -lallegro_main

SRC_DIR := src
TARGET := game
 
SOURCES := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJECTS := $(SOURCES:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $^ -o $@ $(LIBS)

clean:
	$(RM) -r $(TARGET_DIR) $(shell find . -type f -name "*.o")

.PHONY: all clean
