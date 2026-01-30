BUILD ?= debug 

CC      := gcc
CSTD    := -std=c99 -c

WARN    := -Wall -Wextra -Wpedantic \
           -Wshadow -Wconversion -Wsign-conversion \
           -Wstrict-prototypes -Wmissing-prototypes \
           -Wformat=2 -Wundef

BASE    := $(CSTD) $(WARN) -fno-common -D_POSIX_C_SOURCE=200809L

ifeq ($(BUILD), debug)
	CFLAGS_DEBUG := $(BASE) \
		-ggdb \
		-O0 -g3 \
		-fsanitize=address,undefined \
		-fno-omit-frame-pointer \
		-DDEBUG

	CLFAGS := $(CFLAGS_DEBUG)
else
	CFLAGS_RELEASE := $(BASE) \
		-O3 -DNDEBUG \
		-flto \
		-fvisibility=hidden

	CFLAGS := $(CFLAGS_RELEASE)
endif

SRCS = $(shell find . -name '*.c')
OBJS = $(SRCS:.c=.o)

TARGET = test 

.PHONY: check-leaks run clean

check-leaks: $(TARGET)
	valgrind --track-origins=yes --leak-check=full -s ./$(TARGET)

run: $(TARGET)
	./$(TARGET)

build: $(TARGET)

clean:
	rm -f $(OBJS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^
%.o: %.c
	gcc $(CFLAGS) -o $@ $< -I./src -lpthread
