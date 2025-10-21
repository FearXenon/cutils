
SRCS = $(shell find . -name '*.c')
OBJS = $(SRCS:.c=.o)

TARGET = test 

.PHONY: check-leaks run clean

check-leaks: $(TARGET)
	valgrind --track-origins=yes --leak-check=full -s ./$(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS)

$(TARGET): $(OBJS)
	gcc -o $@ $^
%.o: %.c
	gcc -g -ggdb -c -o $@ $< -I./src
