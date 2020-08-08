CC := gcc
CFLAGS := -g -Wall
LIBS := -lallegro -lallegro_main
SOURCES := $(shell find src -type f -name "*.c")
OBJECTS := $(SOURCES:.c=.o)
TARGET_DIR := target
TARGET := $(TARGET_DIR)/game

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(TARGET_DIR)
	@echo " Linking..."; $(CC) $^ -o $@ $(LIBS)

%.o: %.c
	@echo " CC $<"; $(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo " Cleaning..."; $(RM) src/*.o $(TARGET)

.PHONY: all clean
