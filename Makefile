DEBUG := 0
CFLAGS := -Wall -Wextra -Werror -O3 -DDEBUG=$(DEBUG)
LDFLAGS := -O3

main: main.o

.PHONY: debug
debug:
	$(MAKE) DEBUG=1

.PHONY: run
run:
	./main

.PHONY: clean
clean:
	$(RM) *.o main
